#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <memory.h>
#include <vector>

using namespace std;

#include "../config.h"
#include "mathlib.h"
#include "kissdb.h"
#include "rapidcsv.h"

struct salesrec_t {
  VIP_ENCUINT zip;
  VIP_ENCUINT beds;
  VIP_ENCUINT baths;
  VIP_ENCUINT sqft;
  VIP_ENCDOUBLE price;
};

// key format is zip:cnt
#define KEYFMT(zip,cnt)   (((uint64_t)zip << 32) + (uint64_t)cnt)

void
print_salesrec(FILE *fd, struct salesrec_t *rec)
{
  fprintf(fd, "{ zip:%u, beds:%u, baths:%u, sqft:%u, price:$%.2lf }\n",
          VIP_DEC(rec->zip), VIP_DEC(rec->beds), VIP_DEC(rec->baths), VIP_DEC(rec->sqft), VIP_DEC(rec->price));
}

vector<unsigned> zipcodes =
{
  95603, 95608, 95610, 95614, 95619, 95621, 95623, 95624, 95626, 95628,
  95630, 95631, 95632, 95633, 95635, 95648, 95650, 95655, 95660, 95661,
  95662, 95663, 95667, 95670, 95673, 95677, 95678, 95682, 95683, 95690,
  95691, 95693, 95722, 95726, 95742, 95746, 95747, 95757, 95758, 95762,
  95765, 95811, 95814, 95815, 95816, 95817, 95818, 95819, 95820, 95821,
  95822, 95823, 95824, 95825, 95826, 95827, 95828, 95829, 95831, 95832,
  95833, 95834, 95835, 95838, 95841, 95842, 95843, 95864
};

enum op_t { op_avg, op_stddev, op_max, op_min };
enum field_t {field_price, field_pricesqft };
enum bedsize_t { bedsize_small, bedsize_large, bedsize_all };

VIP_ENCDOUBLE
filtered_query(KISSDB *pdb, unsigned zipcode, enum op_t op, enum field_t field, enum bedsize_t bedsize, VIP_ENCDOUBLE avg)
{
  struct salesrec_t rec;

  VIP_ENCDOUBLE sum = 0.0;
  VIP_ENCUINT count = 0;
  VIP_ENCDOUBLE max = -1.0;
  VIP_ENCDOUBLE min = 1000000000.0;
  for (unsigned j=0; ; j++)
  {
    uint64_t key = KEYFMT(zipcode, j);
    if (KISSDB_get(pdb, &key, &rec))
    {
      // no more entries in this zip code
      break;
    }
    // fprintf(stderr, "<zip,i> = <%u,%u>\n", zipcodes[i], j);

#ifdef VIP_DO_MODE
    // process this entry
    VIP_ENCBOOL _matched =
      (((VIP_ENCBOOL)(bedsize == bedsize_all))
       || ((VIP_ENCBOOL)(bedsize == bedsize_small) && rec.beds < 3)
       || ((VIP_ENCBOOL)(bedsize == bedsize_large) && rec.beds >= 3));
#else /* !VIP_DO_MODE */
    // process this entry
    if ((bedsize == bedsize_all)
        || (bedsize == bedsize_small && rec.beds < 3)
        || (bedsize == bedsize_large && rec.beds >= 3))
#endif /* !VIP_DO_MODE */
    {
      VIP_ENCDOUBLE val = 0.0;
      count++;
      if (field == field_price)
        val = rec.price;
      else if (field == field_pricesqft)
        val = (VIP_ENCDOUBLE)rec.price/(VIP_ENCDOUBLE)rec.sqft;

      if (op == op_avg)
      {
#ifdef VIP_DO_MODE
        sum = VIP_CMOV(_matched, sum+val, sum);
#else /* !VIP_DO_MODE */
        sum += val;
#endif /* !VIP_DO_MODE */
      }
      else if (op == op_stddev)
      {
#ifdef VIP_DO_MODE
        sum = VIP_CMOV(_matched, sum+((val - avg) * (val - avg)), sum);
#else /* !VIP_DO_MODE */
        sum += ((val - avg) * (val - avg));
#endif /* !VIP_DO_MODE */
      }
      else
      {
#ifdef VIP_DO_MODE
        max = VIP_CMOV(val > max && op == op_max, val, max);
        min = VIP_CMOV(val < min && op == op_min, val, min);
#else /* !VIP_DO_MODE */
        if (op == op_max && val > max)
          max = val;
        else if (op == op_min && val < min)
          min = val;
#endif /* !VIP_DO_MODE */
      }
    }
  }

  // compute the final statistic
  VIP_ENCDOUBLE retval = 0.0;
  if (op == op_avg)
  {
#ifdef VIP_DO_MODE
    retval = VIP_CMOV(count > 0, sum / (VIP_ENCDOUBLE)count, (VIP_ENCDOUBLE)0.0);
#else /* !VIP_DO_MODE */
    if (count > 0)
      retval = sum / (VIP_ENCDOUBLE)count;
    else
      retval = 0.0;
#endif /* !VIP_DO_MODE */
  }
  else if (op == op_stddev)
  {
#ifdef VIP_DO_MODE
    retval = VIP_CMOV(count > 1, mysqrt(sum / (VIP_ENCDOUBLE)(count-1)), (VIP_ENCDOUBLE)0.0);
#else /* !VIP_DO_MODE */
    if (count > 1)
      retval = mysqrt(sum / (VIP_ENCDOUBLE)(count-1));
    else
      retval = 0.0;
#endif /* !VIP_DO_MODE */
  }
  else if (op == op_max)
    retval = max;
  else if (op == op_min)
    retval = min;

  return retval;
}

