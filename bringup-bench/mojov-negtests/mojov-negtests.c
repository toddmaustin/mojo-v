#include "libmin.h"
#include "simon.h"

typedef unsigned __int128 uint128_t;

// Mojo-V asm instruction definitions (using the format-friendly .insn directive in GNU AS
#define LDE(rd,base,ofs) ".insn i 0xb, 0x0, " #rd ", " #base ", " #ofs "\n\t"
#define SDE(src,base,ofs) ".insn s 0xb, 0x1, " #src ", " #ofs "(" #base ")\n\t"
#define FLDE(rd,base,ofs) ".insn i 0xb, 0x2, " #rd ", " #base ", " #ofs "\n\t"
#define FSDE(src,base,ofs) ".insn s 0xb, 0x3, " #src ", " #ofs "(" #base ")\n\t"

// Define your custom CSR number
#define CSR_MPRIVREGCFG 0x0a0

static void
print_mprivregcfg(uint64_t val)
{
  libmin_printf("(mojov_en:%s, key_valid:%s, format_sel:%s, mojov_ver:%u)",
                (val & 0x01) ? "t" : "f",
                (val & 0x02) ? "t" : "f",
                (val & 0x04) ? "strong" : "weak",
                (val >> 3) & 0xff);
}

// Inline helpers
static inline uint64_t
read_mprivregcfg(void)
{
  uint64_t value;
  __asm__ volatile ("csrr %0, %1" : "=r"(value) : "i"(CSR_MPRIVREGCFG));
  return value;
}

static inline void
write_mprivregcfg(uint64_t value)
{
  __asm__ volatile ("csrw %0, %1" :: "i"(CSR_MPRIVREGCFG), "rK"(value));
}

// check that the SECRET INT registers have been cleared (after Mojo-V disabled)
__attribute__((naked)) uint64_t
secret_ints_cleared(void)
{
  __asm__ volatile(
    "or   a0, x28, x29\n\t"
    "or   a0, a0,  x30\n\t"
    "or   a0, a0,  x31\n\t"
    "ret\n\t"
  );
}

// check that the SECRET FP registers have been cleared (after Mojo-V disabled)
__attribute__((naked)) uint64_t
secret_fps_cleared(void)
{
  __asm__ volatile(
    "fmv.x.d  x28, f28\n\t"
    "fmv.x.d  x29, f29\n\t"
    "fmv.x.d  x30, f30\n\t"
    "fmv.x.d  x31, f31\n\t"

    "or   a0, x28, x29\n\t"
    "or   a0, a0,  x30\n\t"
    "or   a0, a0,  x31\n\t"
    "ret\n\t"
  );
}

// Predefined memory values
uint64_t x = 35;
uint64_t max = 25;

uint128_t x_enc;
uint128_t max_enc;
uint128_t bogus_enc = 42;

