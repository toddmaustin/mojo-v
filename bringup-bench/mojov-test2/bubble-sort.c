#include "libmin.h"
#include "simon.h"
#include "dc-fast.h"

typedef unsigned __int128 uint128_t;

#define SECRET

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

// Define your custom CSR number
#define CSR_MPRIVREGCFG 0x0a0
#define CSR_MOJOV_KMSM_ADDR 0x0a1
#define CSR_MOJOV_KMSM_DATA 0x0a2
#define CSR_MOJOV_KMSM_CTRL 0x0a3

#define KMSM_PUBKEY_WORDS 256
#define KMSM_ENCKEY_WORDS 128

static const char *DC_FAST_KEM_HEX =
  "aeef3d701933d8cf8c884251c632e35ed37e894de486e7a307fa30b5b9abc82995b19adbe4861688bd1f261084a4dad58a1eb3c686c4d1a172a6604bdb7c8054a6271bd77bfa340b46a66509e5c7bfc90e863cd05b2d5b2f86453fab7fbf24e05ced6b10485f5c85f902be1a9ef25986568abc7cdfb4eb56bc799f18afef7ba85645c078c08c643e05d3ca48b241a9261d8a05b3b65e2b4b9a95a8b1071934a54717ee41297fd5ed8b30ff3fc7371ddf061b4847b8be2c85a397ae2873fad50d10dd28ed440398a6768c63ea09a266c781e78aa386c2549791f69170f73a09e2e24b8834525457c35ff089bd7b4ae232d946c0361f5564aa5a47005c978fe371dacac0ed4fe1f5839e275960887fe26d9a4011f96d8b079e7578fad158bc89b4b748565f8be6c3f7d09cb6ef665008e5a6e2e7849a28bf512ec55f9f0ed0b7464b3b0bdf87b7c32ac265bbae6d3fad00f5bd4a9cfbf2494bf0cfc8f725dbc628b873e3018dcb3082df19ad2858a510cde5ccd72daade4a8bc2488a0673f7b47103062a5f7a9cf43943a58f8721be2b030ee4c93441ae81820bc11f1f310b4a2df60d067dbd64db492fbd3289d4846d55a2b4af9964871d3e4aa6d3e7c55d588e8ae5ac8bbda6105d1cd85d78771b71ae379a29e71cd37bf4049289b9de9a35fdde6b58bbbded7b8f354e89add01d460dc2e109071580d736d665446dfd0c3260163f39c39e23bcd9f6ce2a8cf407c42a4f6f525a6ef8174f26346078de920862db5944a7b093cc6b4eee98b168910ff66f981fb8b20d784d1c38f5dd9fabd2f63edd5696a52be0b868e992395f33abea191fa38776b55dc5551dea1b8f70bed25d70974189294017e6a5575c2ca540c905d2fa5b25dceaa7fc2911b3a613a96387074a75d80a062a59c906b8540f47dbd489e331d3bd70d0bf2b972da50ee37b74085315e7e936c02a41e939c25752b9c787185b0202102f170bc3bcee21b3e9259e2db87e2dfa4eb6c45e69ba9776a574d03eafc7cd214bb3d6c700997f471f1036e51cddb6c67b8708ca306d5d27bd9ab75f6724f6f918147947ffaf420d75";
static const char *DC_FAST_MSG_HEX =
  "bd6be9fa8b6c214805a6c52790cd8b31cd06529ac94c814ecd7f8cdd7efc90b316b15c4868075b5ec039d090c18f2a74650050e457150734db07f261aa1d8a12";

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

static inline void write_kmsm_addr(uint64_t value)
{
  __asm__ volatile ("csrw %0, %1" :: "i"(CSR_MOJOV_KMSM_ADDR), "rK"(value));
}

static inline void write_kmsm_data(uint64_t value)
{
  __asm__ volatile ("csrw %0, %1" :: "i"(CSR_MOJOV_KMSM_DATA), "rK"(value));
}

static inline uint64_t read_kmsm_ctrl(void)
{
  uint64_t value;
  __asm__ volatile ("csrr %0, %1" : "=r"(value) : "i"(CSR_MOJOV_KMSM_CTRL));
  return value;
}

static inline void write_kmsm_ctrl(uint64_t value)
{
  __asm__ volatile ("csrw %0, %1" :: "i"(CSR_MOJOV_KMSM_CTRL), "rK"(value));
}

static int hex_nibble(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
  return -1;
}

static int load_hex_words_to_kmsm(const char *hex, uint64_t start_addr)
{
  uint64_t word = 0;
  unsigned nibs = 0;
  write_kmsm_addr(start_addr);

  for (unsigned i = 0; hex[i] != '\0'; i++) {
    int hv = hex_nibble(hex[i]);
    if (hv < 0) {
      continue;
    }
    word = (word << 4) | (uint64_t)hv;
    nibs++;
    if (nibs == 16) {
      write_kmsm_data(word);
      nibs = 0;
      word = 0;
    }
  }

  if (nibs != 0) {
    word <<= (16 - nibs) * 4;
    write_kmsm_data(word);
  }

  return 0;
}

