#ifndef MOJOV_REGS_H
#define MOJOV_REGS_H

// included at the start of every C/C++

// Reserve integer registers x24-x31
register long reserved_x24 __asm__("x24");
register long reserved_x25 __asm__("x25");
register long reserved_x26 __asm__("x26");
register long reserved_x27 __asm__("x27");
register long reserved_x28 __asm__("x28");
register long reserved_x29 __asm__("x29");
register long reserved_x30 __asm__("x30");
register long reserved_x31 __asm__("x31");

// Reserve floating-point registers f24-f31
register double reserved_f24 __asm__("f24");
register double reserved_f25 __asm__("f25");
register double reserved_f26 __asm__("f26");
register double reserved_f27 __asm__("f27");
register double reserved_f28 __asm__("f28");
register double reserved_f29 __asm__("f29");
register double reserved_f30 __asm__("f30");
register double reserved_f31 __asm__("f31");

#endif /* MOJOV_REGS_H */
