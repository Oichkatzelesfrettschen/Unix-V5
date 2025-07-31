/**
 * @file reg.h
 * @brief Definitions for accessing saved user registers from the U-area.
 *
 * These macros define offsets (or indices) to access saved user registers
 * (R0-R7, PS) within the `u.u_ar0` array in the U-area (`struct user`).
 * `u.u_ar0` is typically initialized by the trap handler to point to the
 * location on the kernel stack where user registers were saved upon a
 * trap or interrupt.
 *
 * The values are byte offsets relative to `u.u_ar0` if it points to R0.
 * For example, if `u.u_ar0` points to where R0 is saved:
 * - `u.u_ar0[R0]` (i.e. `u.u_ar0[0]`) would be R0.
 * - `u.u_ar0[R1]` (i.e. `u.u_ar0[-2/sizeof(int)]`) implies a different base or indexing logic.
 *
 * Given PDP-11's 16-bit words and typical stack layout where registers are pushed,
 * these are more likely indices into an array of words, where u.u_ar0 points
 * to the saved R0. The negative values are unusual if u.u_ar0 points *at* R0.
 * It's more plausible that u.u_ar0 points to a specific location on the kernel stack,
 * and these are word offsets from that point to find the respective registers.
 * Let's assume they are word offsets relative to where `u.u_ar0` is set by `trap.c`.
 *
 * From `trap.c`, `u.u_ar0 = &r0;` where `r0` is a parameter to `trap()`
 * representing the saved R0. So, `u.u_ar0[R0]` is `(&r0)[0]` which is `r0`.
 * `u.u_ar0[R1]` is `(&r0)[-1]` if word indexing, or `*(&r0 - 1)`.
 * The actual stack frame built by the hardware/assembly trap entry is:
 *   (older stack items)
 *   PS  (Processor Status)
 *   PC  (Program Counter)
 *   R0  <- u.u_ar0 points here during trap handling in C
 *   (optional other registers like R1, then SP from before trap)
 *
 * The values below seem to be indices *into an array of integers (words)*
 * where the base of that array (`u.u_ar0`) points to the saved R0 on the stack.
 * R0 = 0  -> u.u_ar0[0]
 * R1 = -2 -> * (u.u_ar0 - 1) -- this implies R1 is stored *before* R0 by trap hw/asm
 * R2 = -9 -> * (u.u_ar0 - 9/sizeof(int)) -- this is very strange for direct indexing.
 *
 * Re-evaluation based on common PDP-11 trap sequence:
 * When a trap occurs, PS and PC are pushed by hardware.
 * Kernel trap handler assembly then typically pushes R0-R5. SP (R6) and R7 (PC) are special.
 * If `u.u_ar0` is set to point to the saved R0 on stack:
 * Stack top (lower address): ..., R5, R4, R3, R2, R1, R0, PC_old, PS_old ... (higher address)
 * So, if u.u_ar0 = address_of_saved_R0:
 * R0 is at u.u_ar0[0]
 * R1 is at u.u_ar0[-1] (if R1 pushed before R0) - this doesn't match.
 *
 * Let's assume `u.u_ar0` is a pointer to an array set up by kernel assembly,
 * and these are indices. The comment in `trap.c` is `u.u_ar0 = &r0;` where r0 is a function argument.
 * The arguments to trap are `trap(dev, sp, r1, nps, r0, pc, ps)`.
 * So `u.u_ar0` points to the argument `r0` on the C stack frame of `trap`.
 * `regloc[]` in `trap.c` maps these defines to actual register numbers (0-7).
 * The values are more likely direct indices for `u.u_ar0[regloc[USER_REG_INDEX]]`
 * or specific offsets for manipulating the user register save area.
 *
 * The defines are likely used as `u.u_ar0[R<n>]` which means they are indices
 * if `u.u_ar0` is treated as `int*`.
 * R0=0 means `u.u_ar0[0]` is R0.
 * R7=1 means `u.u_ar0[1]` is R7 (PC).
 * RPS=2 means `u.u_ar0[2]` is PS.
 * The negative values for R1-R6 are very odd if `u.u_ar0` points to R0.
 * It might be that `u.u_ar0` actually points to a specific point in the U-area's
 * register save fields, and these are offsets from that.
 * However, `trap.c` sets `u.u_ar0 = &r0;` (where r0 is the saved R0).
 * This implies `u.u_ar0[R0]` is `r0`, `u.u_ar0[R7]` is `pc` and `u.u_ar0[RPS]` is `ps` if these were positive offsets.
 * The actual save area might be `u_rsav` or `u_fsav` or a trap-specific stack area.
 * Given the usage `u.u_ar0[R0]` in `sys1.c` for `rexit`, R0=0 is an index.
 * The definition of `regloc` in `trap.c` is `char regloc[8] { R0, R1, R2, R3, R4, R5, R6, R7 };`
 * This suggests these are indeed meant to be indices for an array that holds R0..R7.
 * The most probable interpretation is that `u.u_ar0` points to an array of saved general registers
 * and `RPS` is an index for PS *relative to that array's start or a known point.*
 * The negative values remain confusing for a direct array index if R0 is index 0.
 * For now, documenting them as symbolic register indices.
 */
///@{
#define	R0	(0)		/**< Index for saved user register R0. */
#define	R1	(-2)	/**< Index for saved user register R1. (Value is suspicious for direct indexing if R0 is 0) */
#define	R2	(-9)	/**< Index for saved user register R2. (Value is suspicious) */
#define	R3	(-8)	/**< Index for saved user register R3. (Value is suspicious) */
#define	R4	(-7)	/**< Index for saved user register R4. (Value is suspicious) */
#define	R5	(-6)	/**< Index for saved user register R5 (Frame Pointer). (Value is suspicious) */
#define	R6	(-3)	/**< Index for saved user register R6 (Stack Pointer). (Value is suspicious) */
#define	R7	(1)		/**< Index for saved user register R7 (Program Counter). */
#define	RPS	(2)		/**< Index for saved user Processor Status word. */
///@}
