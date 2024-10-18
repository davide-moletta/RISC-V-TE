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

If, when flashing the board, you see the output looping on:
```
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021 
rst:0x7 (TG0WDT_SYS_RST),boot:0xc (SPI_FAST_FLASH_BOOT) 
Saved PC:0x40049a42 
SPIWP:0xee mode:QIO, clock div:2 
load:0x3fc88000,len:0xa70 
ets_loader.c 78
```
it means that the flashing procedure has been corrupted.
To address this issue please install [_esptool.py_](https://docs.espressif.com/projects/esptool/en/latest/esp32/) and run the following command:
```
python3 esptool.py --chip esp32c3 --port /dev/ttyUSB0 write_flash --flash_mode dio --flash_freq 40m 0x0 path/to/RISC-V-TE/tools/board_reset.bin 
```

## Import code

To ensure the correct functioning of the instrumenter import or create your files under the _path/to/RISC-V-TE/src/cfi/usercode/_ directory.
After that, edit the _user\_entry.c_ file to call the first function of your code.
