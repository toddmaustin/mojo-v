// See LICENSE for license details.

#include "config.h"
#include "cfg.h"
#include "sim.h"
#include "mmu.h"
#include "arith.h"
#include "remote_bitbang.h"
#include "cachesim.h"
#include "extension.h"
#include <dlfcn.h>
#include <fesvr/option_parser.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <limits>
#include <cinttypes>
#include <sstream>

// Mojo-V: crypto includes
#include <openssl/provider.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "../riscv/mojov_crypto.h"
#include "../VERSION"

static void help(int exit_code = 1)
{
  fprintf(stderr, "Spike RISC-V ISA Simulator " SPIKE_VERSION "\n\n");
  fprintf(stderr, "usage: spike [host options] <target program> [target options]\n");
  fprintf(stderr, "Host Options:\n");
  fprintf(stderr, "  -p<n>                 Simulate <n> processors [default 1]\n");
  fprintf(stderr, "  -m<n>                 Provide <n> MiB of target memory [default 2048]\n");
  fprintf(stderr, "  -m<a:m,b:n,...>       Provide memory regions of size m and n bytes\n");
  fprintf(stderr, "                          at base addresses a and b (with 4 KiB alignment)\n");
  fprintf(stderr, "  -d                    Interactive debug mode\n");
  fprintf(stderr, "  -g                    Track histogram of PCs\n");
  fprintf(stderr, "  -l                    Generate a log of execution\n");
#ifdef HAVE_BOOST_ASIO
  fprintf(stderr, "  -s                    Command I/O via socket (use with -d)\n");
#endif
  fprintf(stderr, "  -h, --help            Print this help message\n");
  fprintf(stderr, "  --halted              Start halted, allowing a debugger to connect\n");
  fprintf(stderr, "  --log=<name>          File name for option -l\n");
  fprintf(stderr, "  --debug-cmd=<name>    Read commands from file (use with -d)\n");
  fprintf(stderr, "  --isa=<name>          RISC-V ISA string [default %s]\n", DEFAULT_ISA);
  fprintf(stderr, "  --pmpregions=<n>      Number of PMP regions [default 16]\n");
  fprintf(stderr, "  --pmpgranularity=<n>  PMP Granularity in bytes [default 4]\n");
  fprintf(stderr, "  --priv=<m|mu|msu>     RISC-V privilege modes supported [default %s]\n", DEFAULT_PRIV);
  fprintf(stderr, "  --pc=<address>        Override ELF entry point\n");
  fprintf(stderr, "  --hartids=<a,b,...>   Explicitly specify hartids, default is 0,1,...\n");
  fprintf(stderr, "  --ic=<S>:<W>:<B>      Instantiate a cache model with S sets,\n");
  fprintf(stderr, "  --dc=<S>:<W>:<B>        W ways, and B-byte blocks (with S and\n");
  fprintf(stderr, "  --l2=<S>:<W>:<B>        B both powers of 2).\n");
  fprintf(stderr, "  --big-endian          Use a big-endian memory system.\n");
  fprintf(stderr, "  --misaligned          Support misaligned memory accesses\n");
  fprintf(stderr, "  --device=<name>       Attach MMIO plugin device from an --extlib library,\n");
  fprintf(stderr, "                          specify --device=<name>,<args> to pass down extra args.\n");
  fprintf(stderr, "  --log-cache-miss      Generate a log of cache miss\n");
  fprintf(stderr, "  --log-commits         Generate a log of commits info\n");
  fprintf(stderr, "  --extension=<name>    Specify RoCC Extension\n");
  fprintf(stderr, "                          This flag can be used multiple times.\n");
  fprintf(stderr, "  --extlib=<name>       Shared library to load\n");
  fprintf(stderr, "                        This flag can be used multiple times.\n");
  fprintf(stderr, "  --rbb-port=<port>     Listen on <port> for remote bitbang connection\n");
  fprintf(stderr, "  --dump-dts            Print device tree string and exit\n");
  fprintf(stderr, "  --dtb=<path>          Use specified device tree blob [default: auto-generate]\n");
  fprintf(stderr, "  --disable-dtb         Don't write the device tree blob into memory\n");
  fprintf(stderr, "  --kernel=<path>       Load kernel flat image into memory\n");
  fprintf(stderr, "  --initrd=<path>       Load kernel initrd into memory\n");
  fprintf(stderr, "  --bootargs=<args>     Provide custom bootargs for kernel [default: %s]\n",
          DEFAULT_KERNEL_BOOTARGS);
  fprintf(stderr, "  --real-time-clint     Increment clint time at real-time rate\n");
  fprintf(stderr, "  --triggers=<n>        Number of supported triggers [default 4]\n");
  fprintf(stderr, "  --dm-progsize=<words> Progsize for the debug module [default 2]\n");
  fprintf(stderr, "  --dm-sba=<bits>       Debug system bus access supports up to "
      "<bits> wide accesses [default 0]\n");
  fprintf(stderr, "  --dm-auth             Debug module requires debugger to authenticate\n");
  fprintf(stderr, "  --dmi-rti=<n>         Number of Run-Test/Idle cycles "
      "required for a DMI access [default 0]\n");
  fprintf(stderr, "  --dm-abstract-rti=<n> Number of Run-Test/Idle cycles "
      "required for an abstract command to execute [default 0]\n");
  fprintf(stderr, "  --dm-no-hasel         Debug module won't support hasel\n");
  fprintf(stderr, "  --dm-no-abstract-csr  Debug module won't support abstract CSR access\n");
  fprintf(stderr, "  --dm-no-abstract-fpr  Debug module won't support abstract FPR access\n");
  fprintf(stderr, "  --dm-no-halt-groups   Debug module won't support halt groups\n");
  fprintf(stderr, "  --dm-no-impebreak     Debug module won't support implicit ebreak in program buffer\n");
  fprintf(stderr, "  --blocksz=<size>      Cache block size (B) for CMO operations(powers of 2) [default 64]\n");
  fprintf(stderr, "  --instructions=<n>    Stop after n instructions\n");
  fprintf(stderr, "  --mojov-verbose       Mojo-V setup processing is verbose\n");
  fprintf(stderr, "  --mojov-fast          Use Mojo-V fast encryption mode (default mode)\n");
  fprintf(stderr, "  --mojov-strong        Use Mojo-V strong encryption format (otherwise using data contract specified mode)\n");
  fprintf(stderr, "  --mojov-proofcarrying Use Mojo-V proof-carrying encryption format (otherwise use data contract specified mode)\n");
  fprintf(stderr, "  --mojov-arg=<n>       Pass a numeric argument to a Mojo-V test code\n");
  fprintf(stderr, "  --mojov-sk=<pem_file> Load Mojo-V CPU secret key from <pem_file>\n");
  fprintf(stderr, "  --mojov-pk=<pem_file> Load Mojo-V CPU public key from <pem_file>\n");
  fprintf(stderr, "  --mojov-dc=<dc_file>  Load Mojo-V data contract from <dc_file>\n");

  exit(exit_code);
}

