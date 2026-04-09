/*
 * calculate e=2.718..., using an interative approximation 
 */


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <math.h>
using namespace std;

#include "../config.h"

#define PI         (4*atan(1)) //Not d-o because its pubic knowledge


VIP_ENCDOUBLE 
rad2deg(VIP_ENCDOUBLE rad)
{
      return ((VIP_ENCDOUBLE)180.0 * rad / (PI));
}

VIP_ENCDOUBLE
deg2rad(VIP_ENCDOUBLE deg)
{
      return ((VIP_ENCDOUBLE)PI * deg / 180.0);
}


int
main(void) 
{ 
  VIP_INIT; 
  cout << fixed;
  cout << setprecision(5);

{ Stopwatch s("VIP_Bench Runtime");
  for (double x = 0.0; x <= 360.0; x += 1.0)
  {
    cout << "INFO: deg2rad(" << x << ") == " << VIP_DEC(deg2rad((VIP_ENCDOUBLE)x))  << endl;
  }

  for (double x = 0.0; x <= (2 * PI + 1e-6); x += (PI / 180)){
    cout << "INFO: rad2deg(" << x << ") == " << VIP_DEC(rad2deg((VIP_ENCDOUBLE)x))  << endl;
  }
}

  return 0;
} 
