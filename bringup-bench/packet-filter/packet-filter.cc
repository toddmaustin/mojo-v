#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"

#define TCP_PROTOCOL 6u
#define FIXED_IP_FIRST 192u
#define FIXED_IP_SECOND 168u
#define PACKET_COUNT 100

struct Packet
{
  uint32e_t src_ip;
  uint32e_t dest_ip;
  uint16e_t src_port;
  uint16e_t dest_port;
  uint16e_t protocol;
  uint16e_t payload[64];
};

static Packet
generate_packet(void)
{
  Packet pkt;

  if (libmin_rand() % 20 == 0)
  {
    uint32_t third = (uint32_t)(libmin_rand() % 256);
    uint32_t fourth = (uint32_t)(libmin_rand() % 256);
    uint32_t fixed_ip = (FIXED_IP_FIRST << 24) | (FIXED_IP_SECOND << 16) | (third << 8) | fourth;

    pkt.dest_ip = fixed_ip;
    pkt.protocol = TCP_PROTOCOL;
  }
  else
  {
    pkt.dest_ip = (uint32_t)libmin_rand();
    pkt.protocol = (uint16_t)(libmin_rand() % 256);
  }

  pkt.src_ip = (uint32_t)libmin_rand();
  pkt.src_port = (uint16_t)(libmin_rand() % 65536);
  pkt.dest_port = (uint16_t)(libmin_rand() % 65536);

  for (int i = 0; i < 63; i++)
    pkt.payload[i] = (uint16_t)('A' + (libmin_rand() % 26));

  pkt.payload[63] = 0;

  return pkt;
}

static uint64e_t
check_packet_filter(const Packet &pkt)
{
  uint32e_t first_octet = (pkt.dest_ip >> 24) & uint32e_t(0xFF);
  uint32e_t second_octet = (pkt.dest_ip >> 16) & uint32e_t(0xFF);

  uint64e_t proto_ok = (uint64e_t)(pkt.protocol == (uint16_t)TCP_PROTOCOL);
  uint64e_t first_ok = (uint64e_t)(first_octet == FIXED_IP_FIRST);
  uint64e_t second_ok = (second_octet == FIXED_IP_SECOND);

  return proto_ok & first_ok & second_ok;
}

static void
print_ip(uint32_t ip)
{
  libmin_printf("%u.%u.%u.%u",
    (ip >> 24) & 0xFF,
    (ip >> 16) & 0xFF,
    (ip >> 8) & 0xFF,
    ip & 0xFF);
}

static void
print_packet(const Packet &pkt)
{
  char payload[64];
  for (int i = 0; i < 64; i++)
    payload[i] = (char)pkt.payload[i].decrypt();

  libmin_printf("Packet Details:\n");
  libmin_printf(" Source IP: ");
  print_ip((uint32_t)pkt.src_ip.decrypt());
  libmin_printf("\n Destination IP: ");
  print_ip((uint32_t)pkt.dest_ip.decrypt());
  libmin_printf("\n Source Port: %u\n", (unsigned)pkt.src_port.decrypt());
  libmin_printf(" Destination Port: %u\n", (unsigned)pkt.dest_port.decrypt());
  libmin_printf(" Protocol: %u\n", (unsigned)pkt.protocol.decrypt());
  libmin_printf(" Payload: %s\n", payload);
  libmin_printf("------------------------------\n");
}

int
main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;

  if (mojov_enable_and_verify() != 0)
    return -1;

  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  libmin_srand(42);

  for (int packet_counter = 1; packet_counter <= PACKET_COUNT; packet_counter++)
  {
    Packet pkt = generate_packet();
    if (check_packet_filter(pkt).decrypt())
    {
      libmin_printf("Matched Packet #%d:\n", packet_counter);
      print_packet(pkt);
    }
  }

  libmin_success();
  return 0;
}