static void suggest_help()
{
  fprintf(stderr, "Try 'spike --help' for more information.\n");
  exit(1);
}

static bool check_file_exists(const char *fileName)
{
  std::ifstream infile(fileName);
  return infile.good();
}

static std::ifstream::pos_type get_file_size(const char *filename)
{
  std::ifstream in(filename, std::ios::ate | std::ios::binary);
  return in.tellg();
}

static void read_file_bytes(const char *filename,size_t fileoff,
                            abstract_mem_t* mem, size_t memoff, size_t read_sz)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  in.seekg(fileoff, std::ios::beg);

  std::vector<char> read_buf(read_sz, 0);
  in.read(&read_buf[0], read_sz);
  mem->store(memoff, read_sz, (uint8_t*)&read_buf[0]);
}

bool sort_mem_region(const mem_cfg_t &a, const mem_cfg_t &b)
{
  if (a.get_base() == b.get_base())
    return (a.get_size() < b.get_size());
  else
    return (a.get_base() < b.get_base());
}

static bool check_mem_overlap(const mem_cfg_t& L, const mem_cfg_t& R)
{
  return std::max(L.get_base(), R.get_base()) <= std::min(L.get_inclusive_end(), R.get_inclusive_end());
}

static bool check_if_merge_covers_64bit_space(const mem_cfg_t& L,
                                              const mem_cfg_t& R)
{
  if (!check_mem_overlap(L, R))
    return false;

  auto start = std::min(L.get_base(), R.get_base());
  auto end = std::max(L.get_inclusive_end(), R.get_inclusive_end());

  return (start == 0ull) && (end == std::numeric_limits<uint64_t>::max());
}

static mem_cfg_t merge_mem_regions(const mem_cfg_t& L, const mem_cfg_t& R)
{
  // one can merge only intersecting regions
  assert(check_mem_overlap(L, R));

  const auto merged_base = std::min(L.get_base(), R.get_base());
  const auto merged_end_incl = std::max(L.get_inclusive_end(), R.get_inclusive_end());
  const auto merged_size = merged_end_incl - merged_base + 1;

  return mem_cfg_t(merged_base, merged_size);
}

