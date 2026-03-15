#include "libmin.h"
#include "mojov-utils.h"

#define CSR_MOJOV_CFG 0x0a0
#define CSR_MOJOV_CIPHERS 0x0a1
#define CSR_MOJOV_KMSM_ADDR 0x0a2
#define CSR_MOJOV_KMSM_DATA 0x0a3
#define CSR_MOJOV_KMSM_CTRL 0x0a4

#define KMSM_PUBKEY_BYTES 800
#define KMSM_ENCKEY_BYTES 768

static const char *DC_FAST_KEM_HEX =
  "aeef3d701933d8cf8c884251c632e35ed37e894de486e7a307fa30b5b9abc82995b19adbe4861688bd1f261084a4dad58a1eb3c686c4d1a172a6604bdb7c8054a6271bd77bfa340b46a66509e5c7bfc90e863cd05b2d5b2f86453fab7fbf24e05ced6b10485f5c85f902be1a9ef25986568abc7cdfb4eb56bc799f18afef7ba85645c078c08c643e05d3ca48b241a9261d8a05b3b65e2b4b9a95a8b1071934a54717ee41297fd5ed8b30ff3fc7371ddf061b4847b8be2c85a397ae2873fad50d10dd28ed440398a6768c63ea09a266c781e78aa386c2549791f69170f73a09e2e24b8834525457c35ff089bd7b4ae232d946c0361f5564aa5a47005c978fe371dacac0ed4fe1f5839e275960887fe26d9a4011f96d8b079e7578fad158bc89b4b748565f8be6c3f7d09cb6ef665008e5a6e2e7849a28bf512ec55f9f0ed0b7464b3b0bdf87b7c32ac265bbae6d3fad00f5bd4a9cfbf2494bf0cfc8f725dbc628b873e3018dcb3082df19ad2858a510cde5ccd72daade4a8bc2488a0673f7b47103062a5f7a9cf43943a58f8721be2b030ee4c93441ae81820bc11f1f310b4a2df60d067dbd64db492fbd3289d4846d55a2b4af9964871d3e4aa6d3e7c55d588e8ae5ac8bbda6105d1cd85d78771b71ae379a29e71cd37bf4049289b9de9a35fdde6b58bbbded7b8f354e89add01d460dc2e109071580d736d665446dfd0c3260163f39c39e23bcd9f6ce2a8cf407c42a4f6f525a6ef8174f26346078de920862db5944a7b093cc6b4eee98b168910ff66f981fb8b20d784d1c38f5dd9fabd2f63edd5696a52be0b868e992395f33abea191fa38776b55dc5551dea1b8f70bed25d70974189294017e6a5575c2ca540c905d2fa5b25dceaa7fc2911b3a613a96387074a75d80a062a59c906b8540f47dbd489e331d3bd70d0bf2b972da50ee37b74085315e7e936c02a41e939c25752b9c787185b0202102f170bc3bcee21b3e9259e2db87e2dfa4eb6c45e69ba9776a574d03eafc7cd214bb3d6c700997f471f1036e51cddb6c67b8708ca306d5d27bd9ab75f6724f6f918147947ffaf420d75";
static const char *DC_FAST_MSG_HEX =
  "bd6be9fa8b6c214805a6c52790cd8b31cd06529ac94c814ecd7f8cdd7efc90b316b15c4868075b5ec039d090c18f2a74650050e457150734db07f261aa1d8a12";

