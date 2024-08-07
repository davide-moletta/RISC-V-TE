# RISC-V-TE

This project aims at implementing Control Flow Integrity for bare-metal RISC-V based micro-controller (project based on the bare-metal infrastructure provided by [Sergey Lyubka](https://github.com/cpq/mdk/tree/main)).
The project provides secure U-mode code execution and performs controls on both return and jump instructions.

Return instructions are checked thanks to a shadow stack.
Jump instructions are checked thanks to the Control Flow Graph.

Note that this project has been developed on Espressif's [ESP32-C3-DevKitM-1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c3/esp32-c3-devkitm-1/index.html). Thus, it uses Espressif's utils to flash the exxecutable on the board.

## Requirements

This project requires the [riscv-none-elf toolchain](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack) to work. 
Other toolchains may be used but must be configured either in the python flasher or in the Makefile.

## How to run

Clone the repository:

```
git clone https://github.com/davide-moletta/RISC-V-TE.git
```

Build and run:
```
make -C .../RISC-V-TE/src/shadowstack clean build flash monitor
```

Build:
```
make -C .../RISC-V-TE/src/shadowstack clean build
```

Clean binaries and dumps:
```
make -C .../RISC-V-TE/src/shadowstack clean
```

Optionally it's possible to use the python flasher inside toolExtra/:
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