// check the user specified memory regions and merge the overlapping or
// eliminate the containing parts
static std::vector<mem_cfg_t>
merge_overlapping_memory_regions(std::vector<mem_cfg_t> mems)
{
  if (mems.empty())
    return {};

  std::sort(mems.begin(), mems.end(), sort_mem_region);

  std::vector<mem_cfg_t> merged_mem;
  merged_mem.push_back(mems.front());

  for (auto mem_it = std::next(mems.begin()); mem_it != mems.end(); ++mem_it) {
    const auto& mem_int = *mem_it;
    if (!check_mem_overlap(merged_mem.back(), mem_int)) {
      merged_mem.push_back(mem_int);
      continue;
    }
    // there is a weird corner case preventing two memory regions from being
    // merged: if the resulting size of a region is 2^64 bytes - currently,
    // such regions are not representable by mem_cfg_t class (because the
    // actual size field is effectively a 64 bit value)
    // so we create two smaller memory regions that total for 2^64 bytes as
    // a workaround
    if (check_if_merge_covers_64bit_space(merged_mem.back(), mem_int)) {
      merged_mem.clear();
      merged_mem.push_back(mem_cfg_t(0ull, 0ull - PGSIZE));
      merged_mem.push_back(mem_cfg_t(0ull - PGSIZE, PGSIZE));
      break;
    }
    merged_mem.back() = merge_mem_regions(merged_mem.back(), mem_int);
  }

  return merged_mem;
}

static mem_cfg_t create_mem_region(unsigned long long base, unsigned long long size)
{
  // page-align base and size
  auto base0 = base, size0 = size;
  size += base0 % PGSIZE;
  base -= base0 % PGSIZE;
  if (size % PGSIZE != 0)
    size += PGSIZE - size % PGSIZE;

  if (size != size0) {
    fprintf(stderr, "Warning: the memory at [0x%llX, 0x%llX] has been realigned\n"
                    "to the %ld KiB page size: [0x%llX, 0x%llX]\n",
            base0, base0 + size0 - 1, long(PGSIZE / 1024), base, base + size - 1);
  }

  if (!mem_cfg_t::check_if_supported(base, size)) {
    fprintf(stderr, "Unsupported memory region "
                    "{base = 0x%llX, size = 0x%llX} specified\n",
            base, size);
    exit(EXIT_FAILURE);
  }

  return mem_cfg_t(base, size);
}

static std::vector<mem_cfg_t> parse_mem_layout(const char* arg)
{
  std::vector<mem_cfg_t> res;

  // handle legacy mem argument
  char* p;
  auto mb = strtoull(arg, &p, 0);
  if (*p == 0) {
    reg_t size = reg_t(mb) << 20;
    if ((size >> 20) != mb)
      throw std::runtime_error("Memory size too large");
    res.push_back(create_mem_region(DRAM_BASE, size));
    return res;
  }

  // handle base/size tuples
  while (true) {
    auto base = strtoull(arg, &p, 0);
    if (!*p || *p != ':')
      help();
    auto size = strtoull(p + 1, &p, 0);

    res.push_back(create_mem_region(base, size));

    if (!*p)
      break;
    if (*p != ',')
      help();
    arg = p + 1;
  }

  auto merged_mem = merge_overlapping_memory_regions(res);

  assert(!merged_mem.empty());
  return merged_mem;
}

static std::vector<std::pair<reg_t, abstract_mem_t*>> make_mems(const std::vector<mem_cfg_t> &layout)
{
  std::vector<std::pair<reg_t, abstract_mem_t*>> mems;
  mems.reserve(layout.size());
  for (const auto &cfg : layout) {
    mems.push_back(std::make_pair(cfg.get_base(), new mem_t(cfg.get_size())));
  }
  return mems;
}

static unsigned long atoul_safe(const char* s)
{
  char* e;
  auto res = strtoul(s, &e, 10);
  if (*e)
    help();
  return res;
}

static unsigned long atoul_nonzero_safe(const char* s)
{
  auto res = atoul_safe(s);
  if (!res)
    help();
  return res;
}

static std::vector<size_t> parse_hartids(const char *s)
{
  std::string const str(s);
  std::stringstream stream(str);
  std::vector<size_t> hartids;

  int n;
  while (stream >> n) {
    if (n < 0) {
      fprintf(stderr, "Negative hart ID %d is unsupported\n", n);
      exit(-1);
    }

    hartids.push_back(n);
    if (stream.peek() == ',') stream.ignore();
  }

  if (hartids.empty()) {
    fprintf(stderr, "No hart IDs specified\n");
    exit(-1);
  }

  std::sort(hartids.begin(), hartids.end());

  const auto dup = std::adjacent_find(hartids.begin(), hartids.end());
  if (dup != hartids.end()) {
    fprintf(stderr, "Duplicate hart ID %zu\n", *dup);
    exit(-1);
  }

  return hartids;
}

