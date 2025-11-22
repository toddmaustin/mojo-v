#include "libmin.h"
#include "simon.h"

typedef unsigned __int128 uint128_t;

volatile double iszero = 0.0;

#define SECRET

uint16_t mojov_arg;

#define SM64_BASIS 0x9e3779b97f4a7c15ull

extern inline uint64_t
sm64_init(void)
{
  return SM64_BASIS;
}

extern inline uint64_t
sm64_hash64(uint64_t h, uint64_t v)
{
  v += 0x9e3779b97f4a7c15ull;
  v = (v ^ (v >> 30)) * 0xbf58476d1ce4e5b9ull;
  v = (v ^ (v >> 27)) * 0x94d049bb133111ebull;
  v ^= (v >> 31);
  return h ^ v;
}

extern inline uint64_t
__instret(void)
{
  uint64_t insts;
  __asm__ volatile ("rdinstret %0" : "=r"(insts));

  return insts;
}

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
                ((val >> 2) & 0x03) == 2 ? "proof-carrying" : ((((val >> 2) & 0x03) == 1) ? "strong" : "fast"),
                (val >> 4) & 0xff);
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

// SECRET int
// secret_cmov(SECRET bool p, SECRET int x, SECRET int y)
// {
//   return (int)p*x + (int)!p*y;
// }

#define MOJOV_PT_SIG   0xdeadbeef

// proofcarrying memory format
union mojov_mem_proofcarrying_t {
  struct {               // ciphertext
    uint128_t ct_lo;       // ciphertext low 128-bits
    uint128_t ct_hi;       // ciphertext high 128-bits
  } ct;
  struct { // 256-bits in size, 128-bit alignment
    double   val; // register plaintext value
    uint64_t salt; // random salt
    uint64_t auth_sig; // authentication signature (from contract)
    uint64_t metadata; // target-specific metadata (e.g., device hash, overflow flag)
  } pt;
};

union mojov_imem_proofcarrying_t {
  struct {               // ciphertext
    uint128_t ct_lo;       // ciphertext low 128-bits
    uint128_t ct_hi;       // ciphertext high 128-bits
  } ct;
  struct { // 256-bits in size, 128-bit alignment
    uint64_t  val; // register plaintext value
    uint64_t salt; // random salt
    uint64_t auth_sig; // authentication signature (from contract)
    uint64_t metadata; // target-specific metadata (e.g., device hash, overflow flag)
  } pt;
};

uint128_t simon_key = GEN128(0x0f0e0d0c0b0a0908, 0x0706050403020100);
simon_state_t simon_state;

inline extern double
secret_decrypt(union mojov_mem_proofcarrying_t ctval)
{
  union mojov_mem_proofcarrying_t ptval;
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi = ptval.ct.ct_hi ^ ctval.ct.ct_lo;
  return ptval.pt.val;
}

inline extern uint64_t
secret_dfhash(union mojov_mem_proofcarrying_t ctval)
{
  union mojov_mem_proofcarrying_t ptval;
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi = ptval.ct.ct_hi ^ ctval.ct.ct_lo;
  return ptval.pt.metadata;
}

inline extern uint64_t
secret_idecrypt(union mojov_imem_proofcarrying_t ctval)
{
  union mojov_imem_proofcarrying_t ptval;
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_lo, &ptval.ct.ct_lo);
  simon_128_128_decrypt(&simon_state, ctval.ct.ct_hi, &ptval.ct.ct_hi);
  ptval.ct.ct_hi = ptval.ct.ct_hi ^ ctval.ct.ct_lo;
  return ptval.pt.val;
}

inline extern void
secret_print(union mojov_mem_proofcarrying_t ct)
{
  libmin_printf("0x%08x%08x%08x%08x",
    (uint32_t)(ct.ct.ct_hi >> 96),
    (uint32_t)(ct.ct.ct_hi >> 64),
    (uint32_t)(ct.ct.ct_hi >> 32),
    (uint32_t)ct.ct.ct_hi);
  libmin_printf("%08x%08x%08x%08x",
    (uint32_t)(ct.ct.ct_lo >> 96),
    (uint32_t)(ct.ct.ct_lo >> 64),
    (uint32_t)(ct.ct.ct_lo >> 32),
    (uint32_t)ct.ct.ct_lo);
}


static
int rb(int n)                 // uniform in [0..n-1], no modulo bias
{
  int limit = RAND_MAX - (RAND_MAX % n);
  int r; do { r = libmin_rand(); } while (r > limit);
  return r % n;
}