static int configure_kmsm_from_dc(void)
{
  load_hex_words_to_kmsm(DC_FAST_KEM_HEX, KMSM_PUBKEY_WORDS);
  load_hex_words_to_kmsm(DC_FAST_MSG_HEX, KMSM_PUBKEY_WORDS + KMSM_ENCKEY_WORDS);

  write_kmsm_ctrl(1);
  uint64_t ctrl;
  do {
    ctrl = read_kmsm_ctrl();
  } while ((ctrl & 0x6) == 0);

  if (ctrl & 0x2) {
    libmin_printf("INFO: KMSM contract open succeeded (kmsm_ctrl=0x%lx).\n", ctrl);
    return 0;
  }

  libmin_printf("ERROR: KMSM contract open failed (kmsm_ctrl=0x%lx, err=0x%lx).\n", ctrl, (ctrl >> 32));
  return -1;
}

// SECRET int
// secret_cmov(SECRET bool p, SECRET int x, SECRET int y)
// {
//   return (int)p*x + (int)!p*y;
// }

#define MOJOV_PT_SIG   0xdeadbeef
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

inline extern uint64_t
secret_decrypt(uint128_t ct)
{
  union mojov_memfmt_t mempkt;
  simon_128_128_decrypt(&simon_state, ct, &mempkt.ct);
  return mempkt.pt.val;
}

inline extern void
secret_print(uint128_t ct)
{
  libmin_printf("0x%08x%08x%08x%08x",
    (uint32_t)(ct >> 96),
    (uint32_t)(ct >> 64),
    (uint32_t)(ct >> 32),
    (uint32_t)ct);
}


// supported sizes: 256 (default), 512, 1024, 2048
#define DATASET_SIZE 256
uint64_t raw_data[DATASET_SIZE];
SECRET uint128_t secret_data[DATASET_SIZE];

// total swaps executed so far
uint128_t swaps;

void
print_data(uint64_t *data, unsigned size)
{
  libmin_printf("DATA DUMP:\n");
  for (unsigned i=0; i < size; i++)
  {
    libmin_printf("  data[%4u] = %10ld, ct =[", i, data[i]);
    secret_print(secret_data[i]);
    libmin_printf("]\n");
  }
}

void
bubblesort(uint128_t *data, unsigned size)
{
  for (unsigned i=0; i < size-1; i++)
  {
    for (unsigned j=0; j < size-1; j++)
    {
      // swap needed?
      __asm__ volatile (
        // SECRET bool swap = (data[j] > data[j+1]);
        LDE(  t3, %0, 0) // data[j]
        LDE(  t4, %1, 0) // data[j+1]
        "slt  t5, t4, t3\n\t" // swap
        // perform the swap
        // data[j] = secret_cmov(swap, data[j+1], data[j]);
        "czero.eqz t6, t4, t5\n\t"
        "czero.nez t5, t3, t5\n\t"
        "or        t6, t5, t6\n\t"
        SDE(       t6, %0, 0)
        // data[j+1] = secret_cmov(swap, tmp, data[j+1]);
        "slt  t5, t4, t3\n\t" // swap
        "czero.eqz t6, t3, t5\n\t"
        "czero.nez t3, t4, t5\n\t"
        "or        t6, t3, t6\n\t" 
        SDE(       t6, %1, 0)
        // count the number of swaps executed
        // swaps = secret_cmov(swap, swaps+1, swaps);
        LDE(  t3, %2, 0) // swaps
        "add t4, t3, t5\n\t" // swaps+1
        // "czero.eqz t6, t3, t5\n\t"
        // "czero.nez t5, t4, t5\n\t"
        // "or        t6, t5, t6\n\t"
        SDE(  t4, %2, 0) // swaps
        :
        : "r" (&data[j]), "r" (&data[j+1]), "r" (&swaps)
        : "t3", "t4", "t5", "t6" // clobbered registers
      );


    }
  }
}

int
main(void)
{
  if (configure_kmsm_from_dc() != 0)
    return -1;

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

  // initialize the array to sort
  for (unsigned i=0; i < DATASET_SIZE; i++)
  {
    raw_data[i] = libmin_rand();
    __asm__ volatile (
      // secret_data[i] = raw_data[i];
      "ld   t3, (%0)\n\t"
      SDE(  t3, %1, 0)
      :
      : "r" (&raw_data[i]), "r" (&secret_data[i])
      : "t3" // clobbered registers
    );
  }
  print_data(raw_data, DATASET_SIZE);

  {
    // performance monitoring
    // uint64_t icnt_start = __instret();

    bubblesort(secret_data, DATASET_SIZE);

    // uint64_t icnt_end = __instret();
    // libmin_printf("INFO: bubblesort inst count = %lu.\n", icnt_end - icnt_start + 1);
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
  libmin_printf("INFO: %lu swaps executed.\n", secret_decrypt(swaps));
  libmin_printf("INFO: data is properly sorted.\n");

  libmin_success();
  return 0;
}