// Mojo-V: helper routines
#define GCM_IV_LEN     12
#define GCM_TAG_LEN    16
#define DC_WIRE_LEN 64

/* ---- Utility / error ---- */

static EVP_PKEY *read_pubkey_pem(const char *path) {
    BIO *bio = BIO_new_file(path, "rb");
    if (!bio) return NULL;
    EVP_PKEY *pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return pkey;
}

/* ---- Read whole file into NUL-terminated buffer ---- */
static char *read_all_text(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }

    char *buf = (char *)OPENSSL_malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t got = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (got != (size_t)sz) { OPENSSL_free(buf); return NULL; }
    buf[sz] = '\0';
    return buf;
}

/* Find "KEY=" line and return pointer to value */
static const char *find_kv(const char *text, const char *key) {
    size_t klen = strlen(key);
    const char *p = text;
    while (*p) {
        const char *line = p;
        const char *eol = strchr(line, '\n');
        size_t linelen = eol ? (size_t)(eol - line) : strlen(line);

        if (linelen > klen + 1 &&
            strncmp(line, key, klen) == 0 &&
            line[klen] == '=') {
            return line + klen + 1; /* start of value */
        }
        if (!eol) break;
        p = eol + 1;
    }
    return NULL;
}

/* ---- Hex helpers ---- */
static int hex_to_nybble(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + (c - 'a');
    if ('A' <= c && c <= 'F') return 10 + (c - 'A');
    return -1;
}

/* FIXED: only parse up to end-of-line, not entire file */
static int hex_to_bytes(const char *hex, unsigned char **out, size_t *outlen) {
    /* Only parse up to end-of-line (or NUL) */
    size_t n = 0;
    while (hex[n] != '\0' && hex[n] != '\n' && hex[n] != '\r')
        n++;

    /* Trim trailing spaces/tabs */
    while (n > 0 && (hex[n-1] == ' ' || hex[n-1] == '\t'))
        n--;
    
    if (n == 0) return 0;  
    if (n % 2 != 0) return 0;

    size_t blen = n / 2;
    unsigned char *buf = (unsigned char *)OPENSSL_malloc(blen);
    if (!buf) return 0;
    
    for (size_t i = 0; i < blen; i++) {
        int hi = hex_to_nybble(hex[2*i]);
        int lo = hex_to_nybble(hex[2*i+1]);
        if (hi < 0 || lo < 0) {
            OPENSSL_free(buf);
            return 0;
        }
        buf[i] = (unsigned char)((hi << 4) | lo);
    }

    *out = buf;
    *outlen = blen;
    return 1;
}   