int
main(void)
{

  // initilize cipher engine, for checking results
  #define MOJOV_PT_SIG   0xdeadbeef
  union mojov_memfmt_t {
    uint128_t ct;     // ciphertext

    struct {          // plaintext
      uint64_t val;     // register plaintext value
      uint32_t salt;    // random salt
      uint32_t sig;     // fixed signature
    } pt;
  };
  uint128_t simon_key = GEN128(0x0f0e0d0c0b0a0908, 0x0706050403020100);
  simon_state_t simon_state;
  simon_128_128_keyexpand(&simon_state, simon_key, 68);

  //
  // mprivregcfg tests
  //
  libmin_printf("** Running CSR[privreg] tests...\n");

  uint64_t val;

  // read reset value
  val = read_mprivregcfg();
  libmin_printf("Initial mprivregcfg = 0x%lx, ", val);
  print_mprivregcfg(val);
  libmin_printf("\n");

  // read the mojov_arg
  uint16_t mojov_arg = (val >> 11) & 0xffff;

  // enable private register semantics (bit 0 = 1)
  write_mprivregcfg(1);

  val = read_mprivregcfg();
  libmin_printf("After enable, mprivregcfg = 0x%lx, ", val);
  print_mprivregcfg(val);
  libmin_printf("\n");

  libmin_printf("INFO: Running Mojo-V test %u...\n", (uint32_t)mojov_arg);
  switch (mojov_arg)
  {
  // positive tests
  case 0:
    // do some secret computation
    libmin_printf("Initial inputs: x:%lu, max:%lu\n", x, max);

    // inline assembly block
    __asm__ volatile (
      // first encrypt the public X and MAX values
      "ld t3, (%0)\n\t"
      SDE(t3,%2,0)
      "ld t3, (%1)\n\t"
      SDE(t3,%3,0)

      // load third-party encrypted operands
      LDE(t3, %2, 0)
      LDE(t4, %3, 0)

      // Condition: (max < x)?
      "slt       /*p2*/t5, /*p1*/t4, /*p0*/t3\n\t" /* p2 = (p1 < p0) ? 1 : 0 */

      // Build data-oblivious conditional result
      "czero.eqz /*p0*/t3, /*p0*/t3, /*p2*/t5\n\t" // if p2==0 => p0=0, else p0=x
      "czero.nez /*p1*/t4, /*p1*/t4, /*p2*/t5\n\t" // if p2!=0 => p1=0, else p1=max
      "or        /*p3*/t6, /*p0*/t3, /*p1*/t4\n\t" // select: p3 = (x if x>max else max)

      // Store third-party encrypted (potentially) new max value
      SDE(t6,%3,0)
      :
      : "r" (&x), "r" (&max), "r" (&x_enc), "r" (&max_enc), "r" (&bogus_enc) // input operands
      : "t3", "t4", "t5", "t6" // clobbered registers
    );

    // decrypt results locally
    union mojov_memfmt_t x_check, max_check;
    simon_128_128_decrypt(&simon_state, x_enc, &x_check.ct);
    simon_128_128_decrypt(&simon_state, max_enc, &max_check.ct);
    libmin_printf("Final results:   x:%lu, max:%lu\n", x_check.pt.val, max_check.pt.val);

    __asm__ volatile (
      // non-secret → secret GPR (ALU immediates & shifts)
      "lui   x28, 0x12345\n\t"
      "auipc x28, 0\n\t"
      "addi  x28, x5, 1\n\t"
      "slti  x28, x5, 0\n\t"
      "sltiu x28, x5, 1\n\t"
      "xori  x28, x5, 1\n\t"
      "ori   x28, x5, 1\n\t"
      "andi  x28, x5, 1\n\t"
      "slli  x28, x5, 7\n\t"
      "srli  x28, x5, 7\n\t"
      "srai  x28, x5, 7\n\t"
      "addiw x28, x5, 1\n\t"
      "slliw x28, x5, 7\n\t"
      "srliw x28, x5, 7\n\t"
      "sraiw x28, x5, 7\n\t"

      // secret → secret GPR (ALU regs & W-ops)
      "add   x28, x29, x30\n\t"
      "sub   x28, x29, x30\n\t"
      "sll   x28, x29, x30\n\t"
      "srl   x28, x29, x30\n\t"
      "sra   x28, x29, x30\n\t"
      "slt   x28, x29, x30\n\t"
      "sltu  x28, x29, x30\n\t"
      "xor   x28, x29, x30\n\t"
      "or    x28, x29, x30\n\t"
      "and   x28, x29, x30\n\t"

      "addw  x28, x29, x30\n\t"
      "subw  x28, x29, x30\n\t"
      "sllw  x28, x29, x30\n\t"
      "srlw  x28, x29, x30\n\t"
      "sraw  x28, x29, x30\n\t"

      // load non-secret → secret GPR (plain memory)
      "lb   x28, (%0)\n\t"
      "lbu  x28, (%0)\n\t"
      "lh   x28, (%0)\n\t"
      "lhu  x28, (%0)\n\t"
      "lw   x28, (%0)\n\t"
      "lwu  x28, (%0)\n\t"
      "ld   x28, (%0)\n\t"

      // (RV64M) int mul/div/rem — non-secret→secret and secret→secret
      // non-secret → secret
      "mul   x28, x5,  x6\n\t"
      "mulh  x28, x5,  x6\n\t"
      "mulhsu x28, x5, x6\n\t"
      "mulhu x28, x5,  x6\n\t"
      "div   x28, x5,  x6\n\t"
      "divu  x28, x5,  x6\n\t"
      "rem   x28, x5,  x6\n\t"
      "remu  x28, x5,  x6\n\t"
      "mulw  x28, x5,  x6\n\t"
      "divw  x28, x5,  x6\n\t"
      "divuw x28, x5,  x6\n\t"
      "remw  x28, x5,  x6\n\t"
      "remuw x28, x5,  x6\n\t"

      // secret → secret
      "mul   x28, x29, x30\n\t"
      "mulh  x28, x29, x30\n\t"
      "mulhsu x28, x29, x30\n\t"
      "mulhu x28, x29, x30\n\t"
      "div   x28, x29, x30\n\t"
      "divu  x28, x29, x30\n\t"
      "rem   x28, x29, x30\n\t"
      "remu  x28, x29, x30\n\t"
      "mulw  x28, x29, x30\n\t"
      "divw  x28, x29, x30\n\t"
      "divuw x28, x29, x30\n\t"
      "remw  x28, x29, x30\n\t"
      "remuw x28, x29, x30\n\t"

      // non-secret FP → secret FP (S & D, incl. FMA)
      // Single-precision
      "fadd.s   f28, f5,  f6\n\t"
      "fsub.s   f28, f5,  f6\n\t"
      "fmul.s   f28, f5,  f6\n\t"
      "fdiv.s   f28, f5,  f6\n\t"
      "fsqrt.s  f28, f5\n\t"
      "fmin.s   f28, f5,  f6\n\t"
      "fmax.s   f28, f5,  f6\n\t"
      "fsgnj.s  f28, f5,  f6\n\t"
      "fsgnjn.s f28, f5,  f6\n\t"
      "fsgnjx.s f28, f5,  f6\n\t"
      "fmadd.s  f28, f5,  f6, f7\n\t"
      "fmsub.s  f28, f5,  f6, f7\n\t"
      "fnmsub.s f28, f5,  f6, f7\n\t"
      "fnmadd.s f28, f5,  f6, f7\n\t"

      // Double-precision
      "fadd.d   f28, f5,  f6\n\t"
      "fsub.d   f28, f5,  f6\n\t"
      "fmul.d   f28, f5,  f6\n\t"
      "fdiv.d   f28, f5,  f6\n\t"
      "fsqrt.d  f28, f5\n\t"
      "fmin.d   f28, f5,  f6\n\t"
      "fmax.d   f28, f5,  f6\n\t"
      "fsgnj.d  f28, f5,  f6\n\t"
      "fsgnjn.d f28, f5,  f6\n\t"
      "fsgnjx.d f28, f5,  f6\n\t"
      "fmadd.d  f28, f5,  f6, f7\n\t"
      "fmsub.d  f28, f5,  f6, f7\n\t"
      "fnmsub.d f28, f5,  f6, f7\n\t"
      "fnmadd.d f28, f5,  f6, f7\n\t"

      // secret FP → secret FP (S & D, incl. FMA)
      // Single-precision
      "fadd.s   f28, f29, f30\n\t"
      "fsub.s   f28, f29, f30\n\t"
      "fmul.s   f28, f29, f30\n\t"
      "fdiv.s   f28, f29, f30\n\t"
      "fsqrt.s  f28, f29\n\t"
      "fmin.s   f28, f29, f30\n\t"
      "fmax.s   f28, f29, f30\n\t"
      "fsgnj.s  f28, f29, f30\n\t"
      "fsgnjn.s f28, f29, f30\n\t"
      "fsgnjx.s f28, f29, f30\n\t"
      "fmadd.s  f28, f29, f30, f31\n\t"
      "fmsub.s  f28, f29, f30, f31\n\t"
      "fnmsub.s f28, f29, f30, f31\n\t"
      "fnmadd.s f28, f29, f30, f31\n\t"

      // Double-precision
      "fadd.d   f28, f29, f30\n\t"
      "fsub.d   f28, f29, f30\n\t"
      "fmul.d   f28, f29, f30\n\t"
      "fdiv.d   f28, f29, f30\n\t"
      "fsqrt.d  f28, f29\n\t"
      "fmin.d   f28, f29, f30\n\t"
      "fmax.d   f28, f29, f30\n\t"
      "fsgnj.d  f28, f29, f30\n\t"
      "fsgnjn.d f28, f29, f30\n\t"
      "fsgnjx.d f28, f29, f30\n\t"
      "fmadd.d  f28, f29, f30, f31\n\t"
      "fmsub.d  f28, f29, f30, f31\n\t"
      "fnmsub.d f28, f29, f30, f31\n\t"
      "fnmadd.d f28, f29, f30, f31\n\t"

      // load non-secret → secret FP (plain memory)
      "flw f28, (%0)\n\t"
      "fld f28, (%0)\n\t"

      // fmv between secret regs, and non-secret → secret (moves)
      // FPR <-> FPR moves via sign-inject (canonical moves)
      "fsgnj.s  f28, f27, f27\n\t"    // non-secret → secret
      "fsgnj.s  f28, f29, f29\n\t"    // secret → secret
      "fsgnj.d  f28, f27, f27\n\t"
      "fsgnj.d  f28, f29, f29\n\t"

      // INT <-> FP bit moves (RV64)
      "fmv.d.x  f28, x5\n\t"          // non-secret → secret FP
      "fmv.d.x  f28, x29\n\t"         // secret → secret FP
      "fmv.x.d  x28, f7\n\t"          // non-secret FP → secret GPR
      "fmv.x.d  x28, f29\n\t"         // secret FP → secret GPR

      // (RV32+F alias shown for completeness when assembling generically)
      "fmv.s.x  f28, x5\n\t"
      "fmv.x.w  x28, f7\n\t"

      // fcvt between secret regs, and non-secret → secret (conversions)
      // INT -> FP (dest secret FP)
      "fcvt.s.w  f28, x5\n\t"
      "fcvt.s.wu f28, x5\n\t"
      "fcvt.s.l  f28, x5\n\t"
      "fcvt.s.lu f28, x5\n\t"
      "fcvt.d.w  f28, x5\n\t"
      "fcvt.d.wu f28, x5\n\t"
      "fcvt.d.l  f28, x5\n\t"
      "fcvt.d.lu f28, x5\n\t"

      // FP -> INT (dest secret GPR)
      "fcvt.w.s  x28, f5\n\t"
      "fcvt.wu.s x28, f5\n\t"
      "fcvt.l.s  x28, f5\n\t"
      "fcvt.lu.s x28, f5\n\t"
      "fcvt.w.d  x28, f5\n\t"
      "fcvt.wu.d x28, f5\n\t"
      "fcvt.l.d  x28, f5\n\t"
      "fcvt.lu.d x28, f5\n\t"

      // Same but secret → secret
      "fcvt.s.l  f28, x29\n\t"
      "fcvt.d.l  f28, x29\n\t"
      "fcvt.l.d  x28, f29\n\t"
      "fcvt.w.s  x28, f29\n\t"

      // FP compares / class writing to secret GPR
      "feq.s    x28, f5,  f6\n\t"
      "flt.s    x28, f5,  f6\n\t"
      "fle.s    x28, f5,  f6\n\t"
      "fclass.s x28, f5\n\t"

      "feq.d    x28, f5,  f6\n\t"
      "flt.d    x28, f5,  f6\n\t"
      "fle.d    x28, f5,  f6\n\t"
      "fclass.d x28, f5\n\t"

      // secret → secret versions
      "feq.d    x28, f29, f30\n\t"
      "flt.d    x28, f29, f30\n\t"
      "fle.d    x28, f29, f30\n\t"
      "fclass.d x28, f29\n\t"

#ifdef notdef
      // CSR reads to secret GPR (Zicsr)
      "csrrs  x28, mstatus, x0\n\t"
      "csrrw  x28, mstatus, x0\n\t"
      "csrrc  x28, mstatus, x0\n\t"
      "csrrsi x28, mstatus, 1\n\t"
      "csrrwi x28, mstatus, 1\n\t"
      "csrrci x28, mstatus, 1\n\t"
#endif /* notdef */

      // compressed (C) forms that write/load into secret regs
      "c.addi  x28, 1\n\t"
      "c.addiw x28, 1\n\t"
      "c.slli  x28, 7\n\t"
      "c.add   x28, x5\n\t"
      "c.mv    x28, x5\n\t"
      "c.lwsp  x28, 0(sp)\n\t"
      "c.ldsp  x28, 0(sp)\n\t"
      "c.fldsp f28, 0(sp)\n\t"

      // encrypted → secret regs
      LDE(x28, %2, 0)
      FLDE(f28, %2, 0)

      :
      : "r" (&x), "r" (&max), "r" (&x_enc), "r" (&max_enc), "r" (&bogus_enc) // input operands
      : "t3", "t4", "t5", "t6", "f28", "f29", "f30", "f31" // clobbered registers
    );

    libmin_printf("INFO: All positive tests passed.\n");
    break;

  // negative tests
  case 1:
    __asm__ volatile ("addi x5, x28, 1");
    libmin_printf("ERROR: negative test did NOT fail!\n"); libmin_fail(2);
    break;

  case 2:
    __asm__ volatile ("slti x5, x28, 0");
    libmin_printf("ERROR: negative test did NOT fail!\n"); libmin_fail(2);
    break;

  case 3:
    __asm__ volatile ("sltiu x5, x28, 0");
    libmin_printf("ERROR: negative test did NOT fail!\n"); libmin_fail(2);
    break;

  case 4:
    __asm__ volatile ("xori x5, x28, 1");
    libmin_printf("ERROR: negative test did NOT fail!\n"); libmin_fail(2);
    break;

  case 5:
    __asm__ volatile ("ori  x5, x28, 1");
    libmin_printf("ERROR: negative test did NOT fail!\n"); libmin_fail(2);
    break;


  case 99:
    __asm__ volatile (
      // test-load a bogus ciphertext value -- it should get an exception
      LDE(t3, %0, 0)
      :
      : "r" (&simon_key) // input operands
      : "t3", "t4", "t5", "t6" // clobbered registers
    );
    break;

  case 93:
    __asm__ volatile (
      // cannot ld/sd a secret register
      "sd t3, (%0)\n\t"
      :
      : "r" (&simon_key) // input operands
      : "t3", "t4", "t5", "t6" // clobbered registers
    );
    break;

  case 94:
    __asm__ volatile (
      // cannot ld/sd a secret register
      "fsd f28, (%0)\n\t"
      :
      : "r" (&simon_key) // input operands
      : "t3", "t4", "t5", "t6" // clobbered registers
    );
    break;

  case 95:
    __asm__ volatile (
      // Mojo-V test: should have secret dest
      "slt       t0, /*p1*/t4, /*p0*/t3\n\t"
      :
      : "r" (&simon_key) // input operands
      : "t3", "t4", "t5", "t6" // clobbered registers
    );
    break;

  case 96:
    __asm__ volatile (
      // Mojo-V test: should have secret dest
      "flt.d     t0, f28, f27\n\t"
      :
      : "r" (&simon_key) // input operands
      : "t3", "t4", "t5", "t6" // clobbered registers
    );
    break;

  default:
    libmin_printf("ERROR: invalid test (%u).\n", (uint32_t)mojov_arg);
    break;
  }


    // "jalr         ra, 64(t4)\n\t"
    // "sw        t5, (t3)\n\t"
    // "bne       t3, t0, .+12\n\t"
    // "bne       t0, t3, .+12\n\t"

    // try to move the secret predicate, via integer to FP register/ moves/converts
    // "fmv.w.x      f1, t2\n\t"
    // "fcvt.s.w     f3, t2\n\t"
    // "fmv.w.x      f1, t5\n\t"
    // "fmv.d.x      f2, t5\n\t"
    // "fcvt.s.w     f3, t5\n\t"
    // "fcvt.s.wu    f3, t5\n\t"
    // "fcvt.s.l     f5, t5\n\t"
    // "fcvt.s.lu    f6, t5\n\t"
    // "fcvt.d.w     f1, t5\n\t"
    // "fcvt.d.wu    f2, t5\n\t"
    // "fcvt.d.l     f3, t5\n\t"
    // "fcvt.d.lu    f4, t5\n\t"


  // disable private register semantics (write 0)
  write_mprivregcfg(0);

  // check that the secret registers were cleared
  if (secret_ints_cleared() == 0 && secret_fps_cleared() == 0)
    libmin_printf("INFO: Confirmed secret INT and FP regs cleared after disabling Mojo-V mode.\n");
  else
  {
    libmin_printf("INFO: Confirmed secret INT and/or FP regs ARE NOT cleared after disabling Mojo-V mode.\n");
    libmin_fail(1);
  }

  val = read_mprivregcfg();
  libmin_printf("After disable, mprivregcfg = 0x%lx, ", val);
  print_mprivregcfg(val);
  libmin_printf("\n");

  libmin_success();
  return 0;
}