void
filtered_query_byzip(KISSDB *pdb, enum op_t op, enum field_t field, enum bedsize_t bedsize, VIP_ENCDOUBLE avgs[], VIP_ENCDOUBLE v[])
{
  for (unsigned i=0; i < zipcodes.size(); i++)
  {
    v[i] = filtered_query(pdb, zipcodes[i], op, field, bedsize, avgs[i]);
  }
}

VIP_ENCDOUBLE
filtered_query_all(KISSDB *pdb, enum op_t op, enum field_t field, enum bedsize_t bedsize, double avg)
{
  uint64_t key;
  struct salesrec_t rec;
  KISSDB_Iterator dbi;

  KISSDB_Iterator_init(pdb, &dbi);

  VIP_ENCDOUBLE sum = 0.0;
  VIP_ENCUINT count = 0;
  VIP_ENCDOUBLE max = -1.0;
  VIP_ENCDOUBLE min = 1000000000.0;
  while (KISSDB_Iterator_next(&dbi, &key, &rec) > 0)
  {
    VIP_ENCDOUBLE val = 0.0;

#ifdef VIP_DO_MODE
    // process this entry
    VIP_ENCBOOL _matched =
      (((VIP_ENCBOOL)(bedsize == bedsize_all))
       || ((VIP_ENCBOOL)(bedsize == bedsize_small) && rec.beds < 3)
       || ((VIP_ENCBOOL)(bedsize == bedsize_large) && rec.beds >= 3));
#else /* !VIP_DO_MODE */
    // process this entry
    if ((bedsize == bedsize_all)
        || (bedsize == bedsize_small && rec.beds < 3)
        || (bedsize == bedsize_large && rec.beds >= 3))
#endif /* !VIP_DO_MODE */
    {
#ifdef VIP_DO_MODE
      count = VIP_CMOV(_matched, count+1, count);
#else /* !VIP_DO_MODE */
      count++;
#endif /* !VIP_DO_MODE */

      if (field == field_price)
        val = rec.price;
      else if (field == field_pricesqft)
        val = (VIP_ENCDOUBLE)rec.price/(VIP_ENCDOUBLE)rec.sqft;

      if (op == op_avg)
      {
#ifdef VIP_DO_MODE
        sum = VIP_CMOV(_matched, sum+val, sum);
#else /* !VIP_DO_MODE */
        sum += val;
#endif /* !VIP_DO_MODE */
      }
      else if (op == op_stddev)
      {
#ifdef VIP_DO_MODE
        sum = VIP_CMOV(_matched, sum+((val - avg) * (val - avg)), sum);
#else /* !VIP_DO_MODE */
        sum += ((val - avg) * (val - avg));
#endif /* !VIP_DO_MODE */
      }
      else
      {
#ifdef VIP_DO_MODE
        max = VIP_CMOV(val > max && op == op_max, val, max);
        min = VIP_CMOV(val < min && op == op_min, val, min);
#else /* !VIP_DO_MODE */
        if (op == op_max && val > max)
          max = val;
        else if (op == op_min && val < min)
          min = val;
#endif /* !VIP_DO_MODE */
      }
    }
  }

  // compute the final statistic
  VIP_ENCDOUBLE retval = 0.0;
  if (op == op_avg)
  {
#ifdef VIP_DO_MODE
    retval = VIP_CMOV(count > 0, sum / (VIP_ENCDOUBLE)count, (VIP_ENCDOUBLE)0.0);
#else /* !VIP_DO_MODE */
    if (count > 0)
      retval = sum / (VIP_ENCDOUBLE)count;
    else
      retval = 0.0;
#endif /* !VIP_DO_MODE */
  }
  else if (op == op_stddev)
  {
#ifdef VIP_DO_MODE
    retval = VIP_CMOV(count > 1, mysqrt(sum / (VIP_ENCDOUBLE)(count-1)), (VIP_ENCDOUBLE)0.0);
#else /* !VIP_DO_MODE */
    if (count > 1)
      retval = mysqrt(sum / (VIP_ENCDOUBLE)(count-1));
    else
      retval = 0.0;
#endif /* !VIP_DO_MODE */
  }
  else if (op == op_max)
    retval = max;
  else if (op == op_min)
    retval = min;

  return retval;
}