static const char *DC_STRONG_KEM_HEX =
  "74fc9ae5df568f0180512d9cc83ae7278a0ddaba61e5cc1c8dac1aef7a0d8cc7a6ba8046ced61f056fa8a043142b1b1db5c0b30e00c60b8c612d05d7add2c00350e198ec97a39e6463ea5e84bd4d1f2ce8101dcacb7c32efca36704e7f6ce2fb5a3c4d9c5fe9f54620c82da2c7b86742f37000ac7da57f304ebed051a049116f48f9fd2b77c4b8fdff3bb9c7561aac932e6cb45deaf2d1507c91f2e618baa5fd73a9c7832d9c92040a20072e54896f78d8ab545283f2e72ea4d7ba88d08ebbd3f75bcf0c5a25ed0032e6e31cde498add7be347b7973e6fa0346b94c5ebd499009512546818dcd4e33dac4850d2bf864d3204d439959058b574c9a5582389fc773df6dd003243eca011b5fd9aa22bc01740ffabe78756972272803b5cecc072228ed975816282235ebec98b6cf61c0bffdfcc86c1aa540945d6d5b4b256a94fd0bc3db8058bb71198158fa461a27dd185e31e9216271cd324ce556935d0a67437c79f967674e9a7d0edaa4c4ef79e02e11fe4d6909dd1e9e8b5c6306b454076321bae8406be421a7b45735d799298a957d8f691f5727469b8adb090306e2a0bff64e4823061c4f009048839c1033709216f83d6c383716c9bdc4db7793b37159473c743fc7ff3b0239dd8ea931352d2a1eca19037c2ab572d1ddd59a3fabd148ec725b5a5e7c053580f84de70f477d2deaa51d1f76ce4a7de5b832d6a2b46a524e46d0dd5acc826e9d5a0199bf017aafef1a85222ffc757a7b40419d407fc647cc853b3a124d2693b177a0cbf0d1c67947f85d17c7301d7712e97539fb2d9b80375906c1e62d0d52032a4b76eb4bcc5b2da4d608d5507b3095e2836c2ed8015252c9abda64b503619590fbf03a4ee7d09df12c6a67f83f5d87471aeba4390a9ab315a7adbb7dece290bda75617f58b8c56a624ead04589d0e337553832890dd15f21dad3043c4dacfd69d64dff424acf62b674fb7c509f57a30c74557332797259ffac186457f78620ae7b0c0b23f9287464c52551221dfadbc3139cb43f5dd0f03a0309468a8c230938af0f912cd90182d7de155eee68069b267637444a6929d";
static const char *DC_STRONG_MSG_HEX =
  "14a24eb5b6fed167bde6771bb2ce53f00fb83dc99c06bc1e25f6bd5162770cd1a5ca2bdda17f7a2a5ad23143372c2f77719410a1bc47423164a0834454054e30";

static const char *DC_PROOF_KEM_HEX =
  "120cb2eabbef7d1c5ee4ce1a2537fbe1dcd945196e4536479f8bc4cd0f7c6356dc8c12bf302cbe3dd5901774f670fae4813a8af3aee02d573781f061c55575eb2cc7faf2f6e33f9b28433eac8a14282abb34f5598d7b37c8e37e8daf845f6bb2ed387e0171bc66214aac4e4dd1bcac6871879e87a8c2f3322d4f3cefbb8b25185e5c6ad4becdbb21a95bfcaf11cc40e7f4d51ffca2b886d376f5371d7fcecdaa18cee80aaaa8480e17f692a7c04f74671f1cc9b310293e0fe7fa814a693a014220e649e970f0fbcf91756075cc92e7451fc69e2a6b901d2bdb2e82c4c4396aee185c82afff19c44172af110113322966e97aea5e684422e43ff1914b5d36ff8d117555c83789819e1b3b9fdd8da99f21c5da913798f4b70dc40af848cf9fc0df28784b7418bbefbf3f654a00ee10c991558209800bd80102b8bb285e3f4c152cc32b0afcc230e3a675f2dca7666393ba5408171c078791f4479bfbd9fd111f85f5b1864580635dca1b43cef8585a9298fc7d458433fd9c8421d9896f1f64cdf5de31b9d767f3004b1fc99044dd8da856ca33d31889579e472353fbc1ae1aad05266019ec1a77f9f8774271d58dc89e5d7fb21111cb86afb1cca2e853acf998deb35a6fc6d191170098ec9b3171abcd7ac421a2fc3371fae7cb8f9ed2018c9a039465cd34852c9e1aaa0c18a67dc9cf74dc8c3982ef3ce64666a6e75dd7c35e5a3ddfa88c0da7af2247a739d5e207041bc21a7d9e50ededd882f2f47883be9c73364ac9ca13bef61fb0bd5ded999b115d9b44826ec1d4b865a528a170e8225b44c2138f10a9f740bdd7d2841fad8dea75aa37e37581212df9eb92716726e536f710cfdd5d5d4bbcc84e00bf919ce86d1f10a524ef9ff9094f1c5ee27a4f51e749386a1ec02797849fecc79ea4f1a3249302ad719245f015e75194c37f79a681d621ef055beabc924bf83e25a1bff643ca556ff22520901392430b74e0acc8a317c66d60d90a027094900fb12923747f7bdfb71a7ec3b9f68084622f03781b3ff9aca8616fae7e85d570c527970e1a72ca2d9376d0d8ac6d3061d95aa958cd3f2a";
static const char *DC_PROOF_MSG_HEX =
  "50a07cce562c363cb9d662adc2b3a9e06280021478babc002ea627c41e8fd7ab33b4d6fb388910e3e5e87cc3c1f180056b4bbefa1f3217d17367892bfceb8a9c";

