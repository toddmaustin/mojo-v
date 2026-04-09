/*
 * calculate e=2.718..., using an interative approximation 
 */


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
using namespace std;

#include "../config.h"


int
main(void) 
{ 
  VIP_INIT; 

  double steps = 10000000; /* STEPS is usually a very large number eg 10000000 */
  VIP_ENCDOUBLE x, y;

  {
    Stopwatch s("VIP_Bench Runtime");
    y = (VIP_ENCDOUBLE)1.0 + (1.0/steps);
    x = (VIP_ENCDOUBLE)1.0;

    for(; steps > 0; steps--)
    {
      x = x * y;
    }
  }
  
  cout << "INFO: EulersNumberApprox(" << steps << ") == " << VIP_DEC(x)  << endl;
  return 0;

} 