int
main(int argc,char **argv)
{
  KISSDB db;
  struct salesrec_t rec;

  printf("INFO: Generating homesales.db...\n");

  if (KISSDB_open(&db, "homesales.db", KISSDB_OPEN_MODE_RWREPLACE, 1024, sizeof(uint64_t), sizeof(rec)))
  {
    printf("KISSDB_open failed\n");
    return 1;
  }

  rapidcsv::Document doc("homesales.csv");
  fprintf(stdout, "INFO: data set size: %lu\n", doc.GetRowCount());

  unsigned currentZip = -1;
  unsigned zipCnt = 0;
  uint64_t key = 0;
  for (unsigned i=0; i < doc.GetRowCount(); i++)
  {
    unsigned lastZip;
    rec.zip = lastZip = doc.GetCell<unsigned>(0, i);
    rec.beds = doc.GetCell<unsigned>(1, i);
    rec.baths = doc.GetCell<unsigned>(2, i);
    rec.sqft = doc.GetCell<unsigned>(3, i);
    rec.price = doc.GetCell<double>(4, i);

    // compute the key val
    if (currentZip != lastZip)
    {
      currentZip = lastZip;
      zipCnt = 0;
    }
    key = KEYFMT(lastZip, zipCnt);
    zipCnt++;

    // print_salesrec(stderr, &rec);

    if (KISSDB_put(&db, &key, &rec)) {
      printf("KISSDB_put failed (%u)\n", i);
      exit(1);
    }
  }

  printf("INFO: Closing and re-opening database in read-only mode...\n");
  KISSDB_close(&db);

  if (KISSDB_open(&db,"homesales.db", KISSDB_OPEN_MODE_RDONLY, 1024, sizeof(uint64_t), sizeof(rec))) {
    printf("KISSDB_open failed\n");
    exit(1);
  }

  {
    Stopwatch s("VIP_Bench Runtime");

    // compute average/stddev price for zip code 95864 
    VIP_ENCDOUBLE avg = filtered_query(&db, 95864, op_avg, field_price, bedsize_all, 0.0);
    fprintf(stdout, "STAT: average price for zipcode 95864: $%.2lf\n", VIP_DEC(avg));

    VIP_ENCDOUBLE stddev = filtered_query(&db, 95864, op_stddev, field_price, bedsize_all, avg);
    fprintf(stdout, "STAT: stddev price for zipcode 95864: $%.2lf\n", VIP_DEC(stddev));

    // compute average/stddev price/sqft for zip code 95864 
    avg = filtered_query(&db, 95864, op_avg, field_pricesqft, bedsize_all, 0.0);
    fprintf(stdout, "STAT: average price/sqft for zipcode 95864: $%.2lf\n", VIP_DEC(avg));

    stddev = filtered_query(&db, 95864, op_stddev, field_pricesqft, bedsize_all, avg);
    fprintf(stdout, "STAT: stddev price/sqft for zipcode 95864: $%.2lf\n", VIP_DEC(stddev));

    // find the most expensive home price in the database
    VIP_ENCDOUBLE max_price = filtered_query_all(&db, op_max, field_price, bedsize_all, 0.0);
    fprintf(stdout, "STAT: price of most expensive home price: $%.2lf\n", VIP_DEC(max_price));

    // find the most expensive home price/sqft in the database
    VIP_ENCDOUBLE max_pricesqft = filtered_query_all(&db, op_max, field_pricesqft, bedsize_all, 0.0);
    fprintf(stdout, "STAT: price of most expensive home price/sqft: $%.2lf\n", VIP_DEC(max_pricesqft));

    // find the average price of small homes
    VIP_ENCDOUBLE avg_small = filtered_query_all(&db, op_avg, field_price, bedsize_small, 0.0);
    fprintf(stdout, "STAT: average price of small homes: $%.2lf\n", VIP_DEC(avg_small));

    // find the average price of large homes
    VIP_ENCDOUBLE avg_large = filtered_query_all(&db, op_avg, field_price, bedsize_large, 0.0);
    fprintf(stdout, "STAT: average price of large homes: $%.2lf\n", VIP_DEC(avg_large));

    // compute average/stddev price/sqft, by zip code
    VIP_ENCDOUBLE avgs[zipcodes.size()];
    VIP_ENCDOUBLE stddevs[zipcodes.size()];
    filtered_query_byzip(&db, op_avg, field_pricesqft, bedsize_all, avgs, avgs);
    filtered_query_byzip(&db, op_stddev, field_pricesqft, bedsize_all, avgs, stddevs);
    for (unsigned i=0; i < zipcodes.size(); i++)
    {
      fprintf(stdout, "STAT: Zipcode %u: average price/sqft $%.2lf, stddev price/sqft $%.2lf\n",
              zipcodes[i], VIP_DEC(avgs[i]), VIP_DEC(stddevs[i]));
    }
  }

  // shut down
  KISSDB_close(&db);

  return 0;
}

