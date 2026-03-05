# 😝😝😝 RudzaniOS

## Author: Ndaedzo Rudzani Brandon Mudau

RudzaniOS is a custom, minimal 32-bit protected-mode operating system built entirely from scratch. It features its own bootloader, kernel, memory management, virtual filesystem, and an interactive shell loaded with custom tools and arcade games.

## ✨ Features

- **Custom Bootloader** (16-bit to 32-bit protected mode translation via LBA)
- **Interactive Shell** with the following built-in commands:
  - `help`      - Show the help message
  - `clear`     - Clear the screen
  - `about`     - Display OS information
  - `uptime`    - Show ticks since boot
  - `meminfo`   - Show memory usage and page allocation
  - `malloc`    - Test heap allocation
  - `ls`        - List files in the current directory
  - `cd <dir>`  - Change directory
  - `pwd`       - Print working directory
  - `mkdir <d>` - Create a new directory
  - `touch <f>` - Create an empty file
  - `show_me <f>` - Print file contents to the screen
  - `echo`      - Echo text back (or print file if argument is a file)
  - `cp <src> <dst>` - Copy a file
  - `cut <src> <dst>` - Move (cut) a file
  - `rm <f/d>`  - Remove a file or directory
  - `nda <f>`   - A fully functional terminal text editor (nano-style)
  - `sysmon`    - Dynamic system monitor (tracking CPU info and RAM usage, top-style)
  - `calc <eq>` - Mathematical calculator (supports `sin`, `cos`, `tan`, `sqrt`, `log`, `^`, parentheses, PEMDAS)
  - `ps`        - List active running processes
  - `lets_play` - Launch the RudzaniOS Arcade Machine
  - `reboot`    - Reboot the system
  - `halt`      - Halt the CPU safely
- **Virtual File System** (VFS) with an initial Ramdisk (initrd) and dynamic slot reuse for directories/files.
- **RudzaniOS Arcade Machine** (`lets_play`):
  - 🍎 **Snake**
  - 🏓 **Pong** (vs AI (Honestly ist just the computer BUT for 2026 markets sake we will call it an AI lol))
  - 🧱 **Tetris**

## 🛠️ Prerequisites

To build and run RudzaniOS, you need the following tools (the MSYS2/MinGW toolchain is recommended on Windows):

- `gcc` (MinGW 32-bit cross-compiler or similar)
- `nasm` (Assembler)
- `ld`, `objcopy` (Binutils)
- `qemu-system-x86_64` (For emulation)

## 🏗️ Building and Running

The project includes a unified `build.sh` script to handle assembling the bootloader, compiling the C kernel, linking the PE binary, and generating the final disk image.

```bash
# Build the OS image (os.iso)
./build.sh

# Build the project and launch it instantly via QEMU
./build.sh run

# Clean all build artifacts
./build.sh clean
```

## 📦 Running in VirtualBox

If you want to run RudzaniOS on VirtualBox instead of QEMU, you need to convert the raw disk image (`os.iso`) into a VirtualBox Disk Image (`.vdi`).

If you have VirtualBox's tools in your system PATH, you can run:

```bash
VBoxManage convertfromraw build\os.iso build\os.vdi --format VDI
```

Alternatively, `build.sh` is configured to automatically attempt converting it via `qemu-img` when building.
