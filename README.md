# RISC-V-TE

This project aims at implementing Control Flow Integrity for bare-metal RISC-V based micro-controller (project based on the bare-metal infrastructure provided by [Sergey Lyubka](https://github.com/cpq/mdk/tree/main)).
The project provides secure U-mode code execution and performs controls on both return and jump instructions.

Return instructions are checked thanks to a shadow stack.
Jump instructions are checked thanks to the Control Flow Graph.

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
