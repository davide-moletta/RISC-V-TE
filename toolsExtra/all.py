import sys, subprocess, os, shlex

if ("help" in sys.argv or "-help" in sys.argv or "--help" in sys.argv or "h" in sys.argv or "-h" in sys.argv or "--h" in sys.argv): 
    print("This file is used to compile, instrument and run the code.\n"
          "Input parameters should be provided in the following order.\n"
          "python3 all.py:\n"
          "\t[output file name]: The name of the resulting .bin and .elf files\n"
          "\t[build | run | secure-build | secure-run]:\n"
          "\t\t- build just builds all the files and creates the .bin and .elf files\n"
          "\t\t- run builds and run all the files without instrumentation\n"
          "\t\t- secure-build instruments the code and creates the .bin and .elf files\n"
          "\t\t- secure-run instruments the code, then builds and run it")
    exit(0)

curr_dir = subprocess.run(['pwd'], capture_output = True, text = True) # Retrieve the current directory
DIRECTORY = os.path.dirname(curr_dir.stdout.strip()) # Current working directory
SOURCES_DIRECTORY = DIRECTORY + "/src/shadowstack" # User file directory

TOOLCHAIN = "riscv-none-elf" # Toolchain, can be changed. Must be included in the PATH
CFLAGS = ("-W -Wall -Wextra -Werror -Wundef -Wshadow -pedantic -Wdouble-promotion "
          "-fno-common -Wconversion -march=rv32imc_zicsr -mabi=ilp32 -O1 -ffunction-sections "
          f"-fdata-sections -fno-builtin-printf -I{SOURCES_DIRECTORY} -I{DIRECTORY}/esp32c3") # GCC flags for building
LINKFLAGS = f"-T{DIRECTORY}/esp32c3/link.ld -nostdlib -nostartfiles -Wl,--gc-sections" # Linker flags
SOURCES = f"{DIRECTORY}/esp32c3/boot.c {SOURCES_DIRECTORY}/main.c {SOURCES_DIRECTORY}/intr_vector_table.c {SOURCES_DIRECTORY}/shadow_stack.c" # Needed C files 

ESPUTIL = f"{DIRECTORY}/esputil/esputil" # Espressif utils to flash to the board
FLASH_ADDR = 0 # Flash starting address
OUTPUT = sys.argv[1] # Name of the output file

def run_command(command, new_file):
    if(new_file):
        command_list = shlex.split(command)
        with open(f"{DIRECTORY}/toolsExtra/{OUTPUT}.S", "w") as output_file:
            with open(f"{DIRECTORY}/toolsExtra/warnings.log", "a") as error_file:
                subprocess.run(command_list, stdout=output_file, stderr=error_file, check=True)
    else:
        command_list = shlex.split(command)
        with open(f"{DIRECTORY}/toolsExtra/warnings.log", "a") as error_file:
            subprocess.run(command_list, stderr=error_file, check=True)

# Removes old .elf, .bin and .S files before building new ones
def clear():
    print("Clearing previous files...")
    files_to_delete = " ".join([f"{DIRECTORY}/toolsExtra/{file}" for file in os.listdir(DIRECTORY + "/toolsExtra") if file.endswith(('.bin', '.elf', '.S', ".log"))])
    if (files_to_delete != ""):
        remove_command = f"rm {files_to_delete}"
        command_list = shlex.split(remove_command)
        subprocess.run(command_list, check=True)
        print("Files deleted successfully")
    else:
        print("No files to delete")

def build():
    clear() # Remove files
    print("Retrieving user source files...")
    # Retrieve user files in the sub folder
    extra_sources = " ".join([f"{SOURCES_DIRECTORY}/usercode/{file}" for file in os.listdir(SOURCES_DIRECTORY + "/usercode") if file.endswith(".c")])

    print("Creating the .elf file...")
    elf_command = f"{TOOLCHAIN}-gcc {CFLAGS} {SOURCES} {extra_sources} {LINKFLAGS} -o {OUTPUT}.elf" # Builds the .elf file with toolchain
    run_command(elf_command, False)

    print("Creating the .bin file...")
    bin_command = f"{ESPUTIL} mkbin {OUTPUT}.elf {OUTPUT}.bin" # Creates the .bin file from the .elf file with Espressif utils
    run_command(bin_command, False)

    print("Creating the .S file...")
    dump_command = f"{TOOLCHAIN}-objdump -D {OUTPUT}.elf" # Creates the .S file (for inspections)
    run_command(dump_command, True)

    print("Files built successfully")

def run():
    build()
    print("Running the executable...")

def secure_build():
    print("Securely building the sources...")

def secure_run():
    secure_build()
    print("Securely running the executable...")

match sys.argv[2]:
    case "build":
        build()
    case "run":
        run()
    case "secure-build":
        secure_build()
    case "secure-run":
        secure_run()
    case _:
        print("The specified operation does not exists correct\n")
        exit(0)
