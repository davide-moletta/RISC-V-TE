import sys
import subprocess
import os
import time
import shlex
import instrumenter
import CFGextractor
from pathlib import Path

#############
# CONSTANTS #
#############
curr_dir = subprocess.run(['pwd'], capture_output = True, text = True) # Retrieve the current directory
DIRECTORY = os.path.dirname(curr_dir.stdout.strip())                   # Current working directory
SOURCES_DIRECTORY = f"{DIRECTORY}/src/cfi"                             # User file directory

TOOLCHAIN = "riscv-none-elf" # Toolchain, can be changed. Must be included in the PATH or specified here
ISA = "rv32imc_zicsr"        # ISA, extensions can be changed for other needs
CFLAGS = (
    "-W -Wall -Wextra -Werror -Wundef -Wshadow -pedantic -Wdouble-promotion -ffixed-a7 "
    f"-fno-common -Wconversion -march={ISA} -mabi=ilp32 -O1 -ffunction-sections "
    f"-fdata-sections -fno-builtin-printf -I{SOURCES_DIRECTORY} -I{DIRECTORY}/esp32c3") # GCC flags for building
LINKFLAGS = f"-T{DIRECTORY}/esp32c3/link.ld -nostdlib -nostartfiles -Wl,--gc-sections"  # Linker flags
SOURCES = f"{DIRECTORY}/esp32c3/boot.c {SOURCES_DIRECTORY}/main.c {SOURCES_DIRECTORY}/intr_vector_table.c {SOURCES_DIRECTORY}/shadow_stack.c {SOURCES_DIRECTORY}/cfg.c" # Needed C files 
LOGGING_SOURCE = F"{SOURCES_DIRECTORY}/ij_logger.c" # C file for register logging

ESPUTIL = f"{DIRECTORY}/esputil/esputil" # Espressif utils to flash to the board
FLASH_ADDR = 0                           # Flash starting address
OUTPUT = "firmware"                      # Name for the output file

