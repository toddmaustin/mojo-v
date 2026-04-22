#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#include "mojov-exo.h"
#include "mojov-string.h"

using namespace exo;

static constexpr unsigned kStringCap = 32;
static constexpr unsigned kPatternCap = 8;

static const char *kEntries[] = {
  "Abomination", "Abusive Sergeant", "Acidic Swamp Ooze", "Acidmaw", "Acolyte of Pain",
  "Al'Akir the Windlord", "Alarm-o-Bot", "Aldor Peacekeeper", "Alexstrasza", "Alexstrasza's Champion",
  "Amani Berserker", "Ancestor's Call", "Ancestral Healing", "Ancestral Knowledge", "Ancestral Spirit",
  "Ancient Brewmaster", "Ancient Mage", "Ancient of Lore", "Ancient of War", "Ancient Shade",
  "Ancient Watcher", "Angry Chicken", "Anima Golem", "Animal Companion", "Animated Armor",
  "Annoy-o-Tron", "Anodized Robo Cub", "Antique Healbot", "Anub'ar Ambusher", "Anub'arak",
  "Anubisath Sentinel", "Anyfin Can Happen", "Arathi Weaponsmith", "Arcane Blast", "Arcane Explosion",
  "Arcane Golem", "Arcane Intellect", "Arcane Missiles", "Arcane Nullifier X-21", "Arcane Shot",
  "Arcanite Reaper", "Arch-Thief Rafaam", "Archmage", "Archmage Antonidas", "Argent Commander",
  "Argent Horserider", "Argent Lance", "Argent Protector", "Argent Squire", "Argent Watchman",
  "Armored Warhorse", "Armorsmith", "Assassin's Blade", "Assassinate", "Astral Communion",
  "Auchenai Soulpriest", "Avenge", "Avenging Wrath", "Aviana", "Axe Flinger", "Azure Drake",
  "Backstab", "Ball of Spiders", "Bane of Doom", "Baron Geddon", "Baron Rivendare", "Bash",
  "Battle Rage", "Bear Trap", "Beneath the Grounds", "Bestial Wrath", "Betrayal", "Big Game Hunter",
  "Bite", "Blackwing Corruptor", "Blackwing Technician", "Blade Flurry", "Blessed Champion",
  "Blessing of Kings", "Blessing of Might", "Blessing of Wisdom", "Blingtron 3000", "Blizzard",
  "Blood Imp", "Blood Knight", "Bloodfen Raptor", "Bloodlust", "Bloodmage Thalnos", "Bloodsail Corsair",
  "Bloodsail Raider", "Bluegill Warrior", "Bolf Ramshield", "Bolster", "Bolvar Fordragon", "Bomb Lobber",
  "Boneguard Lieutenant", "Booty Bay Bodyguard", "Boulderfist Ogre", "Bouncing Blade", "Brann Bronzebeard",
  "Brave Archer", "Brawl", "Buccaneer", "Burgle", "Burly Rockjaw Trogg", "Cabal Shadow Priest",
  "Cairne Bloodhoof", "Call Pet", "Captain Greenskin", "Captain's Parrot", "Captured Jormungar", "Cenarius",
  "Charge", "Charged Hammer", "Chillmaw", "Chillwind Yeti", "Chromaggus", "Circle of Healing", "Claw",
  "Cleave", "Clockwork Giant", "Clockwork Gnome", "Clockwork Knight", "Cobalt Guardian", "Cobra Shot",
  "Coghammer", "Cogmaster", "Cogmaster's Wrench", "Cold Blood", "Coldarra Drake", "Coldlight Oracle",
  "Coldlight Seer", "Coliseum Manager", "Commanding Shout", "Competitive Spirit", "Conceal", "Cone of Cold",
  "Confessor Paletress", "Confuse", "Consecration", "Convert", "Core Hound", "Core Rager", "Corruption",
  "Counterspell", "Crackle", "Crazed Alchemist", "Crowd Favorite", "Cruel Taskmaster", "Crush", "Cult Master",
  "Curse of Rafaam", "Cursed Blade", "Cutpurse", nullptr
};

