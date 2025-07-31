/**
 * @file seg.h
 * @brief Definitions related to the PDP-11 Memory Management Unit (MMU) and segmentation.
 *
 * This file defines constants for the addresses of the segmentation registers
 * (UISA - User Instruction Space Address, UISD - User Instruction Space Descriptor)
 * and bits within the descriptor registers (e.g., Read-Only, Read-Write, Extend Down).
 * It also declares `ka6`, a pointer typically used to access the U-area of the
 * current process via Kernel Address Register 6.
 *
 * Copyright 1973 Bell Telephone Laboratories Inc
 */

/** @name PDP-11 KT-11 Memory Management Unit Registers and Bits */
///@{
#define	UISD	0177600		/**< Base address of User Instruction Space Descriptor registers (for current process, via KISA6 mapping). */
#define	UISA	0177640		/**< Base address of User Instruction Space Address registers (for current process, via KISA6 mapping). */
// Note: These are often accessed via KISA6 which maps the current U-area and its u_uisa/u_uisd arrays
// to these fixed kernel virtual addresses when the kernel is active for that process.
// Other sets like KISA0-KISA7 / KISD0-KISD7 are for kernel mode segments.
// SISA0-SISA7 / SISD0-SISD7 are for supervisor mode segments (not typically used in V5/V6 Unix).

#define	RO	02		/**< Read-Only bit in segment descriptor (PAR - Page Address Register). Allows read access. */
#define	RW	06		/**< Read-Write bits in segment descriptor (PAR). Allows read and write access. (06 = Read + Write enabled) */
#define	WO	04		/**< Write-Only bit in segment descriptor (PAR). This is unusual; typically implies write, and read might be implicitly allowed or denied. Often part of RW. */
#define	ED	010		/**< Extend Down bit in segment descriptor (PAR). Used for stack segments that grow downwards. */
///@}

/**
 * @struct anon_seg_regs
 * @brief Anonymous structure defining an array of 8 integers.
 *
 * This is a common C idiom in early Unix source to declare a memory layout
 * without needing a named struct type, often used for direct memory mapping
 * of hardware registers like UISA or UISD. The compiler would allocate space for this
 * if it were a variable declaration, or it could be used for casting a pointer.
 * Here, it's likely just a style of noting that UISA and UISD are arrays of 8 registers.
 * It is not instantiated as a variable in this header.
 */
struct pdp11_seg_reg {
	int	r[8];		/**< Array of 8 integer registers (e.g., for UISA or UISD). */
};

#define UISD ((struct pdp11_seg_reg *)0177600)
#define UISA ((struct pdp11_seg_reg *)0177640)


extern int	*ka6;		/**< Pointer to Kernel Address Register 6 (KISA6).
						 * KISA6 is typically set up by the low-level context switch code (`swtch`)
						 * to point to the base of the current process's U-area (struct user).
						 * The `u.u_uisa` and `u.u_uisd` arrays within the U-area are then used
						 * to load the actual UISA and UISD hardware registers for user mode.
						 * This pointer itself (`ka6`) might be the fixed address of KISA6 (0172354) if it is
						 * directly addressable, or a kernel variable holding that address.
						 */
