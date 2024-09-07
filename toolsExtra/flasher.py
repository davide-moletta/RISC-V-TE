import sys
import subprocess
import os
import shlex
import instrumenter
from pathlib import Path

#############
# CONSTANTS #
#############
curr_dir = subprocess.run(['pwd'], capture_output = True, text = True) # Retrieve the current directory
DIRECTORY = os.path.dirname(curr_dir.stdout.strip())                   # Current working directory
SOURCES_DIRECTORY = f"{DIRECTORY}/src/shadowstack"                     # User file directory

TOOLCHAIN = "riscv-none-elf" # Toolchain, can be changed. Must be included in the PATH or specified here
CFLAGS = (
    "-W -Wall -Wextra -Werror -Wundef -Wshadow -pedantic -Wdouble-promotion -ffixed-a7 "
    "-fno-common -Wconversion -march=rv32imc_zicsr -mabi=ilp32 -O1 -ffunction-sections "
    f"-fdata-sections -fno-builtin-printf -I{SOURCES_DIRECTORY} -I{DIRECTORY}/esp32c3") # GCC flags for building
LINKFLAGS = f"-T{DIRECTORY}/esp32c3/link.ld -nostdlib -nostartfiles -Wl,--gc-sections" # Linker flags
SOURCES = f"{DIRECTORY}/esp32c3/boot.c {SOURCES_DIRECTORY}/main.c {SOURCES_DIRECTORY}/intr_vector_table.c {SOURCES_DIRECTORY}/shadow_stack.c" # Needed C files 

ESPUTIL = f"{DIRECTORY}/esputil/esputil" # Espressif utils to flash to the board
FLASH_ADDR = 0                           # Flash starting address

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

# Function to clear files before creating new ones
def clear():
    print("Clearing files...")
    tools_extra_dir = Path(f"{DIRECTORY}/toolsExtra")
    files_to_delete = [f for f in tools_extra_dir.glob("*") if f.suffix in {".bin", ".elf", ".s", ".log"}]
    for file in files_to_delete:
        file.unlink()

# Function to flash and monitor the application
def flash(output_name):
    run_command(f"{ESPUTIL} flash {FLASH_ADDR} {output_name}.bin") # Flash the executable onto the board
    run_command(f"{ESPUTIL} monitor")                              # Monitor the application to see I/O

# Function to build the executable
def build(output_name):
    clear() # Clear files

    # Retrieve user imported source files
    print("Retrieving user source files...")
    extra_sources = " ".join([f"{SOURCES_DIRECTORY}/usercode/{file}" for file in os.listdir(f"{SOURCES_DIRECTORY}/usercode") if file.endswith(".c")])

    print("Creating .elf file...")
    run_command(f"{TOOLCHAIN}-gcc {CFLAGS} {SOURCES} {extra_sources} {LINKFLAGS} -o {output_name}.elf") # Creates .elf file

    print("Creating .bin file...")
    run_command(f"{ESPUTIL} mkbin {output_name}.elf {output_name}.bin") # Creates .bin file

    print("Creating .s file...")
    run_command(f"{TOOLCHAIN}-objdump -D {output_name}.elf", capture_output=True, output_file=Path(f"{DIRECTORY}/toolsExtra/{output_name}.s")) # Creates .s file (for inspections)

    print("Files built successfully")

# Function to instrument and build the executable
def secure_build(output_name):
    clear() # Clear files

    print("Retrieving user source files...")
    # Retrieve user imported source files
    extra_sources = " ".join([f"{SOURCES_DIRECTORY}/usercode/{file}" for file in os.listdir(f"{SOURCES_DIRECTORY}/usercode") if file.endswith(".c")])

    # Retrieve files to instrument
    files_to_instrument = [source.replace("src/shadowstack/usercode/", "toolsExtra/").replace(".c", ".s") for source in extra_sources.split()]
    
    print("Creating individual assembly files...")
    run_command(f"{TOOLCHAIN}-gcc -S {CFLAGS} {SOURCES} {extra_sources}") # Creates individual assembly files

    # Instrument the files
    print("Instrumenting code...")
    instrumenter.instrument(files_to_instrument) 

    # Retrieve all assembly files to be assembled and linked
    all_assembly_files = " ".join([f"{DIRECTORY}/toolsExtra/{file}" for file in os.listdir(f"{DIRECTORY}/toolsExtra") if file.endswith(".s")])
  
    print("Creating .elf file...")
    run_command(f"{TOOLCHAIN}-gcc {all_assembly_files} {LINKFLAGS} -o {output_name}.elf") # Creates .elf file

    print("Creating .bin file...")
    run_command(f"{ESPUTIL} mkbin {output_name}.elf {output_name}.bin") # Creates .bin file

    print("Creating .s file...")
    run_command(f"{TOOLCHAIN}-objdump -D {output_name}.elf", capture_output=True, output_file=Path(f"{DIRECTORY}/toolsExtra/{output_name}.s")) # Creates .s file (for inspections)

    # print("Clearing assembly files...")
    # run_command(f"rm {all_assembly_files}") # Removes assembly files

    print("Files instrumented and built successfully")


def main():
    if len(sys.argv) < 2 or sys.argv[1] in {"help", "-help", "--help", "h", "-h", "--h"}:
        print("Usage: python3 flasher.py [output file name] [build | run | secure-build | secure-run | clear (no need for output file name)]")
        sys.exit(1)
    
    command = sys.argv[1]
    
    if command == "clear":
        clear()
        sys.exit(0)
    
    if len(sys.argv) < 3:
        print("Error: Missing parameters. Provide an output file name and a command")
        sys.exit(1)
    
    output_name = sys.argv[1]
    command = sys.argv[2]

    if command == "build":
        build(output_name)
    elif command == "run":
        build(output_name)
        print("Flashing the program...")
        flash(output_name)
    elif command == "secure-build":
        secure_build(output_name)
    elif command == "secure-run":
        secure_build(output_name)
        print("Flashing the instrumented program...")
        flash(output_name)
    elif command == "clear":
        clear()
    else:
        print("Error: Invalid operation specified.")
        sys.exit(1)

if __name__ == "__main__":
    main()
