#include "libmin.h"
#include "simon.h"
#include "mojov-utils.h"

#include "dc-fast.h"

typedef mojov_mem_fast_u64_t _uint64e_t;
typedef mojov_mem_fast_fp64_t _fp64e_t;
#include "mojov-exo.h"

#define PATIENTS 10

typedef struct {
  uint64_t age;
  uint64_t bmi;
  uint64_t systolic_bp;
  uint64_t smoker;
  uint64_t diabetes;
  uint64_t family_history;
} patient_features_t;

typedef struct {
  uint64e_t age;
  uint64e_t bmi;
  uint64e_t systolic_bp;
  uint64e_t smoker;
  uint64e_t diabetes;
  uint64e_t family_history;
} patient_secret_t;

static const patient_features_t patient_data[PATIENTS] = {
  {23, 22, 110, 0, 0, 0},
  {35, 27, 126, 0, 0, 1},
  {42, 30, 135, 1, 0, 1},
  {51, 34, 142, 1, 1, 1},
  {28, 31, 118, 0, 0, 0},
  {65, 29, 155, 0, 1, 1},
  {58, 41, 165, 1, 1, 1},
  {47, 26, 130, 0, 0, 0},
  {72, 33, 172, 1, 1, 1},
  {39, 24, 124, 0, 0, 0}
};

static patient_secret_t encrypted_patients[PATIENTS];

static uint64e_t
compute_secret_risk_score(const patient_secret_t *patient)
{
  uint64e_t age_points = patient->age * 2;
  uint64e_t bmi_points = patient->bmi;
  uint64e_t bp_points = patient->systolic_bp / 2;
  uint64e_t smoker_points = patient->smoker * 30;
  uint64e_t diabetes_points = patient->diabetes * 25;
  uint64e_t family_points = patient->family_history * 15;

  return age_points + bmi_points + bp_points + smoker_points + diabetes_points + family_points;
}

static uint64e_t
compute_secret_risk_tier(uint64e_t score)
{
  uint64e_t high = (score >= 180);
  uint64e_t moderate = (score >= 120) && (score < 180);
  return cmov(high, 2, cmov(moderate, 1, 0));
}

static uint64_t
compute_plain_risk_score(const patient_features_t *patient)
{
  return (patient->age * 2)
      + patient->bmi
      + (patient->systolic_bp / 2)
      + (patient->smoker * 30)
      + (patient->diabetes * 25)
      + (patient->family_history * 15);
}

static uint64_t
compute_plain_risk_tier(uint64_t score)
{
  if (score >= 180)
    return 2;
  if (score >= 120)
    return 1;
  return 0;
}

static const char *
tier_label(uint64_t tier)
{
  if (tier == 2)
    return "HIGH";
  if (tier == 1)
    return "MODERATE";
  return "LOW";
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

  for (unsigned i = 0; i < PATIENTS; ++i)
  {
    encrypted_patients[i].age = patient_data[i].age;
    encrypted_patients[i].bmi = patient_data[i].bmi;
    encrypted_patients[i].systolic_bp = patient_data[i].systolic_bp;
    encrypted_patients[i].smoker = patient_data[i].smoker;
    encrypted_patients[i].diabetes = patient_data[i].diabetes;
    encrypted_patients[i].family_history = patient_data[i].family_history;
  }

  for (unsigned i = 0; i < PATIENTS; ++i)
  {
    uint64e_t secret_score = compute_secret_risk_score(&encrypted_patients[i]);
    uint64e_t secret_tier = compute_secret_risk_tier(secret_score);

    uint64_t revealed_score = secret_score.decrypt();
    uint64_t revealed_tier = secret_tier.decrypt();

    uint64_t expected_score = compute_plain_risk_score(&patient_data[i]);
    uint64_t expected_tier = compute_plain_risk_tier(expected_score);

    libmin_printf("risk-score patient=%u score=%lu tier=%s\n",
      i, revealed_score, tier_label(revealed_tier));

    if (revealed_score != expected_score || revealed_tier != expected_tier)
    {
      libmin_printf("ERROR: mismatch for patient %u\n", i);
      return -1;
    }
  }

  libmin_printf("risk-score: encrypted scoring completed for %u patients\n", PATIENTS);

  libmin_success();
  return 0;
}
