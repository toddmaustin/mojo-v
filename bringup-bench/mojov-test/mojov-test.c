#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-fast.h"

typedef unsigned __int128 uint128_t;

// Mojo-V asm instruction definitions (using the format-friendly .insn directive in GNU AS
#define LDE(rd,base,ofs) ".insn i 0xb, 0x0, " #rd ", " #base ", " #ofs "\n\t"
#define SDE(src,base,ofs) ".insn s 0xb, 0x1, " #src ", " #ofs "(" #base ")\n\t"

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
  union mojov_memfmt_t {
    uint128_t ct;     // ciphertext

    struct {          // plaintext
      uint64_t val;     // register plaintext value
      uint32_t salt;    // random salt
      uint32_t sig;     // fixed signature
    } pt;
  };
  uint128_t simon_key = SIMON128_KEY;
  simon_state_t simon_state;
  simon_128_128_keyexpand(&simon_state, simon_key, 68);

  //
  // mprivregcfg tests
  //
  libmin_printf("** Running CSR[privreg] tests...\n");

  uint64_t val;

  // read reset value
  val = mojov_read_mprivregcfg();
  libmin_printf("Initial mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  // enable private register semantics (bit 0 = 1)
  if (mojov_enable_and_verify() != 0)
    return -1;

  val = mojov_read_mprivregcfg();
  libmin_printf("After enable, mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");


  // do some secret computation
  libmin_printf("Initial inputs: x:%lu, max:%lu\n", x, max);

  // inline assembly block
  __asm__ volatile (
    // first encrypt the public X and MAX values
    "ld  x24, (%0)\n\t"
    SDE (x24,%2,0)
    "ld  x24, (%1)\n\t"
    SDE (x24,%3,0)

    // test-load a bogus ciphertext value -- it should get an exception
    // LDE  (x24, %4, 0)

    // cannot ld/sd a secret register
    // "sd   x24, (%0)\n\t"
    // "sd   x15, (%0)\n\t"

    // load third-party encrypted operands
    LDE(x24, %2, 0)
    LDE(x25, %3, 0)

    // Condition: (max < x)?
    // "slt       /*p2*/x26, x1, x2\n\t" // Mojo-V test: no secret inputs
    // "jalr         ra, 64(x25)\n\t"
    // "sw        x26, (x24)\n\t"
    // "bne       x24, x15, .+12\n\t"
    // "bne       x15, x24, .+12\n\t"
    // "slt       x15, /*p1*/x25, /*p0*/x24\n\t" // Mojo-V test: should have secret dest
    "slt   x26, x25, x24\n\t" /* p2 = (p1 < p0) ? 1 : 0 */

    // try to move the secret predicate, via integer to FP register/ moves/converts
    "fmv.w.x      f1, t2\n\t"
    "fcvt.s.w     f3, t2\n\t"
    // "fmv.w.x      f1, x26\n\t"
    // "fmv.d.x      f2, x26\n\t"
    // "fcvt.s.w     f3, x26\n\t"
    // "fcvt.s.wu    f3, x26\n\t"
    // "fcvt.s.l     f5, x26\n\t"
    // "fcvt.s.lu    f6, x26\n\t"
    // "fcvt.d.w     f1, x26\n\t"
    // "fcvt.d.wu    f2, x26\n\t"
    // "fcvt.d.l     f3, x26\n\t"
    // "fcvt.d.lu    f4, x26\n\t"

    // Build data-oblivious conditional result
    "czero.eqz x24, x24, x26\n\t" // if p2==0 => p0=0, else p0=x
    "czero.nez x25, x25, x26\n\t" // if p2!=0 => p1=0, else p1=max
    "or        x27, x24, x25\n\t" // select: p3 = (x if x>max else max)

    // Store third-party encrypted (potentially) new max value
    SDE(x27,%3,0)

    :
    : "r" (&x), "r" (&max), "r" (&x_enc), "r" (&max_enc), "r" (&bogus_enc) // input operands
    : "x24", "x25", "x26", "x27", "x15" // clobbered registers
  );

  // decrypt results locally
  union mojov_memfmt_t x_check, max_check;
  simon_128_128_decrypt(&simon_state, x_enc, &x_check.ct);
  simon_128_128_decrypt(&simon_state, max_enc, &max_check.ct);

  if (x_check.pt.sig != (uint32_t)CONTRACT_SIG || max_check.pt.sig != (uint32_t)CONTRACT_SIG)
  {
    libmin_printf("ERROR: local decrypt validation failed.\n");
    return -1;
  }

  libmin_printf("Final results:   x:%lu, max:%lu\n", x_check.pt.val, max_check.pt.val);


  // disable private register semantics (write 0)
  mojov_write_mprivregcfg(0);

  val = mojov_read_mprivregcfg();
  libmin_printf("After disable, mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  libmin_success();
  return 0;
}

