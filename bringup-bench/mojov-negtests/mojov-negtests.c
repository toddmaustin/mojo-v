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

void negfailed(void)
{
  libmin_printf("ERROR: negative test failed because NO exception occurred!\n");
  libmin_fail(2);
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

      // CSR reads to secret GPR (Zicsr)
      "csrrsi x28, fcsr, 1\n\t"
      "csrrwi x28, fcsr, 1\n\t"
      "csrrci x28, fcsr, 1\n\t"

      "csrrs x28, fcsr, x0\n\t"
      "csrrw x28, fcsr, x0\n\t"
      "csrrc x28, fcsr, x0\n\t"

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

  //
  // GPR ALU immediates (secret -> non-secret)
  //
  case 1:
    __asm__ volatile ("addi x5, x28, 1");
    negfailed();
    break;

  case 2:
    __asm__ volatile ("slti x5, x28, 0");
    negfailed();
    break;

  case 3:
    __asm__ volatile ("sltiu x5, x28, 0");
    negfailed();
    break;

  case 4:
    __asm__ volatile ("xori x5, x28, 1");
    negfailed();
    break;

  case 5:
    __asm__ volatile ("ori  x5, x28, 1");
    negfailed();
    break;

  case 6:
    __asm__ volatile ("andi x5, x28, 1");
    negfailed();
    break;

  case 7:
    __asm__ volatile ("slli x5, x28, 7");
    negfailed();
    break;

  case 8:
    __asm__ volatile ("srli x5, x28, 7");
    negfailed();
    break;

  case 9:
    __asm__ volatile ("srai x5, x28, 7");
    negfailed();
    break;

  case 10:
    __asm__ volatile ("addiw x5, x28, 1");
    negfailed();
    break;

  case 11:
    __asm__ volatile ("slliw x5, x28, 7");
    negfailed();
    break;

  case 12:
    __asm__ volatile ("srliw x5, x28, 7");
    negfailed();
    break;

  case 13:
    __asm__ volatile ("sraiw x5, x28, 7");
    negfailed();
    break;

  //
  // GPR ALU regs (any secret source -> non-secret dest)
  //
  case 14:
    __asm__ volatile ("add  x5, x28, x6");
    negfailed();
    break;

  case 15:
    __asm__ volatile ("add  x5, x6,  x28");
    negfailed();
    break;

  case 16:
    __asm__ volatile ("sub  x5, x28, x6");
    negfailed();
    break;

  case 17:
    __asm__ volatile ("sub  x5, x6,  x28");
    negfailed();
    break;

  case 18:
    __asm__ volatile ("sll  x5, x28, x6");
    negfailed();
    break;

  case 19:
    __asm__ volatile ("sll  x5, x6,  x28");
    negfailed();
    break;

  case 20:
    __asm__ volatile ("srl  x5, x28, x6");
    negfailed();
    break;

  case 21:
    __asm__ volatile ("srl  x5, x6,  x28");
    negfailed();
    break;

  case 22:
    __asm__ volatile ("sra  x5, x28, x6");
    negfailed();
    break;

  case 23:
    __asm__ volatile ("sra  x5, x6,  x28");
    negfailed();
    break;

  case 24:
    __asm__ volatile ("slt  x5, x28, x6");
    negfailed();
    break;

  case 25:
    __asm__ volatile ("slt  x5, x6,  x28");
    negfailed();
    break;

  case 26:
    __asm__ volatile ("sltu x5, x28, x6");
    negfailed();
    break;

  case 27:
    __asm__ volatile ("sltu x5, x6,  x28");
    negfailed();
    break;

  case 28:
    __asm__ volatile ("xor  x5, x28, x6");
    negfailed();
    break;

  case 29:
    __asm__ volatile ("xor  x5, x6,  x28");
    negfailed();
    break;

  case 30:
    __asm__ volatile ("or   x5, x28, x6");
    negfailed();
    break;

  case 31:
    __asm__ volatile ("or   x5, x6,  x28");
    negfailed();
    break;

  case 32:
    __asm__ volatile ("and  x5, x28, x6");
    negfailed();
    break;

  case 33:
    __asm__ volatile ("and  x5, x6,  x28");
    negfailed();
    break;

  case 34:
    __asm__ volatile ("addw x5, x28, x6");
    negfailed();
    break;

  case 35:
    __asm__ volatile ("addw x5, x6,  x28");
    negfailed();
    break;

  case 36:
    __asm__ volatile ("subw x5, x28, x6");
    negfailed();
    break;

  case 37:
    __asm__ volatile ("subw x5, x6,  x28");
    negfailed();
    break;

  case 38:
    __asm__ volatile ("sllw x5, x28, x6");
    negfailed();
    break;

  case 39:
    __asm__ volatile ("sllw x5, x6,  x28");
    negfailed();
    break;

  case 40:
    __asm__ volatile ("srlw x5, x28, x6");
    negfailed();
    break;

  case 41:
    __asm__ volatile ("srlw x5, x6,  x28");
    negfailed();
    break;

  case 42:
    __asm__ volatile ("sraw x5, x28, x6");
    negfailed();
    break;

  case 43:
    __asm__ volatile ("sraw x5, x6,  x28");
    negfailed();
    break;

  //
  // stores of secret GPR to plain memory
  //
  case 44:
    __asm__ volatile ("sb x28, 0(x1)");
    negfailed();
    break;

  case 45:
    __asm__ volatile ("sh x28, 0(x1)");
    negfailed();
    break;

  case 46:
    __asm__ volatile ("sw x28, 0(x1)");
    negfailed();
    break;

  case 47:
    __asm__ volatile ("sd x28, 0(x1)");
    negfailed();
    break;

  //
  // Use secret GPR as base address (loads/stores)
  //
  case 48:
    __asm__ volatile ("lb  x5, 0(x28)");
    negfailed();
    break;

  case 49:
    __asm__ volatile ("lbu x5, 0(x28)");
    negfailed();
    break;

  case 50:
    __asm__ volatile ("lh  x5, 0(x28)");
    negfailed();
    break;

  case 51:
    __asm__ volatile ("lhu x5, 0(x28)");
    negfailed();
    break;

  case 52:
    __asm__ volatile ("lw  x5, 0(x28)");
    negfailed();
    break;

  case 53:
    __asm__ volatile ("lwu x5, 0(x28)");
    negfailed();
    break;

  case 54:
    __asm__ volatile ("ld  x5, 0(x28)");
    negfailed();
    break;

  case 55:
    __asm__ volatile ("sb  x5, 0(x28)");
    negfailed();
    break;

  case 56:
    __asm__ volatile ("sh  x5, 0(x28)");
    negfailed();
    break;

  case 57:
    __asm__ volatile ("sw  x5, 0(x28)");
    negfailed();
    break;

  case 58:
    __asm__ volatile ("sd  x5, 0(x28)");
    negfailed();
    break;

  case 59:
    __asm__ volatile ("flw f5, 0(x28)");
    negfailed();
    break;

  case 60:
    __asm__ volatile ("fld f5, 0(x28)");
    negfailed();
    break;

  case 61:
    __asm__ volatile ("fsw f5, 0(x28)");
    negfailed();
    break;

  case 62:
    __asm__ volatile ("fsd f5, 0(x28)");
    negfailed();
    break;

  //
  // branch predicate uses secret GPR
  //
  case 63:
    __asm__ volatile ("beq  x28, x0, 0");
    negfailed();
    break;

  case 64:
    __asm__ volatile ("bne  x28, x0, 0");
    negfailed();
    break;

  case 65:
    __asm__ volatile ("blt  x28, x0, 0");
    negfailed();
    break;

  case 66:
    __asm__ volatile ("bge  x28, x0, 0");
    negfailed();
    break;

  case 67:
    __asm__ volatile ("bltu x28, x0, 0");
    negfailed();
    break;

  case 68:
    __asm__ volatile ("bgeu x28, x0, 0");
    negfailed();
    break;

  case 69:
    __asm__ volatile ("beq  x0,  x28, 0");
    negfailed();
    break;

  case 70:
    __asm__ volatile ("bne  x0,  x28, 0");
    negfailed();
    break;

  case 71:
    __asm__ volatile ("blt  x0,  x28, 0");
    negfailed();
    break;

  case 72:
    __asm__ volatile ("bge  x0,  x28, 0");
    negfailed();
    break;

  case 73:
    __asm__ volatile ("bltu  x0,  x28, 0");
    negfailed();
    break;

  case 74:
    __asm__ volatile ("bgeu x0,  x28, 0");
    negfailed();
    break;

  //
  // code pointer (jalr) uses secret GPR
  //
  case 75:
    __asm__ volatile ("jalr x0, 0(x28)");
    negfailed();
    break;

  //
  // FP arithmetic to non-secret FP dest with secret operand(s) (S)
  //
  case 76:
    __asm__ volatile ("fadd.s   f5, f6,  f28");
    negfailed();
    break;

  case 77:
    __asm__ volatile ("fsub.s   f5, f28, f6");
    negfailed();
    break;

  case 78:
    __asm__ volatile ("fsub.s   f5, f6,  f28");
    negfailed();
    break;

  case 79:
    __asm__ volatile ("fmul.s   f5, f28, f6");
    negfailed();
    break;

  case 80:
    __asm__ volatile ("fmul.s   f5, f6,  f28");
    negfailed();
    break;

  case 81:
    __asm__ volatile ("fdiv.s   f5, f28, f6");
    negfailed();
    break;

  case 82:
    __asm__ volatile ("fdiv.s   f5, f6,  f28");
    negfailed();
    break;

  case 83:
    __asm__ volatile ("fsqrt.s  f5, f28");
    negfailed();
    break;

  case 84:
    __asm__ volatile ("fmin.s   f5, f28, f6");
    negfailed();
    break;

  case 85:
    __asm__ volatile ("fmin.s   f5, f6,  f28");
    negfailed();
    break;

  case 86:
    __asm__ volatile ("fmax.s   f5, f28, f6");
    negfailed();
    break;

  case 87:
    __asm__ volatile ("fmax.s   f5, f6,  f28");
    negfailed();
    break;

  case 88:
    __asm__ volatile ("fsgnj.s  f5, f28, f28");
    negfailed();
    break;

  case 89:
    __asm__ volatile ("fsgnjn.s f5, f28, f28");
    negfailed();
    break;

  case 90:
    __asm__ volatile ("fsgnjx.s f5, f28, f28");
    negfailed();
    break;

  case 91:
    __asm__ volatile ("fmadd.s  f5, f28, f6, f7");
    negfailed();
    break;

  case 92:
    __asm__ volatile ("fmsub.s  f5, f28, f6, f7");
    negfailed();
    break;

  case 93:
    __asm__ volatile ("fnmsub.s f5, f28, f6, f7");
    negfailed();
    break;

  case 94:
    __asm__ volatile ("fnmadd.s f5, f28, f6, f7");
    negfailed();
    break;

  //
  // FP arithmetic to non-secret FP dest with secret operand(s) (D)
  //
  case 95:
    __asm__ volatile ("fadd.d   f5, f28, f6");
    negfailed();
    break;

  case 96:
    __asm__ volatile ("fadd.d   f5, f6,  f28");
    negfailed();
    break;

  case 97:
    __asm__ volatile ("fsub.d   f5, f28, f6");
    negfailed();
    break;

  case 98:
    __asm__ volatile ("fsub.d   f5, f6,  f28");
    negfailed();
    break;

  case 99:
    __asm__ volatile ("fmul.d   f5, f28, f6");
    negfailed();
    break;

  case 100:
    __asm__ volatile ("fmul.d   f5, f6,  f28");
    negfailed();
    break;

  case 101:
    __asm__ volatile ("fdiv.d   f5, f28, f6");
    negfailed();
    break;

  case 102:
    __asm__ volatile ("fdiv.d   f5, f6,  f28");
    negfailed();
    break;

  case 103:
    __asm__ volatile ("fsqrt.d  f5, f28");
    negfailed();
    break;

  case 104:
    __asm__ volatile ("fmin.d   f5, f28, f6");
    negfailed();
    break;

  case 105:
    __asm__ volatile ("fmin.d   f5, f6,  f28");
    negfailed();
    break;

  case 106:
    __asm__ volatile ("fmax.d   f5, f28, f6");
    negfailed();
    break;

  case 107:
    __asm__ volatile ("fmax.d   f5, f6,  f28");
    negfailed();
    break;

  case 108:
    __asm__ volatile ("fsgnj.d  f5, f28, f28");
    negfailed();
    break;

  case 109:
    __asm__ volatile ("fsgnjn.d f5, f28, f28");
    negfailed();
    break;

  case 110:
    __asm__ volatile ("fsgnjx.d f5, f28, f28");
    negfailed();
    break;

  case 111:
    __asm__ volatile ("fmadd.d  f5, f28, f6, f7");
    negfailed();
    break;

  case 112:
    __asm__ volatile ("fmsub.d  f5, f28, f6, f7");
    negfailed();
    break;

  case 113:
    __asm__ volatile ("fnmsub.d f5, f28, f6, f7");
    negfailed();
    break;

  case 114:
    __asm__ volatile ("fnmadd.d f5, f28, f6, f7");
    negfailed();
    break;

  //
  // FP compares/class to non-secret GPR with secret sources
  //
  case 115:
    __asm__ volatile ("feq.s    x5, f28, f6");
    negfailed();
    break;

  case 116:
    __asm__ volatile ("feq.s    x5, f6,  f28");
    negfailed();
    break;

  case 117:
    __asm__ volatile ("flt.s    x5, f28, f6");
    negfailed();
    break;

  case 118:
    __asm__ volatile ("fle.s    x5, f28, f6");
    negfailed();
    break;

  case 119:
    __asm__ volatile ("fclass.s x5, f28");
    negfailed();
    break;

  case 120:
    __asm__ volatile ("feq.d    x5, f28, f6");
    negfailed();
    break;

  case 121:
    __asm__ volatile ("feq.d    x5, f6,  f28");
    negfailed();
    break;

  case 122:
    __asm__ volatile ("flt.d    x5, f28, f6");
    negfailed();
    break;

  case 123:
    __asm__ volatile ("fle.d    x5, f28, f6");
    negfailed();
    break;

  case 124:
    __asm__ volatile ("fclass.d x5, f28");
    negfailed();
    break;

  //
  // FP/GPR moves & converts: secret -> non-secret
  //
  case 125:
    __asm__ volatile ("fmv.x.w  x5,  f28");
    negfailed();
    break;

  case 126:
    __asm__ volatile ("fmv.x.d  x5,  f28");
    negfailed();
    break;

  case 127:
    __asm__ volatile ("fmv.s.x  f5,  x28");
    negfailed();
    break;

  case 128:
    __asm__ volatile ("fmv.d.x  f5,  x28");
    negfailed();
    break;

  case 129:
    __asm__ volatile ("fcvt.w.s  x5,  f28");
    negfailed();
    break;

  case 130:
    __asm__ volatile ("fcvt.wu.s x5,  f28");
    negfailed();
    break;

  case 131:
    __asm__ volatile ("fcvt.l.s  x5,  f28");
    negfailed();
    break;

  case 132:
    __asm__ volatile ("fcvt.lu.s x5,  f28");
    negfailed();
    break;

  case 133:
    __asm__ volatile ("fcvt.w.d  x5,  f28");
    negfailed();
    break;

  case 134:
    __asm__ volatile ("fcvt.wu.d x5,  f28");
    negfailed();
    break;

  case 135:
    __asm__ volatile ("fcvt.l.d  x5,  f28");
    negfailed();
    break;

  case 136:
    __asm__ volatile ("fcvt.lu.d x5,  f28");
    negfailed();
    break;

  case 137:
    __asm__ volatile ("fcvt.s.w  f5,  x28");
    negfailed();
    break;

  case 138:
    __asm__ volatile ("fcvt.s.wu f5,  x28");
    negfailed();
    break;

  case 139:
    __asm__ volatile ("fcvt.s.l  f5,  x28");
    negfailed();
    break;

  case 140:
    __asm__ volatile ("fcvt.s.lu f5,  x28");
    negfailed();
    break;

  case 141:
    __asm__ volatile ("fcvt.d.w  f5,  x28");
    negfailed();
    break;

  case 142:
    __asm__ volatile ("fcvt.d.wu f5,  x28");
    negfailed();
    break;

  case 143:
    __asm__ volatile ("fcvt.d.l  f5,  x28");
    negfailed();
    break;

  case 144:
    __asm__ volatile ("fcvt.d.lu f5,  x28");
    negfailed();
    break;

  //
  // stores of secret FP to plain memory
  //
  case 145:
    __asm__ volatile ("fsw f28, 0(x1)");
    negfailed();
    break;

  case 146:
    __asm__ volatile ("fsd f28, 0(x1)");
    negfailed();
    break;

  //
  // compressed moves/stores with secret sources
  //
  case 147:
    __asm__ volatile ("c.mv   x5, x28");
    negfailed();
    break;

  case 148:
    __asm__ volatile ("c.add  x5, x28");
    negfailed();
    break;

  case 149:
    __asm__ volatile ("c.swsp x28, 0(sp)");
    negfailed();
    break;

  case 150:
    __asm__ volatile ("c.sdsp x28, 0(sp)");
    negfailed();
    break;

  case 151:
    __asm__ volatile ("c.fsdsp f28, 0(sp)");
    negfailed();
    break;

  //
  // atomics with secret sources / addresses
  //
  case 152:
    __asm__ volatile ("lr.d      x5, (x28)");
    negfailed();
    break;

  case 153:
    __asm__ volatile ("sc.d      x5, x6,   (x28)");
    negfailed();
    break;

  case 154:
    __asm__ volatile ("sc.d      x5, x28,  (x1)");
    negfailed();
    break;

  case 155:
    __asm__ volatile ("amoswap.d x5, x28,  (x1)");
    negfailed();
    break;

  case 156:
    __asm__ volatile ("amoadd.d  x5, x28,  (x1)");
    negfailed();
    break;

  case 157:
    __asm__ volatile ("amoxor.d  x5, x28,  (x1)");
    negfailed();
    break;

  case 158:
    __asm__ volatile ("amoor.d   x5, x28,  (x1)");
    negfailed();
    break;

  case 159:
    __asm__ volatile ("amoand.d  x5, x28,  (x1)");
    negfailed();
    break;

  case 160:
    __asm__ volatile ("amomin.d  x5, x28,  (x1)");
    negfailed();
    break;

  case 161:
    __asm__ volatile ("amomax.d  x5, x28,  (x1)");
    negfailed();
    break;

  case 162:
    __asm__ volatile ("amominu.d x5, x28,  (x1)");
    negfailed();
    break;

  case 163:
    __asm__ volatile ("amomaxu.d x5, x28,  (x1)");
    negfailed();
    break;

  case 164:
    __asm__ volatile ("amoswap.d x5, x6,   (x28)");
    negfailed();
    break;

  case 165:
    __asm__ volatile ("amoadd.d  x5, x6,   (x28)");
    negfailed();
    break;

  case 166:
    __asm__ volatile ("amoxor.d  x5, x6,   (x28)");
    negfailed();
    break;

  case 167:
    __asm__ volatile ("amoor.d   x5, x6,   (x28)");
    negfailed();
    break;

  case 168:
    __asm__ volatile ("amoand.d  x5, x6,   (x28)");
    negfailed();
    break;

  case 169:
    __asm__ volatile ("amomin.d  x5, x6,   (x28)");
    negfailed();
    break;

  case 170:
    __asm__ volatile ("amomax.d  x5, x6,   (x28)");
    negfailed();
    break;

  case 171:
    __asm__ volatile ("amominu.d x5, x6,   (x28)");
    negfailed();
    break;

  case 172:
    __asm__ volatile ("amomaxu.d x5, x6,   (x28)");
    negfailed();
    break;

  case 173:
    __asm__ volatile ("lr.w      x5, (x28)");
    negfailed();
    break;

  case 174:
    __asm__ volatile ("sc.w      x5, x6,   (x28)");
    negfailed();
    break;

  case 175:
    __asm__ volatile ("sc.w      x5, x28,  (x1)");
    negfailed();
    break;

  case 176:
    __asm__ volatile ("amoswap.w x5, x28,  (x1)");
    negfailed();
    break;

  case 177:
    __asm__ volatile ("amoadd.w  x5, x28,  (x1)");
    negfailed();
    break;

  case 178:
    __asm__ volatile ("amoxor.w  x5, x28,  (x1)");
    negfailed();
    break;

  case 179:
    __asm__ volatile ("amoor.w   x5, x28,  (x1)");
    negfailed();
    break;

  case 180:
    __asm__ volatile ("amoand.w  x5, x28,  (x1)");
    negfailed();
    break;

  case 181:
    __asm__ volatile ("amomin.w  x5, x28,  (x1)");
    negfailed();
    break;

  case 182:
    __asm__ volatile ("amomax.w  x5, x28,  (x1)");
    negfailed();
    break;

  case 183:
    __asm__ volatile ("amominu.w x5, x28,  (x1)");
    negfailed();
    break;

  case 184:
    __asm__ volatile ("amomaxu.w x5, x28,  (x1)");
    negfailed();
    break;

  case 185:
    __asm__ volatile ("amoswap.w x5, x6,   (x28)");
    negfailed();
    break;

  case 186:
    __asm__ volatile ("amoadd.w  x5, x6,   (x28)");
    negfailed();
    break;

  case 187:
    __asm__ volatile ("amoxor.w  x5, x6,   (x28)");
    negfailed();
    break;

  case 188:
    __asm__ volatile ("amoor.w   x5, x6,   (x28)");
    negfailed();
    break;

  case 189:
    __asm__ volatile ("amoand.w  x5, x6,   (x28)");
    negfailed();
    break;

  case 190:
    __asm__ volatile ("amomin.w  x5, x6,   (x28)");
    negfailed();
    break;

  case 191:
    __asm__ volatile ("amomax.w  x5, x6,   (x28)");
    negfailed();
    break;

  case 192:
    __asm__ volatile ("amominu.w x5, x6,   (x28)");
    negfailed();
    break;

  case 193:
    __asm__ volatile ("amomaxu.w x5, x6,   (x28)");
    negfailed();
    break;


  // --- CSR & SFENCE with secret operands (affecting public state) ---
  case 194:
    __asm__ volatile ("csrrs x5, fcsr,    x28");
    negfailed();
    break;

  case 195:
    __asm__ volatile ("csrrw x5, fcsr,    x28");
    negfailed();
    break;

  case 196:
    __asm__ volatile ("csrrc x5, fcsr,    x28");
    negfailed();
    break;

  case 197:
    __asm__ volatile ("sfence.vma x28, x0");
    negfailed();
    break;

  case 198:
    __asm__ volatile ("sfence.vma x0,  x28");
    negfailed();
    break;

  default:
    libmin_printf("ERROR: invalid test (%u).\n", (uint32_t)mojov_arg);
    break;
  }

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