void mojov_print_mojov_cfg(uint64_t val)
{
  libmin_printf("(mojov_en:%s, key_valid:%s, format_sel:%s, mojov_ver:%u)",
                (val & 0x01) ? "t" : "f",
                (val & 0x02) ? "t" : "f",
                ((val >> 2) & 0x03) == 2 ? "proof-carrying" : ((((val >> 2) & 0x03) == 1) ? "strong" : "fast"),
                (val >> 4) & 0xff);
}

uint64_t mojov_read_mojov_cfg(void)
{
  uint64_t value;
  __asm__ volatile ("csrr %0, %1" : "=r"(value) : "i"(CSR_MOJOV_CFG));
  return value;
}

void mojov_write_mojov_cfg(uint64_t value)
{
  __asm__ volatile ("csrw %0, %1" :: "i"(CSR_MOJOV_CFG), "rK"(value));
}

uint64_t mojov_read_mojov_ciphers(void)
{
  uint64_t value;
  __asm__ volatile ("csrr %0, %1" : "=r"(value) : "i"(CSR_MOJOV_CIPHERS));
  return value;
}

void mojov_print_mprivregcfg(uint64_t val) { mojov_print_mojov_cfg(val); }
uint64_t mojov_read_mprivregcfg(void) { return mojov_read_mojov_cfg(); }
void mojov_write_mprivregcfg(uint64_t value) { mojov_write_mojov_cfg(value); }

int mojov_enable_and_verify(void)
{
  mojov_write_mojov_cfg(1);
  const uint64_t cfg = mojov_read_mojov_cfg();
  if ((cfg & 0x1) == 0) {
    libmin_printf("ERROR: failed to enable Mojo-V (mojov_en stayed 0).\n");
    mojov_print_mojov_cfg(cfg);
    libmin_printf("\n");
    return -1;
  }
  return 0;
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

static void load_hex_words_to_kmsm(const char *hex, uint64_t start_addr)
{
  uint64_t word = 0;
  unsigned byte_idx = 0;
  int hi_nibble = -1;

  write_kmsm_addr(start_addr);

  for (unsigned i = 0; hex[i] != '\0'; i++) {
    int hv = hex_nibble(hex[i]);
    if (hv < 0)
      continue;

    if (hi_nibble < 0) {
      hi_nibble = hv;
      continue;
    }

    uint8_t byte = (uint8_t)((hi_nibble << 4) | hv);
    word |= ((uint64_t)byte) << (byte_idx * 8);
    byte_idx++;
    hi_nibble = -1;

    if (byte_idx == 8) {
      write_kmsm_data(word);
      word = 0;
      byte_idx = 0;
    }
  }

  if (hi_nibble >= 0) {
    uint8_t byte = (uint8_t)(hi_nibble << 4);
    word |= ((uint64_t)byte) << (byte_idx * 8);
    byte_idx++;
  }

  if (byte_idx != 0)
    write_kmsm_data(word);
}

static int mojov_configure_kmsm_from_dc(const char *kem_hex, const char *msg_hex)
{
  load_hex_words_to_kmsm(kem_hex, KMSM_PUBKEY_BYTES);
  load_hex_words_to_kmsm(msg_hex, KMSM_PUBKEY_BYTES + KMSM_ENCKEY_BYTES);

  write_kmsm_ctrl(1);

  uint64_t ctrl = read_kmsm_ctrl();
  while ((ctrl & 0x2) != 0)
    ctrl = read_kmsm_ctrl();

  const uint64_t status = (ctrl >> 2) & 0x7;

  if (status == 0) {
    libmin_printf("INFO: KMSM contract open succeeded.\n");
    return 0;
  }

  libmin_printf("ERROR: KMSM contract open failed (kmsm_ctrl=0x%lx, status=%lu).\n", ctrl, status);
  return -1;
}

int mojov_configure_kmsm_from_dc_fast(void)
{
  return mojov_configure_kmsm_from_dc(DC_FAST_KEM_HEX, DC_FAST_MSG_HEX);
}

int mojov_configure_kmsm_from_dc_strong(void)
{
  return mojov_configure_kmsm_from_dc(DC_STRONG_KEM_HEX, DC_STRONG_MSG_HEX);
}

int mojov_configure_kmsm_from_dc_proof(void)
{
  return mojov_configure_kmsm_from_dc(DC_PROOF_KEM_HEX, DC_PROOF_MSG_HEX);
}
