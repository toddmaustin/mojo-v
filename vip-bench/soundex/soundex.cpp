#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
using namespace std;

#include "utils.h"

// include build configuration defines
#include "../config.h"

vector<string> namesA =
  { "Johnson", "Adams", "Davis", "Simons", "Richards", "Taylor", "Carter", "Stevenson",
    "Taylor", "Smith", "McDonald", "Harris", "Sim", "Williams", "Baker", "Wells",
    "Fraser", "Jones", "Wilks", "Hunt", "Sanders", "Parsons", "Robson", "Harker"
  };
vector<string> namesB =
  { "Jonson", "Addams", "Davies", "Simmons", "Richardson", "Tailor", "Chater", "Stephenson",
    "Naylor", "Smythe", "MacDonald", "Harrys", "Sym", "Wilson", "Barker", "Wills",
    "Frazer", "Johns", "Wilkinson", "Hunter", "Saunders", "Pearson", "Robertson", "Parker"
  };

typedef VIP_ENCCHAR sndex_t[4];

void
string_to_soundex(string& name, sndex_t sndex)
{
  int si = 1;
  int c;

  //                 ABCDEFGHIJKLMNOPQRSTUVWXYZ
  char mappings[] = "01230120022455012623010202";

  sndex[0] = toupper(name[0]);

  for (unsigned i = 1, len = name.size(); i < len; i++)
  {
    c = (int)toupper(name[i]) - 65;

    if (c >= 0 && c <= 25)
    {
      if (mappings[c] != '0')
      {
        if (mappings[c] != VIP_DEC(sndex[si-1]))
        {
          sndex[si] = mappings[c];
          si++;
        }
        if (si > 3)
          break;
      }
    }
  }

  if (si <= 3)
  {
    while(si <= 3)
    {
      sndex[si] = '0';
      si++;
    }
  }
}

VIP_ENCBOOL
soundex_equal(sndex_t sndexA, sndex_t sndexB)
{
  VIP_ENCBOOL equiv = true;
  for (int i=0; i < 4; i++)
  {
#ifdef VIP_DO_MODE
    equiv = VIP_CMOV(sndexA[i] != sndexB[i], (VIP_ENCBOOL)false, equiv);
#else
    if (sndexA[i] != sndexB[i])
    {
      equiv = false;
      break;
    }
#endif
  }
  return equiv;
}

int
main(void)
{
  // initialize the privacy enhanced execution target
  VIP_INIT;

  unsigned trial;
  vector<sndex_t> sndexA(namesA.size());
  vector<sndex_t> sndexB(namesB.size());
  VIP_ENCBOOL results[sndexA.size()];

  // generate SoundEx string representations
  for (unsigned i = 0; i < sndexA.size(); i++)
  {
    string_to_soundex(namesA[i], sndexA[i]);
    string_to_soundex(namesB[i], sndexB[i]);
  }

  {
    Stopwatch s("VIP_Bench Runtime");

    // perform SoundEx string comparisons
    for (trial=0; trial < sndexA.size(); trial++)
      results[trial] = soundex_equal(sndexA[trial], sndexB[trial]);
   }

  // print the results
  fprintf(stderr, "INFO: %u trials performed.\n", trial);
  for (trial=0; trial < sndexA.size(); trial++)
  {
    fprintf(stdout, "trial %3u: %-20s[%c%c%c%c] =? %-20s[%c%c%c%c] --> %s\n",
            trial, namesA[trial].c_str(), VIP_DEC(sndexA[trial][0]), VIP_DEC(sndexA[trial][1]), VIP_DEC(sndexA[trial][2]), VIP_DEC(sndexA[trial][3]),
            namesB[trial].c_str(), VIP_DEC(sndexB[trial][0]), VIP_DEC(sndexB[trial][1]), VIP_DEC(sndexB[trial][2]), VIP_DEC(sndexB[trial][3]),
            VIP_DEC(results[trial]) ? "true" : "false");
  }
  return 0;
}

