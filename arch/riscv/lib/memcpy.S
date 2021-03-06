#include <linux/linkage.h>

/* void *memcpy(void *, const void *, size_t) */

#ifdef CONFIG_64BIT

ENTRY(memcpy)
	move v0, a0  /* Initialize return value */

	/* Defer to byte-oriented copy for small sizes */
	sltiu a3, a2, 128
	bnez a3, 4f
	/* Use word-oriented copy only if low-order bits match */
	andi a3, a0, 0x7
	andi a4, a1, 0x7
	bne a3, a4, 4f

	beqz a3, 2f  /* Skip if already aligned */
	/* Round to nearest double word-aligned address
	   greater than or equal to start address */
	andi a3, a1, ~(0x7)
	addi a3, a3, 0x8
	/* Handle initial misalignment */
	sub a4, a3, a1
1:
	lb a5, 0(a1)
	addi a1, a1, 1
	sb a5, 0(a0)
	addi a0, a0, 1
	bltu a1, a3, 1b
	sub a2, a2, a4  /* Update count */

2:
	andi a4, a2, ~(0x7f)
	beqz a4, 4f
	add a3, a1, a4
3:
	ld  a4, 0x00(a1)
	ld  a5, 0x08(a1)
	ld  a6, 0x10(a1)
	ld  a7, 0x18(a1)
	ld  a8, 0x20(a1)
	ld  a9, 0x28(a1)
	ld a10, 0x30(a1)
	ld a11, 0x38(a1)
	ld a12, 0x40(a1)
	ld a13, 0x48(a1)
	sd  a4, 0x00(a0)
	sd  a5, 0x08(a0)
	sd  a6, 0x10(a0)
	sd  a7, 0x18(a0)
	sd  a8, 0x20(a0)
	sd  a9, 0x28(a0)
	sd a10, 0x30(a0)
	sd a11, 0x38(a0)
	sd a12, 0x40(a0)
	sd a13, 0x48(a0)
	ld  a4, 0x50(a1)
	ld  a5, 0x58(a1)
	ld  a6, 0x60(a1)
	ld  a7, 0x68(a1)
	ld  a8, 0x70(a1)
	ld  a9, 0x78(a1)
	addi a1, a1, 0x80
	sd  a4, 0x50(a0)
	sd  a5, 0x58(a0)
	sd  a6, 0x60(a0)
	sd  a7, 0x68(a0)
	sd  a8, 0x70(a0)
	sd  a9, 0x78(a0)
	addi a0, a0, 0x80
	bltu a1, a3, 3b
	andi a2, a2, 0x7f  /* Update count */

4:
	/* Handle trailing misalignment */
	beqz a2, 6f
	add a3, a1, a2
5:
	lb a4, 0(a1)
	addi a1, a1, 1
	sb a4, 0(a0)
	addi a0, a0, 1
	bltu a1, a3, 5b
6:
	ret
END(memcpy)

#else
#error RV32 memcpy unimplemented
#endif /* CONFIG_64BIT */

