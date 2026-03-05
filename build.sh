#!/bin/bash
# ============================================================
#  RudzaniOS Build Pipeline
# ============================================================
#  Usage:
#    ./build.sh          Build the OS image (os.img)
#    ./build.sh run      Build + launch in QEMU
#    ./build.sh clean    Remove all build artifacts
#    ./build.sh debug    Build + launch QEMU in debug mode
# ============================================================

set -e

# ------ Toolchain ------
CC="gcc"
LD="ld"
OBJCOPY="objcopy"
ASM="nasm"
QEMU="qemu-system-x86_64"
QEMU_IMG="qemu-img"

# ------ Directories ------
BOOT_DIR="boot"
KERNEL_DIR="kernel"
BUILD_DIR="build"

# ------ Compiler Flags ------
CFLAGS="-ffreestanding -m32 -nostdlib -fno-builtin -fno-stack-protector -fno-pie -mno-sse -mno-sse2 -mno-mmx -Wall -Wextra -c"
LDFLAGS="-T linker.ld -nostdlib -mi386pe"

# ------ Source Files ------
KERNEL_C_SOURCES="kernel.c vga.c string.c idt.c keyboard.c timer.c memory.c shell.c paging.c kheap.c fs.c initrd.c editor.c sysmon.c calc.c process.c games.c snake.c pong.c tetris.c"
KERNEL_ASM_SOURCES="kernel_entry.asm isr.asm"

# ============================================================
#  Pipeline
# ============================================================

build() {
	mkdir -p "$BUILD_DIR"

	echo ""
	echo "============================================"
	echo "  RudzaniOS Build Pipeline"
	echo "============================================"
	echo ""

	# ---- Stage 1: Assemble Bootloader ----
	echo "[Stage 1/4] Assembling Bootloader"
	echo "  boot/boot.asm -> build/boot.bin"
	$ASM -f bin "$BOOT_DIR/boot.asm" -o "$BUILD_DIR/boot.bin"
	echo "  Done."
	echo ""

	# ---- Stage 2: Compile Kernel ----
	echo "[Stage 2/4] Compiling Kernel"

	for src in $KERNEL_ASM_SOURCES; do
		obj="${src%.asm}.o"
		echo "  [ASM] kernel/$src -> build/$obj"
		$ASM -f win32 "$KERNEL_DIR/$src" -o "$BUILD_DIR/$obj"
	done

	for src in $KERNEL_C_SOURCES; do
		obj="${src%.c}.o"
		echo "  [CC]  kernel/$src -> build/$obj"
		$CC $CFLAGS "$KERNEL_DIR/$src" -o "$BUILD_DIR/$obj"
	done
	echo "  Done."
	echo ""

	# ---- Stage 3: Link Kernel Binary ----
	echo "[Stage 3/4] Linking Kernel Binary"

	# Build object list (entry point first)
	OBJECTS="$BUILD_DIR/kernel_entry.o $BUILD_DIR/isr.o"
	for src in $KERNEL_C_SOURCES; do
		OBJECTS="$OBJECTS $BUILD_DIR/${src%.c}.o"
	done

	echo "  Linking -> build/kernel.pe"
	$LD $LDFLAGS $OBJECTS -o "$BUILD_DIR/kernel.pe"

	echo "  Extracting -> build/kernel.bin"
	$OBJCOPY -O binary "$BUILD_DIR/kernel.pe" "$BUILD_DIR/kernel.bin"
	echo "  Done."
	echo ""

	# ---- Stage 4: Create Disk Image ----
	echo "[Stage 4/4] Creating Disk Image"

	echo "  Combining boot.bin + kernel.bin -> build/os.iso"
	cat "$BUILD_DIR/boot.bin" "$BUILD_DIR/kernel.bin" > "$BUILD_DIR/os.iso"

	# Pad to at least 64 sectors (bootloader reads 64)
	# 1 boot sector + 64 kernel sectors is ~33KB, but VirtualBox 
	# requires a larger size for VDI conversion (usually at least 1MB).
	# We pad to exactly 1MB (1,048,576 bytes).
	local img_size
	img_size=$(stat -c%s "$BUILD_DIR/os.iso" 2>/dev/null || wc -c < "$BUILD_DIR/os.iso")
	local target_size=1048576
	if [ "$img_size" -lt "$target_size" ]; then
		local pad_size=$((target_size - img_size))
		dd if=/dev/zero bs=1 count=$pad_size >> "$BUILD_DIR/os.iso" 2>/dev/null
	fi

	echo "  Done."
	echo ""

	# ---- Stage 5: Convert to VirtualBox VDI ----
	echo "[Stage 5/5] Converting to VirtualBox VDI"
	rm -f "$BUILD_DIR/os.vdi"
	if command -v $QEMU_IMG >/dev/null 2>&1; then
		$QEMU_IMG convert -f raw -O vdi "$BUILD_DIR/os.iso" "$BUILD_DIR/os.vdi" >/dev/null
		echo "  Done: build/os.vdi"
	else
		echo "  Warning: qemu-img not found, skipping VDI creation."
	fi
	echo ""

	# ---- Summary ----
	local final_size
	final_size=$(stat -c%s "$BUILD_DIR/os.iso" 2>/dev/null || wc -c < "$BUILD_DIR/os.iso")
	echo "============================================"
	echo "  Build Complete!"
	echo "  Output: build/os.iso ($final_size bytes)"
	if [ -f "$BUILD_DIR/os.vdi" ]; then
		echo "  Output: build/os.vdi"
	fi
	echo "============================================"
	echo ""
}

run() {
	build
	echo "Launching QEMU..."
	echo ""
	# Booting as a raw CD-ROM image
	$QEMU -drive format=raw,file="$BUILD_DIR/os.iso"
}

debug_run() {
	build
	echo "Launching QEMU in debug mode..."
	echo "Connect GDB with: target remote :1234"
	echo ""
	$QEMU -drive format=raw,file="$BUILD_DIR/os.iso" -s -S
}

clean() {
	echo "Cleaning build artifacts..."
	rm -rf "$BUILD_DIR"
	echo "Done."
}

# ---- Entry Point ----
case "${1}" in
	run)    run ;;
	debug)  debug_run ;;
	clean)  clean ;;
	*)      build ;;
esac