double
genrand_fp64(void)
{
  int idigits = 0 + rb(11);         // 0..10
  double v = 0.0;

  // integer part: first digit 1..9, then (idigits-1) digits 0..9
  v = (double)(1 + rb(9));
  for (int i = 1; i < idigits; ++i)
    v = v * 10.0 + (double)rb(10);

  // fractional part: exactly 5 digits
  double frac = 0.0;
  for (int i = 0; i < 5; ++i)
    frac = frac * 10.0 + (double)rb(10);

  v += frac / 100000.0;              // add 5-digit fraction

  // random sign
  if (rb(2)) v = -v;
  return v;
}

union mojov_mem_proofcarrying_t
secret_3rdparty(double dblval, uint64_t in_brand)
{
  uint64_t dfhash = sm64_hash64(sm64_init(), in_brand);
  union mojov_mem_proofcarrying_t ptval = {.pt = { dblval, ((uint64_t)libmin_rand() << 32) | (uint64_t)libmin_rand(), MOJOV_PT_SIG, dfhash} };
  union mojov_mem_proofcarrying_t ctval;

  // encrypt the memory packet with the processor's internal key
  simon_128_128_encrypt(&simon_state, ptval.ct.ct_lo, &ctval.ct.ct_lo);
  simon_128_128_encrypt(&simon_state, (ptval.ct.ct_hi ^ ctval.ct.ct_lo), &ctval.ct.ct_hi);

  return ctval;
}

// supported sizes: 64, 128, 256 (default), 512, 1024, 2048
#define DATASET_SIZE 64
double raw_data[DATASET_SIZE];
SECRET union mojov_mem_proofcarrying_t secret_data[DATASET_SIZE];

// now sum the array data to coalesce the dataflow hashes
union mojov_mem_proofcarrying_t sum_enc;

// total swaps executed so far
union mojov_imem_proofcarrying_t swaps;

void
print_data(double *data, unsigned size)
{
  double sum = 0.0;
  libmin_printf("DATA DUMP:\n");
  for (unsigned i=0; i < size; i++)
  {
    sum += data[i];
    libmin_printf("  data[%4u] = %.20lf, ct =[", i, data[i]);
    secret_print(secret_data[i]);
    libmin_printf("]\n");
  }
  libmin_printf("Total sum = %.20lf\n", sum);
}

void
bubblesort(union mojov_mem_proofcarrying_t *data, unsigned size)
{
  for (unsigned i=0; i < size-1; i++)
  {
    for (unsigned j=0; j < size-1; j++)
    {
      // swap needed?
      __asm__ volatile (
        // SECRET bool swap = (data[j] > data[j+1]);
        FLDE(  f28, %0, 0) // data[j]
        FLDE(  f29, %1, 0) // data[j+1]
        "flt.d  x30, f29, f28\n\t" // swap
        // fmv the values for the conditional swap
        "fmv.x.d x28, f28\n\t"
        "fmv.x.d x29, f29\n\t"
        // perform the swap
        // data[j] = secret_cmov(swap, data[j+1], data[j]);
        "czero.eqz x31, x29, x30\n\t"
        "czero.nez x30, x28, x30\n\t"
        "or        x31, x30, x31\n\t"
        "fmv.d.x f30, x31\n\t"
        FSDE(       f30, %0, 0)
        // data[j+1] = secret_cmov(swap, tmp, data[j+1]);
        "flt.d  x30, f29, f28\n\t" // swap
        "czero.eqz x31, x28, x30\n\t"
        "czero.nez x28, x29, x30\n\t"
        "or        x31, x28, x31\n\t" 
        "fmv.d.x f30, x31\n\t"
        FSDE(       f30, %1, 0)
        // count the number of swaps executed
        // swaps = secret_cmov(swap, swaps+1, swaps);
        LDE(  x28, %2, 0) // swaps
        "add x29, x28, x30\n\t" // swaps+1
        // "czero.eqz f31, f28, f30\n\t"
        // "czero.nez f30, f29, f30\n\t"
        // "or        f31, f30, f31\n\t"
        SDE(  x29, %2, 0) // swaps
        :
        : "r" (&data[j]), "r" (&data[j+1]), "r" (&swaps)
        : "x28", "x29", "x30", "x31", "f28", "f29", "f30", "f31" // clobbered registers
      );
    }
  }

  // clean sum_enc
  __asm__ volatile (
    "fmv.d.x  f28, x0\n\t"
    FSDE     (f28, %0, 0)
    :
    : "r" (&sum_enc)
    : "f28" // clobbered registers
  );

  for (unsigned i=0; i < size; i++)
  {
    if (mojov_arg == 5) 
    {
      __asm__ volatile (
        FLDE     (f29, %0, 0)         // load data[i]

        // DFHASH TEST: change fadd.d to fsub.d
        "fsub.d   f28, f28, f29\n\t"  // add to sum_enc
        :
        : "r" (&data[i])
        : "f28", "f29" // clobbered registers
      );
    }
    else if (mojov_arg == 6)
    {
      __asm__ volatile (
        FLDE     (f29, %0, 0)         // load data[i]

        // DFHASH TEST: change any operand order
        "fadd.d   f28, f29, f28\n\t"  // add to sum_enc
        :
        : "r" (&data[i])
        : "f28", "f29" // clobbered registers
      );
    }
    else
    {
      __asm__ volatile (
        FLDE     (f29, %0, 0)         // load data[i]
        "fadd.d   f28, f28, f29\n\t"  // add to sum_enc
        :
        : "r" (&data[i])
        : "f28", "f29" // clobbered registers
      );
    }
  }

  // save sum_enc to memory
  __asm__ volatile (
    FSDE  (f28, %0, 0)
    :
    : "r" (&sum_enc)
    : "f28" // clobbered registers
  );

}

