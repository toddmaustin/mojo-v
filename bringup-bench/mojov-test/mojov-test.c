#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"
#include "dc-fast.h"

typedef unsigned __int128 uint128_t;

uint128_t simon_key = SIMON128_KEY;
simon_state_t simon_state;

// Mojo-V asm instruction definitions (using the format-friendly .insn directive in GNU AS
#define LDE(rd,base,ofs) ".insn i 0xb, 0x0, " #rd ", " #base ", " #ofs "\n\t"
#define SDE(src,base,ofs) ".insn s 0xb, 0x1, " #src ", " #ofs "(" #base ")\n\t"

// Predefined memory values
uint64_t x = 35;
uint64_t max = 25;

mojov_mem_fast_u64_t x_enc;
mojov_mem_fast_u64_t max_enc;
mojov_mem_fast_u64_t bogus_enc = {.ct = 42};

int
main(void)
{
  uint64_t val;

  // initilize Mojo-V and open a fast-mode contract
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  // initilize cipher engine, for checking results
  simon_128_128_keyexpand(&simon_state, simon_key, 68);

  //
  // mprivregcfg tests
  //
  libmin_printf("** Running CSR[privreg] tests...\n");

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

  // decrypt results locally and validate computation and signature
  uint64_t x_check = mojov_decrypt_fast_u64(&simon_state, x_enc, CONTRACT_SIG);
  uint64_t max_check = mojov_decrypt_fast_u64(&simon_state, max_enc, CONTRACT_SIG);

  // check that the computation was successful
  if (x_check != 35 || max_check != 35)
  {
    libmin_printf("ERROR: computation results are incorrect. (x_enc == %lu, max_enc == %lu)\n", x_check, max_check);
    return -1;
  }

  // looks good, output the correct results
  libmin_printf("Final results:   x:%lu, max:%lu\n", x_check, max_check);

  // disable private register semantics (write 0)
  mojov_write_mprivregcfg(0);

  val = mojov_read_mprivregcfg();
  libmin_printf("After disable, mprivregcfg = 0x%lx, ", val);
  mojov_print_mprivregcfg(val);
  libmin_printf("\n");

  libmin_success();
  return 0;
}