static const char *kPatterns[] = {"core", "work", "sam"};

static uint8e_t secret_tolower(uint8e_t ch)
{
  uint64e_t is_upper = (uint64e_t)(ch >= uint8e_t('A')) && (uint64e_t)(ch <= uint8e_t('Z'));
  return cmov(is_upper, uint8e_t(ch + uint8e_t(32)), ch);
}

static stringe_t encrypt_fixed_string(const char *s, unsigned cap)
{
  stringe_t out(cap);
  for (unsigned i = 0; s[i] != '\0'; ++i)
    out.push_back(uint8e_t((uint8_t)s[i]));
  return out;
}

static uint8e_t secret_char_at(const stringe_t &s, uint64e_t idx)
{
  uint8e_t out(0);
  for (unsigned j = 0; j < s.length(); ++j)
  {
    uint64e_t choose = (idx == uint64e_t(j)) && (uint64e_t(j) < s.size());
    out = cmov(choose, s[j], out);
  }
  return out;
}

static uint64e_t fuzzy_subsequence_match(const stringe_t &pattern, const stringe_t &text)
{
  uint64e_t pi(0);
  for (unsigned i = 0; i < text.length(); ++i)
  {
    uint64e_t text_in = uint64e_t(i) < text.size();
    uint64e_t pat_in = pi < pattern.size();
    uint8e_t tc = secret_tolower(text[i]);
    uint8e_t pc = secret_tolower(secret_char_at(pattern, pi));
    uint64e_t match = text_in && pat_in && (uint64e_t)(tc == pc);
    pi = cmov(match, pi + uint64e_t(1), pi);
  }
  return pi == pattern.size();
}

static void compute_match_bit_vector(
  const stringe_t &pattern,
  stringe_t *entries,
  unsigned n_entries,
  uint64e_t *bitvec,
  unsigned bitvec_words)
{
  for (unsigned w = 0; w < bitvec_words; ++w)
    bitvec[w] = uint64e_t(0);

  for (unsigned i = 0; i < n_entries; ++i)
  {
    uint64e_t is_match = fuzzy_subsequence_match(pattern, entries[i]);
    unsigned w = i / 64;
    unsigned b = i % 64;
    uint64e_t mask = uint64e_t(1ULL << b);
    bitvec[w] = bitvec[w] | cmov(is_match, mask, uint64e_t(0));
  }
}

static bool decrypt_match_bit(const uint64e_t *bitvec, unsigned idx)
{
  unsigned w = idx / 64;
  unsigned b = idx % 64;
  uint64_t plain = bitvec[w].decrypt();
  return (plain & (1ULL << b)) != 0;
}

int main(void)
{
  if (mojov_configure_kmsm_from_dc_fast() != 0)
    return -1;
  if (mojov_enable_and_verify() != 0)
    return -1;
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0)
    return -1;

  unsigned n_entries = 0;
  while (kEntries[n_entries] != nullptr)
    ++n_entries;

  stringe_t entries_enc[256];
  for (unsigned i = 0; i < n_entries; ++i)
    entries_enc[i] = encrypt_fixed_string(kEntries[i], kStringCap);

  static constexpr unsigned kPatternCount = sizeof(kPatterns) / sizeof(kPatterns[0]);
  static constexpr unsigned kBitvecWords = (256 + 63) / 64;
  uint64e_t bitvec[kBitvecWords];

  for (unsigned p = 0; p < kPatternCount; ++p)
  {
    stringe_t pattern_enc = encrypt_fixed_string(kPatterns[p], kPatternCap);

    compute_match_bit_vector(pattern_enc, entries_enc, n_entries, bitvec, kBitvecWords);

    libmin_printf("Matches for `%s':\n", kPatterns[p]);
    for (unsigned i = 0; i < n_entries; ++i)
      if (decrypt_match_bit(bitvec, i))
        libmin_printf(" %s\n", kEntries[i]);
    libmin_printf("\n");
  }

  libmin_success();
  return 0;
}
