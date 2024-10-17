import sys
import subprocess
import os
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
CFLAGS = (
    "-W -Wall -Wextra -Werror -Wundef -Wshadow -pedantic -Wdouble-promotion -ffixed-a7 "
    "-fno-common -Wconversion -march=rv32imc_zicsr -mabi=ilp32 -O1 -ffunction-sections "
    f"-fdata-sections -fno-builtin-printf -I{SOURCES_DIRECTORY} -I{DIRECTORY}/esp32c3") # GCC flags for building
LINKFLAGS = f"-T{DIRECTORY}/esp32c3/link.ld -nostdlib -nostartfiles -Wl,--gc-sections"  # Linker flags
SOURCES = f"{DIRECTORY}/esp32c3/boot.c {SOURCES_DIRECTORY}/main.c {SOURCES_DIRECTORY}/intr_vector_table.c {SOURCES_DIRECTORY}/shadow_stack.c {SOURCES_DIRECTORY}/cfg.c {SOURCES_DIRECTORY}/ij_logger.c" # Needed C files 

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
        sys.exit(0)  
  
def extractor(command):
    command_list = shlex.split(command)
    output_lines = []

    try:
        # Start the command with Popen
        with subprocess.Popen(command_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE,text=True, bufsize=1, universal_newlines=True) as process:
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
        run_command(f"{ESPUTIL} monitor")                     # Else monitor the application to see I/O

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

    print("Files built successfully\n")

# Function to instrument and build the executable
def secure_build():
    clear() # Clear files

    print("Retrieving user source files...")

    extra_sources = " ".join([f"{SOURCES_DIRECTORY}/usercode/{file}" for file in os.listdir(f"{SOURCES_DIRECTORY}/usercode") if file.endswith(".c")]) # Retrieve user imported source files

    files_to_instrument = [source.replace("src/cfi/usercode/", "toolsExtra/").replace(".c", ".s") for source in extra_sources.split()] # Retrieve files to instrument
    
    run_command(f"{TOOLCHAIN}-gcc -S {CFLAGS} {SOURCES} {extra_sources}") # Creates individual assembly files
   
    ind_jumps = instrumenter.instrument(files_to_instrument, CFGLogging=True) # Instrument the files with logging capabilities and retrieve the existence of indirect jumps

    all_assembly_files = " ".join([f"{DIRECTORY}/toolsExtra/{file}" for file in os.listdir(f"{DIRECTORY}/toolsExtra") if file.endswith(".s")]) # Retrieve all assembly files to be assembled and linked
  
    print(f"Creating {OUTPUT}.elf file...")
    run_command(f"{TOOLCHAIN}-gcc {all_assembly_files} {LINKFLAGS} -o {OUTPUT}.elf") # Creates .elf file

    print(f"Creating {OUTPUT}.bin file...")
    run_command(f"{ESPUTIL} mkbin {OUTPUT}.elf {OUTPUT}.bin") # Creates .bin file

    print(f"Creating {OUTPUT}.s file...")
    run_command(f"{TOOLCHAIN}-objdump -D {OUTPUT}.elf", capture_output=True, output_file=Path(f"{DIRECTORY}/toolsExtra/{OUTPUT}.s")) # Creates .s file (for inspections)

    # If there are indirect jumps run program to detect jump addresses
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

    CFGextractor.extract(output_string, blocks, OUTPUT + ".s") # Extract and inject CFG 

    run_command(f"{TOOLCHAIN}-gcc -S {CFLAGS} {SOURCES} {extra_sources}")                                                            # Creates individual assembly files
    instrumenter.instrument(files_to_instrument)                                                                                     # Instrument the files
    run_command(f"{TOOLCHAIN}-gcc {all_assembly_files} {LINKFLAGS} -o {OUTPUT}.elf")                                                 # Creates .elf file
    run_command(f"{ESPUTIL} mkbin {OUTPUT}.elf {OUTPUT}.bin")                                                                        # Creates .bin file
    run_command(f"{TOOLCHAIN}-objdump -D {OUTPUT}.elf", capture_output=True, output_file=Path(f"{DIRECTORY}/toolsExtra/{OUTPUT}.s")) # Creates .s file (for inspections)

    print("Clearing assembly files...")
    run_command(f"rm {all_assembly_files}") # Removes assembly files

    instrumenter.restore_vector_table()

    print("Files instrumented and built successfully\n")

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
        build()
    elif command == "run":
        build()
        print("Flashing the program...\n")
        flash()
    elif command == "secure-build":
        secure_build()
    elif command == "secure-run":
        secure_build()
        print("Flashing the instrumented program...\n")
        flash()
    elif command == "clear":
        clear()
    else:
        print("Error: Invalid operation specified.")
        sys.exit(1)

if __name__ == "__main__":
    main()
