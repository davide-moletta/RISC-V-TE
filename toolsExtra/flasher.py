import sys, subprocess, os, shlex, instrumenter

# Define constants
curr_dir = subprocess.run(['pwd'], capture_output = True, text = True) # Retrieve the current directory
DIRECTORY = os.path.dirname(curr_dir.stdout.strip()) # Current working directory
SOURCES_DIRECTORY = f"{DIRECTORY}/src/shadowstack" # User file directory

TOOLCHAIN = "riscv-none-elf" # Toolchain, can be changed. Must be included in the PATH
CFLAGS = (
    "-W -Wall -Wextra -Werror -Wundef -Wshadow -pedantic -Wdouble-promotion "
    "-fno-common -Wconversion -march=rv32imc_zicsr -mabi=ilp32 -O1 -ffunction-sections "
    f"-fdata-sections -fno-builtin-printf -I{SOURCES_DIRECTORY} -I{DIRECTORY}/esp32c3") # GCC flags for building
LINKFLAGS = f"-T{DIRECTORY}/esp32c3/link.ld -nostdlib -nostartfiles -Wl,--gc-sections" # Linker flags
SOURCES = f"{DIRECTORY}/esp32c3/boot.c {SOURCES_DIRECTORY}/main.c {SOURCES_DIRECTORY}/intr_vector_table.c {SOURCES_DIRECTORY}/shadow_stack.c" # Needed C files 

ESPUTIL = f"{DIRECTORY}/esputil/esputil" # Espressif utils to flash to the board
FLASH_ADDR = 0 # Flash starting address

def run_command(command, capture_output=False):
    command_list = shlex.split(command)
    if(capture_output):
        with open(f"{DIRECTORY}/toolsExtra/{OUTPUT}.s", "w") as output_file:
            with open(f"{DIRECTORY}/toolsExtra/warnings.log", "a") as error_file:
                subprocess.run(command_list, stdout=output_file, stderr=error_file, check=True)
    else:
        try:
            with open(f"{DIRECTORY}/toolsExtra/warnings.log", "a") as error_file:
                subprocess.run(command_list, stderr=error_file, check=True)
        except KeyboardInterrupt:
            sys.exit(0)    

def clear():
    print("Clearing previous files...")
    files_to_delete = " ".join([f"{DIRECTORY}/toolsExtra/{file}" for file in os.listdir(f"{DIRECTORY}/toolsExtra") 
         if file.endswith(('.bin', '.elf', '.s', ".log"))])
    if files_to_delete:
        run_command(f"rm {files_to_delete}", capture_output=False)

def flash():
    run_command(f"{ESPUTIL} flash {FLASH_ADDR} {OUTPUT}.bin", capture_output=False)
    run_command(f"{ESPUTIL} monitor", capture_output=False)

def build():
    clear()

    print("Retrieving user source files...")
    extra_sources = " ".join([f"{SOURCES_DIRECTORY}/usercode/{file}" for file in os.listdir(f"{SOURCES_DIRECTORY}/usercode") if file.endswith(".c")])

    print("Creating .elf file...")
    run_command(f"{TOOLCHAIN}-gcc {CFLAGS} {SOURCES} {extra_sources} {LINKFLAGS} -o {OUTPUT}.elf", capture_output=False)

    print("Creating .bin file...")
    run_command(f"{ESPUTIL} mkbin {OUTPUT}.elf {OUTPUT}.bin", capture_output=False)

    print("Creating .s file...")
    run_command(f"{TOOLCHAIN}-objdump -D {OUTPUT}.elf", capture_output=True)

    print("Files built successfully")

def secure_build():
    clear()

    print("Retrieving user source files...")
    extra_sources = " ".join([f"{SOURCES_DIRECTORY}/usercode/{file}" for file in os.listdir(f"{SOURCES_DIRECTORY}/usercode") if file.endswith(".c")])
    files_to_instrument = [source.replace("src/shadowstack/usercode/", "toolsExtra/").replace(".c", ".s") for source in extra_sources.split()]
    
    print("Creating individual assembly files...")
    assembly_command = f"{TOOLCHAIN}-gcc -S {CFLAGS} {SOURCES} {extra_sources}"
    run_command(assembly_command, capture_output=False)

    # Instrument code here
    # instrumenter.instrument(files_to_instrument)

    all_assembly_files = " ".join([f"{DIRECTORY}/toolsExtra/{file}" for file in os.listdir(f"{DIRECTORY}/toolsExtra") if file.endswith(".s")])

    print("Creating .elf file...")
    run_command(f"{TOOLCHAIN}-gcc {all_assembly_files} {LINKFLAGS} -o {OUTPUT}.elf", capture_output=False)

    print("Creating .bin file...")
    run_command(f"{ESPUTIL} mkbin {OUTPUT}.elf {OUTPUT}.bin", capture_output=False)

    print("Creating .s file...")
    run_command(f"{TOOLCHAIN}-objdump -D {OUTPUT}.elf", capture_output=True)

    # print("Clearing assembly files...")
    # run_command(f"rm {all_assembly_files}", capture_output=False) # Removes assembly files after using them to build the .elf file

    print("Files instrumented and built successfully")

def main():
    if (len(sys.argv) < 3 or "help" in sys.argv or "-help" in sys.argv or "--help" in sys.argv or "h" in sys.argv or "-h" in sys.argv or "--h" in sys.argv): 
        print("This file is used to compile, instrument and run the code.\n"
              "Input parameters should be provided in the following order.\n"
              "python3 flasher.py:\n"
              "\t[output file name]: The name of the resulting .bin and .elf files\n"
              "\t[build | run | secure-build | secure-run]:\n"
              "\t\t- build just builds all the files and creates the .bin and .elf files\n"
              "\t\t- run builds and run all the files without instrumentation\n"
              "\t\t- secure-build instruments the code and creates the .bin and .elf files\n"
              "\t\t- secure-run instruments the code, then builds and run it")
        sys.exit(1)
    
    global OUTPUT
    OUTPUT = sys.argv[1]

    match sys.argv[2]:
        case "build":
            build()
        case "run":
            build()
            print("Flashing the program...")
            flash()
        case "secure-build":
            secure_build()
        case "secure-run":
            secure_build()
            print("Flashing the instrumented program...")
            flash()
        case _:
            print("The specified operation does not exists correct\n")
            sys.exit(1)

if __name__ == "__main__":
    main()