int
main(void)
{
  // initilize cipher engine, for checking results
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
  mojov_arg = (val >> 12) & 0xffff;

  // enable private register semantics (bit 0 = 1)
  write_mprivregcfg(1);

  val = read_mprivregcfg();
  libmin_printf("After enable, mprivregcfg = 0x%lx, ", val);
  print_mprivregcfg(val);
  libmin_printf("\n");

  // initialize the pseudo-RNG
  libmin_srand(42);

  // initialize swaps
  // swaps = 0;
  __asm__ volatile (
    "mv   t3, x0\n\t"
    SDE  (t3, %0, 0)
    :
    : "r" (&swaps)
    : "t3" // clobbered registers
  );

  // initialize the array to sort with 3rd party input-branded data
  for (unsigned i=0; i < DATASET_SIZE; i++)
  {
    if (mojov_arg == 1)
    {
      // DFHASH TEST: modifying input data should not affect dfhash
      raw_data[i] = genrand_fp64() * 0.67;
    }
    else if (mojov_arg == 2)
    {
      // DFHASH TEST: modifying input data should not affect dfhash
      raw_data[i] = genrand_fp64() + genrand_fp64(); 
    }
    else if (mojov_arg == 3)
    {
      // DFHASH TEST: modifying input data should not affect dfhash
      raw_data[i] = genrand_fp64() + genrand_fp64(); 
    }
    else
      raw_data[i] = genrand_fp64();

    secret_data[i] = secret_3rdparty(raw_data[i], /* input type */91);
  }
  print_data(raw_data, DATASET_SIZE);

  if (mojov_arg == 4)
  {
    // swap array elements 12 and 13
    union mojov_mem_proofcarrying_t tmp = secret_data[12];
    secret_data[12] = secret_data[13];
    secret_data[13] = tmp;
  }

  // DFHASH baseline test
  {
    // performance monitoring
    uint64_t icnt_start = __instret();

    bubblesort(secret_data, DATASET_SIZE);

    uint64_t icnt_end = __instret();
    libmin_printf("INFO: bubblesort inst count = %lu.\n", icnt_end - icnt_start + 1);
  }


  // decrypt the array
  for (unsigned i=0; i < DATASET_SIZE; i++)
    raw_data[i] = secret_decrypt(secret_data[i]);
  print_data(raw_data, DATASET_SIZE);

  // check the array
  for (unsigned i=0; i < DATASET_SIZE-1; i++)
  {
    if (raw_data[i] > raw_data[i+1])
    {
      libmin_printf("ERROR: data is not properly sorted.\n");
      return -1;
    }
  }

  double sum = secret_decrypt(sum_enc);
  libmin_printf("INFO: final summary variable: %.20lf\n", sum);

  if (mojov_arg == 7)
  {
    uint64_t dfhash = secret_dfhash(secret_data[DATASET_SIZE-1]);
    libmin_printf("INFO: final dataflow hash: 0x%08x%08x\n", (uint32_t)(dfhash >> 32), (uint32_t)dfhash);
  }
  else
  {
    uint64_t dfhash = secret_dfhash(sum_enc);
    libmin_printf("INFO: final dataflow hash: 0x%08x%08x\n", (uint32_t)(dfhash >> 32), (uint32_t)dfhash);
  }

  libmin_printf("INFO: %lu swaps executed.\n", secret_idecrypt(swaps));
  libmin_printf("INFO: data is properly sorted.\n");

  libmin_success();
  return 0;
}
