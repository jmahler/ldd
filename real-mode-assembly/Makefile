
#
# $ make
# $ qemu -moitor stdio kernel.img
#

all: kernel.bin kernel.img

kernel.bin: kernel.asm
	nasm $< -f bin -o $@

kernel.img: kernel.bin
	dd if=kernel.bin of=kernel.img

clean:
	-rm -f kernel.bin