# Function to run a command
def run_command(command, capture_output=False, output_file=None):
    command_list = shlex.split(command)
    try:
        with (output_file.open("w") if output_file else open(os.devnull, "w")) as output_file:
            with open(f"{DIRECTORY}/toolsExtra/logs.log", "a") as error_file:
                subprocess.run(command_list, stdout=output_file if capture_output else None, stderr=error_file, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error: Command '{command}' failed with error: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        return  
  
def extractor(command):
    command_list = shlex.split(command)
    output_lines = []

    try:
        # Start the subprocess with Popen
        with subprocess.Popen(command_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1, universal_newlines=True) as process:
                while True:                              # Poll the process for new output until it terminates
                    out = process.stdout.readline()
                    if "Source" in out:
                        output_lines.append(out.strip()) # If the line is regarding register logging, store output
                    if "Execution terminated" in out:    # Break loop if process has finished
                        break

                process.terminate()                      # Terminate the process
                process.wait()                           # Wait for the process to terminate
                return "\n".join(output_lines)
    except subprocess.CalledProcessError as e:
        print(f"Error: Command '{command}' failed with error: {e}")
        sys.exit(1)

def run_command_with_time(command):
    command_list = shlex.split(command)
    start_time = time.time()  # Start time recording

    try:
        # Start the subprocess with Popen
        with subprocess.Popen(command_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1) as process:
            while True:
                output = process.stdout.readline()   # Read a line from the process' output
                if output:                           # Only print if there is output
                    print(output.strip())            # Print the output
                
                if "Execution terminated" in output: # Check for "Execution terminated" in the output
                    break                            # Exit the loop if termination message is found

            elapsed_time = time.time() - start_time  # Calculate elapsed time
            process.terminate()                      # Terminate the process
            process.wait()                           # Wait for the process to terminate
    except subprocess.CalledProcessError as e:
        print(f"Error: Command '{command}' failed with error: {e}")
        sys.exit(1)

    return elapsed_time

# Function to clear files before creating new ones
def clear():
    print("Clearing files...")
    tools_extra_dir = Path(f"{DIRECTORY}/toolsExtra")
    files_to_delete = [f for f in tools_extra_dir.glob("*") if f.suffix in {".bin", ".elf", ".s", ".log"}]
    for file in files_to_delete:
        file.unlink()

# Function to flash and monitor the application
def flash(extract = False):
    run_command(f"{ESPUTIL} flash {FLASH_ADDR} {OUTPUT}.bin") # Flash the executable onto the board
    if extract:
        return extractor(f"{ESPUTIL} monitor")                # If extract is true capture output to compute CFG
    else:
        return run_command_with_time(f"{ESPUTIL} monitor")    # Else monitor the application to see I/O

# Function to build the executable
def build():
    clear() # Clear files

    # Retrieve user imported source files
    print("Retrieving user source files...")
    extra_sources = " ".join([f"{SOURCES_DIRECTORY}/usercode/{file}" for file in os.listdir(f"{SOURCES_DIRECTORY}/usercode") if file.endswith(".c")])

    print(f"Creating {OUTPUT}.elf file...")
    run_command(f"{TOOLCHAIN}-gcc {CFLAGS} {SOURCES} {extra_sources} {LINKFLAGS} -o {OUTPUT}.elf") # Creates .elf file

    print(f"Creating {OUTPUT}.bin file...")
    run_command(f"{ESPUTIL} mkbin {OUTPUT}.elf {OUTPUT}.bin") # Creates .bin file

    print(f"Creating {OUTPUT}.s file...")
    run_command(f"{TOOLCHAIN}-objdump -D {OUTPUT}.elf", capture_output=True, output_file=Path(f"{DIRECTORY}/toolsExtra/{OUTPUT}.s")) # Creates .s file (for inspections)

    print(f"Files built successfully")
    return os.path.getsize(f"{DIRECTORY}/toolsExtra/{OUTPUT}.bin")

# Function to instrument and build the executable
def secure_build():
    clear() # Clear files

    print("Retrieving user source files...")

    extra_sources = " ".join([f"{SOURCES_DIRECTORY}/usercode/{file}" for file in os.listdir(f"{SOURCES_DIRECTORY}/usercode") if file.endswith(".c")]) # Retrieve user imported source files

    files_to_instrument = [source.replace("src/cfi/usercode/", "toolsExtra/").replace(".c", ".s") for source in extra_sources.split()] # Retrieve files to instrument
    
    run_command(f"{TOOLCHAIN}-gcc -S {CFLAGS} {SOURCES} {LOGGING_SOURCE} {extra_sources}") # Creates individual assembly files
   
    start = time.time()
    ind_jumps = instrumenter.instrument(files_to_instrument, CFGLogging=True) # Instrument the files with logging capabilities and retrieve the existence of indirect jumps
    instrumentation_time = time.time() - start

    all_assembly_files = " ".join([f"{DIRECTORY}/toolsExtra/{file}" for file in os.listdir(f"{DIRECTORY}/toolsExtra") if file.endswith(".s") and file != "ij_logger.s"]) # Retrieve all assembly files to be assembled and linked
    
    print(f"Creating {OUTPUT}.elf file...")
    run_command(f"{TOOLCHAIN}-gcc {all_assembly_files} {DIRECTORY}/toolsExtra/ij_logger.s  {LINKFLAGS} -o {OUTPUT}.elf") # Creates .elf file

    print(f"Creating {OUTPUT}.bin file...")
    run_command(f"{ESPUTIL} mkbin {OUTPUT}.elf {OUTPUT}.bin") # Creates .bin file

    print(f"Creating {OUTPUT}.s file...")
    run_command(f"{TOOLCHAIN}-objdump -D {OUTPUT}.elf", capture_output=True, output_file=Path(f"{DIRECTORY}/toolsExtra/{OUTPUT}.s")) # Creates .s file (for inspections)

    # If there are indirect jumps run program to detect jump addresses
    start = time.time()
    if ind_jumps:
        output_string = flash(extract=True)
        blocks = CFGextractor.find_blocks(OUTPUT + ".s")

        run_command(f"{TOOLCHAIN}-gcc -S {CFLAGS} {SOURCES} {extra_sources}")                                                            # Creates individual assembly files
        instrumenter.instrument(files_to_instrument)                                                                                     # Instrument the files
        run_command(f"{TOOLCHAIN}-gcc {all_assembly_files} {LINKFLAGS} -o {OUTPUT}.elf")                                                 # Creates .elf file
        run_command(f"{ESPUTIL} mkbin {OUTPUT}.elf {OUTPUT}.bin")                                                                        # Creates .bin file
        run_command(f"{TOOLCHAIN}-objdump -D {OUTPUT}.elf", capture_output=True, output_file=Path(f"{DIRECTORY}/toolsExtra/{OUTPUT}.s")) # Creates .s file (for inspections)
    else:
        output_string = ""
        blocks = []
    simulation_time = time.time() - start

    start = time.time()
    CFGextractor.extract(output_string, blocks, OUTPUT + ".s") # Extract and inject CFG 
    cfg_time = time.time() - start

    run_command(f"{TOOLCHAIN}-gcc -S {CFLAGS} {SOURCES} {extra_sources}")                                                            # Creates individual assembly files
    instrumenter.instrument(files_to_instrument)                                                                                     # Instrument the files
    run_command(f"{TOOLCHAIN}-gcc {all_assembly_files} {LINKFLAGS} -o {OUTPUT}.elf")                                                 # Creates .elf file
    run_command(f"{ESPUTIL} mkbin {OUTPUT}.elf {OUTPUT}.bin")                                                                        # Creates .bin file
    run_command(f"{TOOLCHAIN}-objdump -D {OUTPUT}.elf", capture_output=True, output_file=Path(f"{DIRECTORY}/toolsExtra/{OUTPUT}.s")) # Creates .s file (for inspections)

    print("Clearing assembly files...")
    run_command(f"rm {all_assembly_files} {DIRECTORY}/toolsExtra/ij_logger.s") # Removes assembly files

    instrumenter.restore_vector_table()
 
    print(f"Files instrumented and built successfully")
    return os.path.getsize(f"{DIRECTORY}/toolsExtra/{OUTPUT}.bin"), instrumentation_time, simulation_time, cfg_time

def main():
    if len(sys.argv) < 2 or sys.argv[1] in {"help", "-help", "--help", "h", "-h", "--h"}:
        print("""\nUsage: python3 flasher.py [build | run | secure-build | secure-run | clear]\n
    - Use build to build the executable\n
    - Use run to build and run the executable\n
    - Use secure-build to instrument and build the executable\n
    - Use secure-run to instrument, build and run the executable\n
    - Use clear to remove .elf, .bin, .s, and .log files\n""")
        sys.exit(1)
    
    command = sys.argv[1]

    if command == "build":
        bin_size = build()
        print(f"Binary size: {bin_size} Byte")
    elif command == "run":
        bin_size = build()
        print("Flashing the program...\n")
        time = flash()
        print(f"Binary size: {bin_size} Byte\nExecution time: {time}s")
    elif command == "secure-build":
        bin_size = secure_build()
        print(f"Binary size: {bin_size} Byte")
    elif command == "secure-run":
        bin_size, instrumentation_time, simulation_time, cfg_time = secure_build()
        print("Flashing the instrumented program...\n")
        run_time = flash()
        print(f"Binary size: {bin_size} Byte\nExecution time: {run_time}s\nInstrumentation time: {instrumentation_time}s\nSimulation time: {simulation_time}s\nCFG extraction time: {cfg_time}s")
    elif command == "clear":
        clear()
    else:
        print("Error: Invalid operation specified.")
        sys.exit(1)

if __name__ == "__main__":
    main()
