# RISC-V-TE

This project aims at implementing **Control Flow Integrity** for bare-metal **RISC-V** based micro-controller (project based on the bare-metal infrastructure provided by [Sergey Lyubka](https://github.com/cpq/mdk/tree/main)).
The project provides secure U-mode code execution and performs controls on return instructions.

Return instructions are checked thanks to a shadow stack.

Note that this project has been developed on _Espressif's_ [_ESP32-C3-DevKitM-1_](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c3/esp32-c3-devkitm-1/index.html). Thus, it uses _Espressif's utils_ to flash the exxecutable.

## Installation

Clone the repository:

```
git clone https://github.com/davide-moletta/RISC-V-TE.git
```

Depending on which running method (_MAKE_ or _Python_) you whish to use follow the relative instructions.

### MAKE REQUIREMENTS

Install _make_:
```
sudo apt install make
```

Install [_Docker_](https://docs.docker.com/engine/install/ubuntu/).

Run the following commands:
```
export MDK=/path/to/RISC-V-TE
export ARCH=esp32c3
export PORT=/dev/ttyUSB0
```

### HOW TO RUN WITH MAKE

Build and run:
```
make -C path/to/RISC-V-TE/src/shadowstack clean build flash monitor
```

Build:
```
make -C path/to/RISC-V-TE/src/shadowstack clean build
```

Clean binaries and dumps:
```
make -C path/to/RISC-V-TE/src/shadowstack clean
```

### PYTHON REQUIREMENTS

Install [_Python_](https://www.python.org/downloads/).

Install the [_riscv-none-elf toolchain_](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack). 
Other toolchains may be used but must be configured in the _flasher.py_ file and added to the _PATH_.

### HOW TO RUN WITH PYTHON

Use the file _flasher.py_ inside toolsExtra/ to build, instrument and run the code:
```
python3 flasher.py [output file name] [operation]
```
where the operation can be:
- build: to build the source code and create the .bin, .elf and .s files
- run: to build and flash the executable on the board
- secure-build: to instrument the code and create the .bin, .elf and .s files
- secure-run: to build and flash the instrumented executable on the board
- clear: to delete .bin, .elf, .s and .log files

Note that the file instrumenter.py can be used:
- through the flasher.py script with secure-build and secure-run commands
- manually by providing the assembly files as input (create the assembly files manually, instrument them with instrumenter.py and finish assembly and linking manually)

To use the instumenter file withut the flasher run:
```
python3 instrumenter.py [file1.s] [file2.s] ... [fileN.s]
```