int main(int argc, char** argv)
{
  bool debug = false;
  bool halted = false;
  bool histogram = false;
  bool log = false;
  bool UNUSED socket = false;  // command line option -s
  bool dump_dts = false;
  bool dtb_enabled = true;
  const char* kernel = NULL;
  reg_t kernel_offset, kernel_size;
  std::vector<device_factory_sargs_t> plugin_device_factories;
  std::unique_ptr<icache_sim_t> ic;
  std::unique_ptr<dcache_sim_t> dc;
  std::unique_ptr<cache_sim_t> l2;
  bool log_cache = false;
  bool log_commits = false;
  const char *log_path = nullptr;
  std::vector<std::function<extension_t*()>> extensions;
  const char* initrd = NULL;
  const char* dtb_file = NULL;
  uint16_t rbb_port = 0;
  bool use_rbb = false;
  unsigned dmi_rti = 0;
  reg_t blocksz = 64;
  std::optional<unsigned long long> instructions;
  debug_module_config_t dm_config;

  // Mojo-V: crypto parameters
  const char *mojov_sk = NULL;
  const char *mojov_pk = NULL;
  const char *mojov_dc = NULL;

  cfg_arg_t<size_t> nprocs(1);

  cfg_t cfg;

  auto const device_parser = [&plugin_device_factories](const char *s) {
    const std::string device_args(s);
    std::vector<std::string> parsed_args;
    std::stringstream sstr(device_args);
    while (sstr.good()) {
      std::string substr;
      getline(sstr, substr, ',');
      parsed_args.push_back(substr);
    }
    if (parsed_args.empty()) throw std::runtime_error("Plugin argument is empty.");

    const std::string name = parsed_args[0];
    if (name.empty()) throw std::runtime_error("Plugin name is empty.");

    auto it = mmio_device_map().find(name);
    if (it == mmio_device_map().end()) throw std::runtime_error("Plugin \"" + name + "\" not found in loaded extlibs.");

    parsed_args.erase(parsed_args.begin());
    plugin_device_factories.push_back(std::make_pair(it->second, parsed_args));
  };

  option_parser_t parser;
  parser.help(&suggest_help);
  parser.option('h', "help", 0, [&](const char UNUSED *s){help(0);});
  parser.option('d', 0, 0, [&](const char UNUSED *s){debug = true;});
  parser.option('g', 0, 0, [&](const char UNUSED *s){histogram = true;});
  parser.option('l', 0, 0, [&](const char UNUSED *s){log = true;});
#ifdef HAVE_BOOST_ASIO
  parser.option('s', 0, 0, [&](const char UNUSED *s){socket = true;});
#endif
  parser.option('p', 0, 1, [&](const char* s){nprocs = atoul_nonzero_safe(s);});
  parser.option('m', 0, 1, [&](const char* s){cfg.mem_layout = parse_mem_layout(s);});
  parser.option(0, "halted", 0, [&](const char UNUSED *s){halted = true;});
  parser.option(0, "rbb-port", 1, [&](const char* s){use_rbb = true; rbb_port = atoul_safe(s);});
  parser.option(0, "pc", 1, [&](const char* s){cfg.start_pc = strtoull(s, 0, 0);});
  parser.option(0, "hartids", 1, [&](const char* s){
    cfg.hartids = parse_hartids(s);
    cfg.explicit_hartids = true;
  });
  parser.option(0, "ic", 1, [&](const char* s){ic.reset(new icache_sim_t(s));});
  parser.option(0, "dc", 1, [&](const char* s){dc.reset(new dcache_sim_t(s));});
  parser.option(0, "l2", 1, [&](const char* s){l2.reset(cache_sim_t::construct(s, "L2$"));});
  parser.option(0, "big-endian", 0, [&](const char UNUSED *s){cfg.endianness = endianness_big;});
  parser.option(0, "misaligned", 0, [&](const char UNUSED *s){cfg.misaligned = true;});
  parser.option(0, "log-cache-miss", 0, [&](const char UNUSED *s){log_cache = true;});
  parser.option(0, "isa", 1, [&](const char* s){cfg.isa = s;});
  parser.option(0, "pmpregions", 1, [&](const char* s){cfg.pmpregions = atoul_safe(s);});
  parser.option(0, "pmpgranularity", 1, [&](const char* s){cfg.pmpgranularity = atoul_safe(s);});
  parser.option(0, "priv", 1, [&](const char* s){cfg.priv = s;});
  parser.option(0, "device", 1, device_parser);
  parser.option(0, "extension", 1, [&](const char* s){extensions.push_back(find_extension(s));});
  parser.option(0, "dump-dts", 0, [&](const char UNUSED *s){dump_dts = true;});
  parser.option(0, "disable-dtb", 0, [&](const char UNUSED *s){dtb_enabled = false;});
  parser.option(0, "dtb", 1, [&](const char *s){dtb_file = s;});
  parser.option(0, "kernel", 1, [&](const char* s){kernel = s;});
  parser.option(0, "initrd", 1, [&](const char* s){initrd = s;});
  parser.option(0, "bootargs", 1, [&](const char* s){cfg.bootargs = s;});
  parser.option(0, "real-time-clint", 0, [&](const char UNUSED *s){cfg.real_time_clint = true;});
  parser.option(0, "triggers", 1, [&](const char *s){cfg.trigger_count = atoul_safe(s);});
  parser.option(0, "extlib", 1, [&](const char *s){
    void *lib = dlopen(s, RTLD_NOW | RTLD_GLOBAL);
    if (lib == NULL) {
      fprintf(stderr, "Unable to load extlib '%s': %s\n", s, dlerror());
      exit(-1);
    }
  });
  parser.option(0, "dm-progsize", 1,
      [&](const char* s){dm_config.progbufsize = atoul_safe(s);});
  parser.option(0, "dm-no-impebreak", 0,
      [&](const char UNUSED *s){dm_config.support_impebreak = false;});
  parser.option(0, "dm-sba", 1,
      [&](const char* s){dm_config.max_sba_data_width = atoul_safe(s);});
  parser.option(0, "dm-auth", 0,
      [&](const char UNUSED *s){dm_config.require_authentication = true;});
  parser.option(0, "dmi-rti", 1,
      [&](const char* s){dmi_rti = atoul_safe(s);});
  parser.option(0, "dm-abstract-rti", 1,
      [&](const char* s){dm_config.abstract_rti = atoul_safe(s);});
  parser.option(0, "dm-no-hasel", 0,
      [&](const char UNUSED *s){dm_config.support_hasel = false;});
  parser.option(0, "dm-no-abstract-csr", 0,
      [&](const char UNUSED *s){dm_config.support_abstract_csr_access = false;});
  parser.option(0, "dm-no-abstract-fpr", 0,
      [&](const char UNUSED *s){dm_config.support_abstract_fpr_access = false;});
  parser.option(0, "dm-no-halt-groups", 0,
      [&](const char UNUSED *s){dm_config.support_haltgroups = false;});
  parser.option(0, "log-commits", 0,
                [&](const char UNUSED *s){log_commits = true;});
  parser.option(0, "log", 1,
                [&](const char* s){log_path = s;});
  FILE *cmd_file = NULL;
  parser.option(0, "debug-cmd", 1, [&](const char* s){
     if ((cmd_file = fopen(s, "r"))==NULL) {
        fprintf(stderr, "Unable to open command file '%s'\n", s);
        exit(-1);
     }
  });
  parser.option(0, "blocksz", 1, [&](const char* s){
    blocksz = strtoull(s, 0, 0);
    const unsigned min_blocksz = 16;
    const unsigned max_blocksz = PGSIZE;
    if (blocksz < min_blocksz || blocksz > max_blocksz || ((blocksz & (blocksz - 1))) != 0) {
      fprintf(stderr, "--blocksz must be a power of 2 between %u and %u\n",
        min_blocksz, max_blocksz);
      exit(-1);
    }
    cfg.cache_blocksz = blocksz;
  });
  parser.option(0, "instructions", 1, [&](const char* s){
    instructions = strtoull(s, 0, 0);
  });
  parser.option(0, "mojov-verbose", 0, [&](const char UNUSED *s){cfg.mojov_verbose = true;});
  parser.option(0, "mojov-fast", 0, [&](const char UNUSED *s){cfg.mojov_fast = true;});
  parser.option(0, "mojov-strong", 0, [&](const char UNUSED *s){cfg.mojov_strong = true;});
  parser.option(0, "mojov-proofcarrying", 0, [&](const char UNUSED *s){cfg.mojov_proofcarrying = true;});
  parser.option(0, "mojov-arg", 1, [&](const char *s){
    uint64_t nval = atoul_safe(s);
    if (nval >= (1 << 16))
    {
      fprintf(stderr, "--mojov-arg must be < 2^16\n");
      exit(-1);
    }
    cfg.mojov_arg = nval;
  });

  // Mojo-V: crypto parameters
  parser.option(0, "mojov-sk", 1, [&](const char* s){mojov_sk = s;});
  parser.option(0, "mojov-pk", 1, [&](const char* s){mojov_pk = s;});
  parser.option(0, "mojov-dc", 1, [&](const char* s){mojov_dc = s;});

  auto argv1 = parser.parse(argv);
  std::vector<std::string> htif_args(argv1, (const char*const*)argv + argc);

  if (!*argv1)
    help();

  std::vector<std::pair<reg_t, abstract_mem_t*>> mems =
      make_mems(cfg.mem_layout);

  if ((mojov_sk && !mojov_pk) || (!mojov_sk && mojov_pk))
  {
    fprintf(stderr, "Mojo-V: --mojov-sk and --mojov-pk must be specified together\n");
    exit(-1);
  }

  if (mojov_dc && (!mojov_sk || !mojov_pk))
  {
    fprintf(stderr, "Mojo-V: --mojov-dc requires both --mojov-sk and --mojov-pk\n");
    exit(-1);
  }

  if (mojov_pk)
  {
    OSSL_PROVIDER *prov = OSSL_PROVIDER_load(NULL, "default");
    if (!prov) {
      fprintf(stderr, "Mojo-V: Unable to OSSL_PROVIDER_load(default)\n");
      exit(-1);
    }

    EVP_PKEY *pk = read_pubkey_pem(mojov_pk);
    if (!pk) {
      fprintf(stderr, "Mojo-V: Unable to read public key (PK) PEM file\n");
      exit(-1);
    }

    int der_len = i2d_PUBKEY(pk, NULL);
    if (der_len <= 0) {
      fprintf(stderr, "Mojo-V: Unable to encode public key to DER\n");
      exit(-1);
    }

    cfg.mojov_pk_der.resize((size_t)der_len);
    unsigned char *der_ptr = cfg.mojov_pk_der.data();
    if (i2d_PUBKEY(pk, &der_ptr) != der_len) {
      fprintf(stderr, "Mojo-V: Unable to write public key DER bytes\n");
      exit(-1);
    }

    EVP_PKEY_free(pk);
    OSSL_PROVIDER_unload(prov);
  }

  // Mojo-V: crypto parameters
  if (mojov_sk)
    cfg.mojov_sk_pem_path = mojov_sk;

  if (mojov_sk || mojov_dc)
  {
    // need both parameters
    if (!mojov_sk || !mojov_dc)
    {
      fprintf(stderr, "Mojo-V: Both --mojov-sk and --mojov-dc are required to configure Mojo-V PKI engine\n");
      exit(-1);
    }

    // load the data contract (DC) file
    char *text = read_all_text(mojov_dc);
    if (!text) {
      fprintf(stderr, "Mojo-V: unable to read data contract (DC) file\n");
      exit(-1);
    }

    // parse the encrypted data contract file fields
    const char *kem_dc_hex = find_kv(text, "KEM_DC");
    const char *msg_dc_hex = find_kv(text, "MSG_DC");
    const char *hsh_hex    = find_kv(text, "PT_SHA256");

    if (!kem_dc_hex || !msg_dc_hex || !hsh_hex) {
      fprintf(stderr, "Mojo-V: missing fields in data contract (DC) file\n");
      exit(-1);
    }

    unsigned char *kem_dc = NULL, *msg_dc = NULL, *hsh_b = NULL;
    size_t kem_dc_len = 0, msg_dc_len = 0, hsh_len = 0;

    if (hex_to_bytes(kem_dc_hex, &kem_dc, &kem_dc_len) != 1) {
      fprintf(stderr, "Mojo-V: parse KEM_DC hex failed\n");
      exit(-1);
    }
    if (hex_to_bytes(msg_dc_hex, &msg_dc, &msg_dc_len) != 1) {
      fprintf(stderr, "Mojo-V: parse MSG_DC hex failed\n");
      exit(-1);
    }
    if (hex_to_bytes(hsh_hex, &hsh_b, &hsh_len) != 1) {
      fprintf(stderr, "Mojo-V: parse PT_SHA256 hex failed\n");
      exit(-1);
    }

    if (msg_dc_len != DC_WIRE_LEN || hsh_len != 32) {
      fprintf(stderr, "Mojo-V: invalid field lengths in data contract (DC) file\n");
      exit(-1);
    }

    unsigned char pt_sha[32];
    mojov_open_status_t open_status = mojov_open_status_t::OK;
    if (!mojov_open_contract_from_components(mojov_sk, kem_dc, kem_dc_len,
                                             msg_dc, msg_dc_len,
                                             hsh_b,
                                             &cfg.mojov_dc,
                                             pt_sha,
                                             &open_status)) {
      switch (open_status) {
        case mojov_open_status_t::MISSING_SK_PATH:
        case mojov_open_status_t::SK_LOAD:
          fprintf(stderr, "Mojo-V: Unable to read secret key (SK) PEM file\n");
          exit(-1);
        case mojov_open_status_t::OSSL_PROVIDER_LOAD:
          fprintf(stderr, "Mojo-V: Unable to OSSL_PROVIDER_load(default)\n");
          exit(-1);
        case mojov_open_status_t::DECAP_CTX:
          fprintf(stderr, "Mojo-V: failed to initialize context, EVP_PKEY_CTX_new_from_pkey(decap)\n");
          exit(-1);
        case mojov_open_status_t::DECAP_INIT:
          fprintf(stderr, "Mojo-V: failed to initialize decapsulation, EVP_PKEY_decapsulate_init\n");
          exit(-1);
        case mojov_open_status_t::DECAP_SIZE:
          fprintf(stderr, "Mojo-V: ciphertext decapsulation size query failed, EVP_PKEY_decapsulate()\n");
          exit(-1);
        case mojov_open_status_t::SS_MALLOC:
          fprintf(stderr, "Mojo-V: SS malloc failed\n");
          exit(-1);
        case mojov_open_status_t::DECAP:
          fprintf(stderr, "Mojo-V: decapsulation failed, EVP_PKEY_decapsulate()\n");
          exit(-1);
        case mojov_open_status_t::HKDF:
          fprintf(stderr, "Mojo-V: HKDF key derivation failed\n");
          exit(-1);
        case mojov_open_status_t::SHA256:
          fprintf(stderr, "Mojo-V: SHA256(pt) failed\n");
          exit(-1);
        case mojov_open_status_t::PT_HASH_MISMATCH:
          fprintf(stderr, "Mojo-V: ciphertext authentication FAILED (PT_SHA256 mismatch)\n");
          exit(-1);
        case mojov_open_status_t::BAD_SIGNATURE:
          fprintf(stderr, "Mojo-V: invalid data contract signature header\n");
          exit(-1);
        default:
          fprintf(stderr, "Mojo-V: open contract failed\n");
          exit(-1);
      }
    }

    // perform simon key expansion on the decrypted SIMON128 key
    // simon_128_128_keyexpand(&simon_state, *((uint128_t *)cfg.mojov_dc.sym_key_128), 68);
    // printf("INFO: simon_key = 0x"); for (int i = 0; i < 16; i++) printf("%02x", cfg.mojov_dc.sym_key_128[i]); printf("\n");

    // override the data contract (DC) memory mode, if the command line specified one
    if (cfg.mojov_fast)
      cfg.mojov_dc.format_sel = 0;
    if (cfg.mojov_strong)
      cfg.mojov_dc.format_sel = 1;
    if (cfg.mojov_proofcarrying)
      cfg.mojov_dc.format_sel = 2;

    // install the valid Mojo-V data contract in the simulator configuration
    cfg.mojov_dcvalid = true;

    // sync up the memory mode config options
    if      (cfg.mojov_dc.format_sel == 0) cfg.mojov_fast = true;
    else if (cfg.mojov_dc.format_sel == 1) cfg.mojov_strong = true;
    else if (cfg.mojov_dc.format_sel == 2) cfg.mojov_proofcarrying = true;
    else
    {
      fprintf(stderr, "ERROR: invalid Mojo-V memory mode.\n");
      abort();
    }

    OPENSSL_free(kem_dc);
    OPENSSL_free(msg_dc);
    OPENSSL_free(hsh_b);
    OPENSSL_free(text);

  }
  else
    cfg.mojov_dcvalid = false;

  if (kernel && check_file_exists(kernel)) {
    const char *isa = cfg.isa;
    kernel_size = get_file_size(kernel);
    if (isa[2] == '6' && isa[3] == '4')
      kernel_offset = 0x200000;
    else
      kernel_offset = 0x400000;
    for (auto& m : mems) {
      if (kernel_size && (kernel_offset + kernel_size) < m.second->size()) {
         read_file_bytes(kernel, 0, m.second, kernel_offset, kernel_size);
         break;
      }
    }
  }

  if (initrd && check_file_exists(initrd)) {
    size_t initrd_size = get_file_size(initrd);
    for (auto& m : mems) {
      if (initrd_size && (initrd_size + 0x1000) < m.second->size()) {
         reg_t initrd_end = m.first + m.second->size() - 0x1000;
         reg_t initrd_start = initrd_end - initrd_size;
         cfg.initrd_bounds = std::make_pair(initrd_start, initrd_end);
         read_file_bytes(initrd, 0, m.second, initrd_start - m.first, initrd_size);
         break;
      }
    }
  }

  if (cfg.explicit_hartids) {
    if (nprocs.overridden() && (nprocs() != cfg.nprocs())) {
      std::cerr << "Number of specified hartids ("
                << cfg.nprocs()
                << ") doesn't match specified number of processors ("
                << nprocs() << ").\n";
      exit(1);
    }
  } else {
    // Set default set of hartids based on nprocs, but don't set the
    // explicit_hartids flag (which means that downstream code can know that
    // we've only set the number of harts, not explicitly chosen their IDs).
    std::vector<size_t> default_hartids;
    default_hartids.reserve(nprocs());
    for (size_t i = 0; i < nprocs(); ++i) {
      default_hartids.push_back(i);
    }
    cfg.hartids = default_hartids;
  }

  sim_t s(&cfg, halted,
      mems, plugin_device_factories, htif_args, dm_config, log_path, dtb_enabled, dtb_file,
      socket,
      cmd_file,
      instructions);
  std::unique_ptr<remote_bitbang_t> remote_bitbang((remote_bitbang_t *) NULL);
  std::unique_ptr<jtag_dtm_t> jtag_dtm(
      new jtag_dtm_t(&s.debug_module, dmi_rti));
  if (use_rbb) {
    remote_bitbang.reset(new remote_bitbang_t(rbb_port, &(*jtag_dtm)));
    s.set_remote_bitbang(&(*remote_bitbang));
  }

  if (dump_dts) {
    printf("%s", s.get_dts());
    return 0;
  }

  if (ic && l2) ic->set_miss_handler(&*l2);
  if (dc && l2) dc->set_miss_handler(&*l2);
  if (ic) ic->set_log(log_cache);
  if (dc) dc->set_log(log_cache);
  for (size_t i = 0; i < cfg.nprocs(); i++)
  {
    if (ic) s.get_core(i)->get_mmu()->register_memtracer(&*ic);
    if (dc) s.get_core(i)->get_mmu()->register_memtracer(&*dc);
    for (auto e : extensions)
      s.get_core(i)->register_extension(e());
  }

  s.set_debug(debug);
  s.configure_log(log, log_commits);
  s.set_histogram(histogram);

  auto return_code = s.run();

  for (auto& mem : mems)
    delete mem.second;

  return return_code;
}
