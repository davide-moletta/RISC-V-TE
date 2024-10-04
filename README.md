# RISC-V-TE

This project aims at implementing **Control Flow Integrity** for bare-metal **RISC-V** based micro-controller (project based on the bare-metal infrastructure provided by [Sergey Lyubka](https://github.com/cpq/mdk/tree/main)).
The project provides secure U-mode code execution and performs controls on both jump and return instructions thanks to a Control Flow Graph and a Shadow Stack.

**Note:** This project has been developed on _Espressif's_ [_ESP32-C3-DevKitM-1_](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c3/esp32-c3-devkitm-1/index.html). Thus, it uses _Espressif's utils_ to flash the executable.

## Installation

Clone the repository:

```
git clone https://github.com/davide-moletta/RISC-V-TE.git
```

Depending on which running method (_MAKE_ or _Python_) you wish to use follow the relative instructions.

### Make requirements

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

#### How to run (Make)

Build and run:
```
make -C path/to/RISC-V-TE/src/cfi clean build flash monitor
```

Build:
```
make -C path/to/RISC-V-TE/src/cfi clean build
```

Clean binaries and dumps:
```
make -C path/to/RISC-V-TE/src/cfi clean
```

### Python requirements

Install [_Python_](https://www.python.org/downloads/).

Install the [_riscv-none-elf toolchain_](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack). 
**Note:** Other toolchains may be used but must be configured in the _flasher.py_ file and added to the _$PATH_.

#### How to run (Python)

Use the file _flasher.py_ inside toolsExtra/ to build, instrument, and run the code:
```
python3 flasher.py [operation]
```

Where the operation can be:
- **build**: to build the source code and create the _.bin_, _.elf_ and _.s_ files;
- **run**: to build and flash the executable on the board;
- **secure-build**: to instrument the code and create the _.bin_, _.elf_ and _.s_ files;
- **secure-run**: to build and flash the instrumented executable on the board;
- **clear**: to delete _.bin_, _.elf_, _.s_ and _.log_ files.

**Note:** The file _instrumenter.py_ can be used:
- Through the _flasher.py_ script with **secure-build** and **secure-run** commands;
- Manually by providing the assembly files as input (create the assembly files manually, instrument them with _instrumenter.py_, and finish assembly and linking manually).

To use _instumenter.py_ withut _flasher.py_ run:
```
python3 instrumenter.py [file1.s] [file2.s] ... [fileN.s]
```

### Troubleshooting

If you encounter problems when flashing the code and the file _logs.log_ states:
```
open(/dev/ttyUSB0): -1 (Permission denied)
```
open a terminal and run the following command:
```
sudo chmod a+rw /dev/ttyUSB0
```

**Note:** This command must be run at each computer reboot.
If you wish to avoid doing so, add your user to the _dialout_ and _plugdev_ groups.
To do so run the following commands:
```
sudo usermod -a -G dialout $USER
sudo usermod -a -G plugdev $USER
```
after that, either logout and log in or run:
```
sudo service udev restart
```

## Import code

To ensure the correct functioning of the instrumenter import or create your files under the _path/to/RISC-V-TE/src/cfi/usercode/_ directory.
After that, edit the _user\_entry.c_ file to call the first function of your code.
