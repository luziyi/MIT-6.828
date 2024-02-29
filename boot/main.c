#include <inc/x86.h>
#include <inc/elf.h>

/**********************************************************************
 * This a dirt simple boot loader, whose sole job is to boot
 * an ELF kernel image from the first IDE hard disk.
 *
 * DISK LAYOUT
 *  * This program(boot.S and main.c) is the bootloader.  It should
 *    be stored in the first sector of the disk.
 *
 *  * The 2nd sector onward holds the kernel image.
 *
 *  * The kernel image must be in ELF format.
 *
 * BOOT UP STEPS
 *  * when the CPU boots it loads the BIOS into memory and executes it
 *
 *  * the BIOS intializes devices, sets of the interrupt routines, and
 *    reads the first sector of the boot device(e.g., hard-drive)
 *    into memory and jumps to it.
 *
 *  * Assuming this boot loader is stored in the first sector of the
 *    hard-drive, this code takes over...
 *
 *  * control starts in boot.S -- which sets up protected mode,
 *    and a stack so C code then run, then calls bootmain()
 *
 *  * bootmain() in this file takes over, reads in the kernel and jumps to it.
 **********************************************************************/

#define SECTSIZE	512
#define ELFHDR		((struct Elf *) 0x10000) // scratch space

void readsect(void*, uint32_t);
void readseg(uint32_t, uint32_t, uint32_t);

void
bootmain(void)
{
	struct Proghdr *ph, *eph;

	// read 1st page off disk 读取磁盘的第一页/扇区？
	readseg((uint32_t) ELFHDR, SECTSIZE*8, 0);

	// is this a valid ELF? 判断是否是有效的ELF
	if (ELFHDR->e_magic != ELF_MAGIC)
		goto bad;

	// load each program segment (ignores ph flags)加载各个程序段
	ph = (struct Proghdr *) ((uint8_t *) ELFHDR + ELFHDR->e_phoff);//将 ph 指针设置为 ELF 文件头（ELFHDR）后面紧接着程序段头表（program header table）的位置。它将 ELF 文件头地址（ELFHDR）转换为 uint8_t 类型的指针，然后加上 ELF 文件头中的程序段头表的偏移量（e_phoff），最后将结果转换为 struct Proghdr 类型的指针，以便访问程序段头。
	eph = ph + ELFHDR->e_phnum;//将 eph 指针设置为指向程序段头表的末尾。它通过将 ph 指针与程序段头表中的条目数（e_phnum）相加来实现
	for (; ph < eph; ph++)//一个简单的 for 循环，它遍历了程序段头表中的每个条目
		// p_pa is the load address of this segment (as well
		// as the physical address)
		readseg(ph->p_pa, ph->p_memsz, ph->p_offset);//调用 readseg 函数，将程序段从 ELF 文件中加载到内存中。它使用 ph->p_pa（段在物理内存中的地址）、ph->p_memsz（段在内存中的大小）和 ph->p_offset（段在 ELF 文件中的偏移量）作为参数。

	// call the entry point from the ELF header
	// note: does not return!
	((void (*)(void)) (ELFHDR->e_entry))(); //函数指针调用，它调用了程序的入口点（entry point），启动程序的执行。它通过 ELFHDR->e_entry 获取入口点的地址，然后将其转换为函数指针类型并调用该函数。

bad:
	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8E00);
	while (1)
		/* do nothing */;
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked
void
readseg(uint32_t pa, uint32_t count, uint32_t offset)
{
	uint32_t end_pa;

	end_pa = pa + count;

	// round down to sector boundary
	pa &= ~(SECTSIZE - 1);

	// translate from bytes to sectors, and kernel starts at sector 1
	offset = (offset / SECTSIZE) + 1;

	// If this is too slow, we could read lots of sectors at a time.
	// We'd write more to memory than asked, but it doesn't matter --
	// we load in increasing order.
	while (pa < end_pa) {
		// Since we haven't enabled paging yet and we're using
		// an identity segment mapping (see boot.S), we can
		// use physical addresses directly.  This won't be the
		// case once JOS enables the MMU.
		readsect((uint8_t*) pa, offset);
		pa += SECTSIZE;
		offset++;
	}
}

void
waitdisk(void)
{
	// wait for disk reaady
	while ((inb(0x1F7) & 0xC0) != 0x40)
		/* do nothing */;
}

void
readsect(void *dst, uint32_t offset)
{
	// wait for disk to be ready
	waitdisk();

	outb(0x1F2, 1);		// count = 1
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20);	// cmd 0x20 - read sectors

	// wait for disk to be ready
	waitdisk();

	// read a sector
	insl(0x1F0, dst, SECTSIZE/4);
}

