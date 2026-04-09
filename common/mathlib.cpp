

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../config.h"

VIP_ENCDOUBLE
myfloor(VIP_ENCDOUBLE x)
{
  VIP_ENCLONG i = (VIP_ENCLONG)x;
#ifdef VIP_DO_MODE
  VIP_ENCDOUBLE _retval = 0.0;
  VIP_ENCBOOL _pred = (i > x);
  _retval = VIP_CMOV(_pred, (VIP_ENCDOUBLE)(i-1), (VIP_ENCDOUBLE)i);
 
  return _retval;
#else /* !VIP_DO_MODE */
  return i - (i > x);
#endif /* VIP_DO_MODE */
}

VIP_ENCDOUBLE
myround(VIP_ENCDOUBLE x)
{
  VIP_ENCDOUBLE t = 0.0;

#ifdef VIP_DO_MODE
  VIP_ENCBOOL _pred = (x >= 0);
  t = VIP_CMOV(_pred, myfloor(x), t);
  VIP_ENCBOOL _pred1 = ((t - x) <= -0.5);
  t = VIP_CMOV(_pred && _pred1, t + 1, t);

  t = VIP_CMOV(!_pred, myfloor(-x), t);
  VIP_ENCBOOL _pred2 = ((t + x) <= -0.5);
  t = VIP_CMOV(!_pred && _pred2, t + 1, t);
  t = VIP_CMOV(!_pred, -t, t);
#else /* !VIP_DO_MODE */
  if (x >= 0)
  {
    t = myfloor(x);
    if ((t - x) <= -0.5)
      t += 1;
  }
  else /* x < 0 */
  {
    t = myfloor(-x);
    if ((t + x) <= -0.5)
      t += 1;
    t = -t;
  }
#endif /* !VIP_DO_MODE */

  return t;
}

VIP_ENCDOUBLE
myfabs(VIP_ENCDOUBLE x)
{
#ifdef VIP_DO_MODE
  VIP_ENCDOUBLE _retval = 0.0;
  VIP_ENCBOOL _pred = (x < 0.0);
  _retval = VIP_CMOV(_pred, -x, x);

  return _retval;
#else /* !VIP_DO_MODE */
  if (x < 0.0)
    return -x;
  else
    return x;
#endif /* VIP_DO_MODE */
}

VIP_ENCDOUBLE
mypow(VIP_ENCDOUBLE x, unsigned exp)
{
#ifdef VIP_DO_MODE
  VIP_ENCDOUBLE _retval = 0.0;

  VIP_ENCBOOL _pred1 = (exp == 0);
  _retval = VIP_CMOV(_pred1, (VIP_ENCDOUBLE)1.0L, _retval);

  _retval = VIP_CMOV(!_pred1, x, _retval);
  for (unsigned i=1; i < exp; i++)
    _retval = VIP_CMOV(!_pred1, _retval * x, _retval);

  return _retval;  
#else /* !VIP_DO_MODE */
  if (exp == 0)
    return 1.0L;

  double retval = x;
  for (unsigned i=1; i < exp; i++)
    retval *= x;
  return retval;
#endif /* VIP_DO_MODE */
}


VIP_ENCDOUBLE
invsqrt(VIP_ENCDOUBLE num)
{
  VIP_ENCDOUBLE y = num;
  VIP_ENCDOUBLE x2 = y * 0.5;
  VIP_ENCLONG i = *(VIP_ENCLONG *) &y;

  // The magic number is for doubles is from https://cs.uwaterloo.ca/~m32rober/rsqrt.pdf
  i = (VIP_ENCLONG)0x5fe6eb50c7b537a9 - (i >> 1);
  y = *(VIP_ENCDOUBLE *) &i;
  y = y * ((VIP_ENCDOUBLE)1.5 - (x2 * y * y));   // 1st iteration
  y  = y * ((VIP_ENCDOUBLE)1.5 - ( x2 * y * y ) );   // 2nd iteration, this can be removed
  return y;
}

VIP_ENCDOUBLE
mysqrt(VIP_ENCDOUBLE num)
{
  return (VIP_ENCDOUBLE)1.0 / invsqrt(num);
}

void
sincos(VIP_ENCDOUBLE *psin, VIP_ENCDOUBLE *pcos, VIP_ENCDOUBLE x)
{
  VIP_ENCDOUBLE sin = *psin, cos = *pcos;

#ifdef VIP_DO_MODE
  //always wrap input angle to -PI..PI
  VIP_ENCBOOL _pred1 = (x < -3.14159265);
  VIP_ENCBOOL _pred2 = (x >  3.14159265);
  x = VIP_CMOV(_pred1, x + 6.28318531, x);
  x = VIP_CMOV(!_pred1 && _pred2, x - 6.28318531, x);

  //compute sine
  VIP_ENCBOOL _pred3 = (x < 0);

  sin = VIP_CMOV(_pred3, (VIP_ENCDOUBLE)1.27323954 * x + (VIP_ENCDOUBLE).405284735 * x * x, x);
  VIP_ENCBOOL _pred4 = (sin < 0);
  sin = VIP_CMOV(_pred3 && _pred4, (VIP_ENCDOUBLE)0.225 * (sin *-sin - sin) + sin, sin);
  sin = VIP_CMOV(_pred3 && !_pred4, (VIP_ENCDOUBLE)0.225 * (sin * sin - sin) + sin, sin);

  sin = VIP_CMOV(!_pred3, (VIP_ENCDOUBLE)1.27323954 * x - (VIP_ENCDOUBLE)0.405284735 * x * x, sin);
  VIP_ENCBOOL _pred5 = (sin < 0);
  sin = VIP_CMOV(!_pred3 && _pred5, (VIP_ENCDOUBLE)0.225 * (sin *-sin - sin) + sin, sin);
  sin = VIP_CMOV(!_pred3 && !_pred5, (VIP_ENCDOUBLE)0.225 * (sin * sin - sin) + sin, sin);

  //compute cosine: sin(x + PI/2) = cos(x)
  x += 1.57079632;

  VIP_ENCBOOL _pred6 = (x >  3.14159265);
  x = VIP_CMOV(_pred6, x - 6.28318531, x);

  VIP_ENCBOOL _pred7 = (x < 0);

  cos = VIP_CMOV(_pred7, (VIP_ENCDOUBLE)1.27323954 * x + (VIP_ENCDOUBLE)0.405284735 * x * x, cos);
  VIP_ENCBOOL _pred8 = (cos < 0);
  cos = VIP_CMOV(_pred7 && _pred8, (VIP_ENCDOUBLE)0.225 * (cos *-cos - cos) + cos, cos);
  cos = VIP_CMOV(_pred7 && !_pred8, (VIP_ENCDOUBLE)0.225 * (cos * cos - cos) + cos, cos);

  cos = VIP_CMOV(!_pred7, (VIP_ENCDOUBLE)1.27323954 * x - (VIP_ENCDOUBLE)0.405284735 * x * x, cos);
  VIP_ENCBOOL _pred9 = (cos < 0);
  cos = VIP_CMOV(!_pred7 && _pred9, (VIP_ENCDOUBLE)0.225 * (cos *-cos - cos) + cos, cos);
  cos = VIP_CMOV(!_pred7 && !_pred9, (VIP_ENCDOUBLE)0.225 * (cos * cos - cos) + cos, cos);
#else /* !VIP_DO_MODE */
  //always wrap input angle to -PI..PI
  if (x < -3.14159265) /* _pred1: ... */
    x += 6.28318531;
  else if (x >  3.14159265) /* _pred2: ... */
    x -= 6.28318531;

  //compute sine
  if (x < 0) /* _pred3: ... */
  {
    sin = 1.27323954 * x + .405284735 * x * x;

    if (sin < 0) /* _pred4: ... */
      sin = .225 * (sin *-sin - sin) + sin;
    else
      sin = .225 * (sin * sin - sin) + sin;
  }
  else
  {
    sin = 1.27323954 * x - 0.405284735 * x * x;

    if (sin < 0) /* _pred5: ... */
      sin = .225 * (sin *-sin - sin) + sin;
    else
     sin = .225 * (sin * sin - sin) + sin;
  }

  //compute cosine: sin(x + PI/2) = cos(x)
  x += 1.57079632;
  if (x >  3.14159265) /* _pred6: ... */
    x -= 6.28318531;

  if (x < 0) /* _pred7: ... */
  {
    cos = 1.27323954 * x + 0.405284735 * x * x;

    if (cos < 0) /* _pred8: ... */
      cos = .225 * (cos *-cos - cos) + cos;
    else
      cos = .225 * (cos * cos - cos) + cos;
  }
  else
  {
    cos = 1.27323954 * x - 0.405284735 * x * x;

    if (cos < 0) /* _pred9: ... */
      cos = .225 * (cos *-cos - cos) + cos;
    else
      cos = .225 * (cos * cos - cos) + cos;
  }
#endif /* VIP_DO_MODE */

  *psin = sin;
  *pcos = cos;
}

VIP_ENCDOUBLE
mysin(VIP_ENCDOUBLE x)
{
  VIP_ENCDOUBLE sin, cos;

  sincos(&sin, &cos, x);
  return sin;
}

VIP_ENCDOUBLE
mycos(VIP_ENCDOUBLE x)
{
  VIP_ENCDOUBLE sin, cos;

  sincos(&sin, &cos, x);
  return cos;
}

static const double
one = 1.0,
halF[2] = {0.5,-0.5,},
huge  = 1.0e+300,
o_threshold=  7.09782712893383973096e+02,  /* 0x40862E42, 0xFEFA39EF */
u_threshold= -7.45133219101941108420e+02,  /* 0xc0874910, 0xD52D3051 */
ln2HI[2]   ={ 6.93147180369123816490e-01,  /* 0x3fe62e42, 0xfee00000 */
       -6.93147180369123816490e-01,},/* 0xbfe62e42, 0xfee00000 */
ln2LO[2]   ={ 1.90821492927058770002e-10,  /* 0x3dea39ef, 0x35793c76 */
       -1.90821492927058770002e-10,},/* 0xbdea39ef, 0x35793c76 */
invln2 =  1.44269504088896338700e+00, /* 0x3ff71547, 0x652b82fe */
P1   =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
P2   = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
P3   =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
P4   = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
P5   =  4.13813679705723846039e-08; /* 0x3E663769, 0x72BEA4D0 */

static volatile double
twom1000= 9.33263618503218878990e-302;     /* 2**-1000=0x01700000,0*/

#define GET_HIGH_WORD(i,d) \
do { \
  VIP_ENCDOUBLE gh_u = (d); \
  VIP_ENCULONG gh_v = *(VIP_ENCULONG *)&gh_u; \
  (i) = (VIP_ENCUINT)(gh_v >> 32); \
} while (0)

#define GET_LOW_WORD(i,d) \
do { \
  VIP_ENCDOUBLE gh_u = (d); \
  VIP_ENCULONG gh_v = *(VIP_ENCULONG *)&gh_u; \
  (i) = (VIP_ENCUINT)(gh_v & 0xffffffff); \
} while (0)

#define STRICT_ASSIGN(type, lval, rval) ((lval) = (rval))

#define INSERT_WORDS(d,ix0,ix1)         \
do {                \
  VIP_ENCUINT iw_h = (ix0); \
  VIP_ENCUINT iw_l = (ix1); \
  VIP_ENCULONG iw = (((VIP_ENCULONG)iw_h << 32) | (VIP_ENCULONG)iw_l); \
  VIP_ENCDOUBLE gh_u = *(VIP_ENCDOUBLE *)&iw; \
  (d) = gh_u; \
} while (0)

#define EXTRACT_WORDS(ix0,ix1,d)        \
do {                \
  VIP_ENCDOUBLE gh_u = (d); \
  VIP_ENCULONG gh_v = *(VIP_ENCULONG *)&gh_u; \
  (ix0) = (VIP_ENCUINT)(gh_v >> 32); \
  (ix1) = (VIP_ENCUINT)(gh_v & 0xffffffff); \
} while (0)

#define SET_HIGH_WORD(d,v) \
do { \
  VIP_ENCDOUBLE gh_u = (d); \
  VIP_ENCULONG gh_v = *(VIP_ENCULONG *)&gh_u; \
  gh_v = (((VIP_ENCULONG)(v)) << 32) | (gh_v & 0xffffffffUL); \
  (d) = *(VIP_ENCDOUBLE *)&gh_v; \
} while (0)

VIP_ENCDOUBLE
myexp(VIP_ENCDOUBLE x)  /* default IEEE double exp */
{
  VIP_ENCDOUBLE y=0.0,hi=0.0,lo=0.0,c,t=0.0,twopk=0.0;
  VIP_ENCINT k=0,xsb;
  VIP_ENCUINT hx;

  GET_HIGH_WORD(hx,x);
  xsb = (hx>>31)&1;   /* sign bit of x */
  hx &= 0x7fffffff;   /* high word of |x| */

#ifdef VIP_DO_MODE
  // uint16_t except;
  VIP_ENCDOUBLE retval = 0.0;

  VIP_ENCBOOL _pred1 = (hx >= 0x40862E42);  /* _pred1: if |x|>=709.78... */
  VIP_ENCBOOL _pred2 = (hx>=0x7ff00000);  /* _pred2: ... */
  VIP_ENCUINT lx;
  GET_LOW_WORD(lx,x);
  VIP_ENCBOOL _pred3 = (((hx&0xfffff)|lx)!=0);  /* _pred3: ... */
  retval = VIP_CMOV(_pred1 && _pred2 && _pred3, x+x, retval);
  VIP_ENCBOOL _pred3a = (xsb == 0);  /* _pred3a: exp(+-inf)={inf,0} */
  retval = VIP_CMOV(_pred1 && _pred2 && !_pred3 && _pred3a, x, retval);
  retval = VIP_CMOV(_pred1 && _pred2 && !_pred3 && !_pred3a, (VIP_ENCDOUBLE)0.0, retval);

  // /* ignore FP exceptions: */ fegetexceptflag(&except, FE_ALL_EXCEPT);

  VIP_ENCBOOL _pred4 = (x > o_threshold);  /* _pred4: ... */
  VIP_ENCBOOL _pred5 = (x < u_threshold);  /* _pred5: ... */
  retval = VIP_CMOV(_pred1 && !_pred2 && _pred4, (VIP_ENCDOUBLE)(huge*huge), retval); /* overflow */
  retval = VIP_CMOV(_pred1 && !_pred2 && !_pred4 && _pred5, (VIP_ENCDOUBLE)(twom1000*twom1000), retval); /* underflow */

  /* pre-check escape predicate, must be true for all later CMOV's */
  VIP_ENCBOOL _escape1 = (!_pred1 || (_pred1 && !_pred2 && !_pred4 && !_pred5));

  /* this implementation gives 2.7182818284590455 for exp(1.0),
     which is well within the allowable error. however,
     2.718281828459045 is closer to the true value so we prefer that
     answer, given that 1.0 is such an important argument value. */
  VIP_ENCBOOL _pred6 = (x == 1.0); /* _pred6: x == 1.0 */
  retval = VIP_CMOV(_escape1 && _pred6, (VIP_ENCDOUBLE)2.718281828459045235360, retval);

  /* pre-check escape predicate, must be true for all later CMOV's */
  VIP_ENCBOOL _escape2 = (_escape1 && !_pred6);

  /* argument reduction */
  VIP_ENCBOOL _pred7 = (hx > 0x3fd62e42); /* _pred7: if  |x| > 0.5 ln2 */
  VIP_ENCBOOL _pred8 = (hx < 0x3FF0A2B2); /* _pred8: and |x| < 1.5 ln2 */
  VIP_ENCDOUBLE _xsbval = VIP_CMOV(xsb == 1, x-ln2HI[1], x-ln2HI[0]);
  hi = VIP_CMOV(_escape2 && _pred7 && _pred8, _xsbval, hi);
  _xsbval = VIP_CMOV(xsb == 1, (VIP_ENCDOUBLE)ln2LO[1], (VIP_ENCDOUBLE)ln2LO[0]);
  lo = VIP_CMOV(_escape2 && _pred7 && _pred8, _xsbval, lo);
  k = VIP_CMOV(_escape2 && _pred7 && _pred8, (VIP_ENCINT)1-xsb-xsb, k);

  _xsbval = VIP_CMOV(xsb == 1, (VIP_ENCDOUBLE)invln2*x+halF[1], (VIP_ENCDOUBLE)invln2*x+halF[0]);
  k = VIP_CMOV(_escape2 && _pred7 && !_pred8, (VIP_ENCINT)(_xsbval), k);
  t = VIP_CMOV(_escape2 && _pred7 && !_pred8, (VIP_ENCDOUBLE)k, t);
  hi = VIP_CMOV(_escape2 && _pred7 && !_pred8, x - t*ln2HI[0], hi); /* t*ln2HI is exact here */
  lo = VIP_CMOV(_escape2 && _pred7 && !_pred8, t*ln2LO[0], lo);

  VIP_ENCDOUBLE tmpx;
  STRICT_ASSIGN(VIP_ENCDOUBLE, tmpx, hi - lo);
  x = VIP_CMOV(_escape2 && _pred7, tmpx, x);

  VIP_ENCBOOL _pred9 = (hx < 0x3e300000); /* _pred9: when |x|<2**-28 */
  VIP_ENCBOOL _pred10 = ((VIP_ENCDOUBLE)huge+x>one); /* _pred10: ... */
  retval = VIP_CMOV(_escape2 && !_pred7 && _pred9 && _pred10, (VIP_ENCDOUBLE)one+x, retval);/* trigger inexact */

  k = VIP_CMOV(_escape2 && !_pred7 && !_pred9, (VIP_ENCINT)0, k);

  /* pre-check escape predicate, must be true for all later CMOV's */
  VIP_ENCBOOL _escape3 = (_escape2 && !(!_pred7 && _pred9 && _pred10));

  /* x is now in primary range */
  t  = x*x;
  VIP_ENCBOOL _pred11 = (k >= -1021); /* _pred11: ... */
  VIP_ENCDOUBLE tmp_twopk;
  INSERT_WORDS(tmp_twopk,(VIP_ENCINT)0x3ff00000+(k<<20), 0);
  twopk = VIP_CMOV(_escape3 && _pred11, tmp_twopk, twopk);

  INSERT_WORDS(tmp_twopk,(VIP_ENCINT)0x3ff00000+((k+1000)<<20), 0);
  twopk = VIP_CMOV(_escape3 && !_pred11, tmp_twopk, twopk);

  c = x - t*((VIP_ENCDOUBLE)P1+t*((VIP_ENCDOUBLE)P2+t*((VIP_ENCDOUBLE)P3+t*((VIP_ENCDOUBLE)P4+t*P5))));
  VIP_ENCBOOL _pred12 = (k==0); /* _pred12: ... */
  retval = VIP_CMOV(_escape3 && _pred12, (VIP_ENCDOUBLE)one-((x*c)/(c-2.0)-x), retval);

  y = VIP_CMOV(_escape3 && !_pred12, (VIP_ENCDOUBLE)one-((lo-(x*c)/((VIP_ENCDOUBLE)2.0-c))-hi), y);

  /* pre-check escape predicate, must be true for all later CMOV's */
  VIP_ENCBOOL _escape4 = (_escape3 && !_pred12);

  VIP_ENCBOOL _pred13 = (k >= -1021); /* _pred13: ... */
  VIP_ENCBOOL _pred14 = (k == 1024);  /* _pred14: ... */
  retval = VIP_CMOV(_escape4 && _pred13 && _pred14, y*2.0*0x1p1023, retval);
  retval = VIP_CMOV(_escape4 && _pred13 && !_pred14, y*twopk, retval);
  retval = VIP_CMOV(_escape4 && !_pred13, y*twopk*twom1000, retval);

  // /* resume FP exceptions: */ fesetexceptflag(&except, FE_ALL_EXCEPT);

  return retval;
#else /* !VIP_DO_MODE */
  /* filter out non-finite argument */
  if (hx >= 0x40862E42) {     /* _pred1: if |x|>=709.78... */
    if (hx>=0x7ff00000) {     /* _pred2: ... */
      u_int32_t lx;
      GET_LOW_WORD(lx,x);
      if (((hx&0xfffff)|lx)!=0)  /* _pred3: ... */
        return x+x;     /* NaN */
      else
        return (xsb==0)? x:0.0; /* !_pred3: exp(+-inf)={inf,0} */
    }
    if (x > o_threshold)  /* _pred4: ... */
      return huge*huge; /* overflow */
    if (x < u_threshold)  /* _pred5: ... */
      return twom1000*twom1000; /* underflow */
  }

  /* this implementation gives 2.7182818284590455 for exp(1.0),
     which is well within the allowable error. however,
     2.718281828459045 is closer to the true value so we prefer that
     answer, given that 1.0 is such an important argument value. */
  if (x == 1.0) /* _pred6: x == 1.0 */
    return 2.718281828459045235360;

  /* argument reduction */
  if (hx > 0x3fd62e42) {    /* _pred7: if  |x| > 0.5 ln2 */
    if (hx < 0x3FF0A2B2) {  /* _pred8: and |x| < 1.5 ln2 */
      hi = x-ln2HI[xsb];
      lo=ln2LO[xsb];
      k = 1-xsb-xsb;
    }
    else {
      k  = (int)(invln2*x+halF[xsb]);
      t  = k;
      hi = x - t*ln2HI[0];  /* t*ln2HI is exact here */
      lo = t*ln2LO[0];
    }
    STRICT_ASSIGN(double, x, hi - lo);
  }
  else if (hx < 0x3e300000)  {  /* _pred9: when |x|<2**-28 */
    if (huge+x>one)   /* _pred10: ... */
      return one+x;/* trigger inexact */
  }
  else
    k = 0;

  /* x is now in primary range */
  t  = x*x;
  if (k >= -1021) /* _pred11: ... */
    INSERT_WORDS(twopk,0x3ff00000+(k<<20), 0);
  else /* !_pred11: ... */
    INSERT_WORDS(twopk,0x3ff00000+((k+1000)<<20), 0);
  c = x - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
  if (k==0) /* _pred12: ... */
    return one-((x*c)/(c-2.0)-x);
  else /* !_pred12: ... */
    y = one-((lo-(x*c)/(2.0-c))-hi);
  if (k >= -1021) { /* _pred13: ... */
    if (k==1024)  /* _pred14: ... */
      return y*2.0*0x1p1023;
    return y*twopk;
  }
  else {  /* !_pred13: ... */
    return y*twopk*twom1000;
  }
#endif /* VIP_DO_MODE */
}

#ifdef notdef

static const double
ln2_hi  =  6.93147180369123816490e-01,  /* 3fe62e42 fee00000 */
ln2_lo  =  1.90821492927058770002e-10,  /* 3dea39ef 35793c76 */
two54   =  1.80143985094819840000e+16,  /* 43500000 00000000 */
Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

static const double zero   =  0.0;

VIP_ENCDOUBLE
mylog(VIP_ENCDOUBLE x)
{
  VIP_ENCDOUBLE hfsq = 0.0,f,s,z,R = 0.0,w,t1,t2,dk = 0.0;
  VIP_ENCINT k,hx,i,j;
  VIP_ENCUINT lx;

#ifdef VIP_DO_MODE
  VIP_ENCINT _tmp;
  VIP_ENCDOUBLE retval = 0.0;
  VIP_ENCBOOL _returned = false;

  EXTRACT_WORDS(hx,lx,x);
  k=0;
  VIP_ENCBOOL _pred0 = (hx < 0x00100000); /* _pred0: x < 2**-1022  */
  VIP_ENCBOOL _pred1 = (((hx&0x7fffffff)|(/*FIXME*/VIP_ENCINT)lx)==0); /* _pred1 */
  retval = VIP_CMOV(_pred0 && _pred1, -((VIP_ENCDOUBLE)two54)/zero, retval); /* log(+-0)=-inf */
  _returned = _returned || (_pred0 && _pred1);

  VIP_ENCBOOL _pred2 = (hx<0); /* _pred2 */
  retval = VIP_CMOV(!_returned && _pred0 && _pred2, (x-x)/zero, retval); /* log(-#) = NaN */
  _returned = _returned || (_pred0 && _pred2);
  k = VIP_CMOV(!_returned && _pred0, k - 54, k); /* subnormal number, scale up */
  x = VIP_CMOV(!_returned && _pred0, x * two54, x);
  GET_HIGH_WORD(_tmp,x);
  hx = VIP_CMOV(!_returned && _pred0, _tmp, hx);

  VIP_ENCBOOL _pred3 = (hx >= 0x7ff00000); /* _pred3 */
  retval = VIP_CMOV(!_returned && _pred3, x+x, retval);
  _returned = _returned || _pred3;

  k += (hx>>20)-1023;
  hx = hx & 0x000fffff;
  i = (hx+0x95f64)&0x100000;
  SET_HIGH_WORD(x,hx|(i^0x3ff00000)); /* normalize x or x/2 */
  k += (i>>20);
  f = x-1.0;

  VIP_ENCBOOL _pred4 = ((((VIP_ENCINT)0x000fffff)&((VIP_ENCINT)2+hx))<3); /* _pred4: -2**-20 <= f < 2**-20 */
  VIP_ENCBOOL _pred5 = (f==zero); /* _pred5 */
  VIP_ENCBOOL _pred6 = (k==0); /* _pred6 */
  retval = VIP_CMOV(!_returned && _pred4 && _pred5 && _pred6, (VIP_ENCDOUBLE)zero, retval);
  _returned = _returned || (_pred4 && _pred5 && _pred6);

  dk = VIP_CMOV(!_returned && _pred4 && _pred5 && !_pred6, (VIP_ENCDOUBLE)k, dk);
  retval = VIP_CMOV(!_returned && _pred4 && _pred5 && !_pred6, dk*ln2_hi+dk*ln2_lo, retval);
  _returned = _returned || (_pred4 && _pred5 && !_pred6);

  R = VIP_CMOV(!_returned && _pred4, f*f*((VIP_ENCDOUBLE)0.5-(VIP_ENCDOUBLE)0.33333333333333333*f), R);
  VIP_ENCBOOL _pred7 = (k==0); /* _pred7 */
  retval = VIP_CMOV(!_returned && _pred4 && _pred7, f-R, retval);
  _returned = _returned || (_pred4 && _pred7);

  dk = VIP_CMOV(!_returned && _pred4 && !_pred7, (VIP_ENCDOUBLE)k, dk);
  retval = VIP_CMOV(!_returned && _pred4 && !_pred7, dk*ln2_hi-((R-dk*ln2_lo)-f), retval);
  _returned = _returned || (_pred4 && !_pred7);

  s = f/((VIP_ENCDOUBLE)2.0+f);
  dk = (VIP_ENCDOUBLE)k;
  z = s*s;
  i = hx-0x6147a;
  w = z*z;
  j = (VIP_ENCINT)0x6b851-hx;
  t1= w*((VIP_ENCDOUBLE)Lg2+w*((VIP_ENCDOUBLE)Lg4+w*Lg6));
  t2= z*((VIP_ENCDOUBLE)Lg1+w*((VIP_ENCDOUBLE)Lg3+w*((VIP_ENCDOUBLE)Lg5+w*Lg7)));
  i = i | j;
  R = t2+t1;

  VIP_ENCBOOL _pred8 = (i>0);
  hfsq = VIP_CMOV(!_returned && _pred8, (VIP_ENCDOUBLE)0.5*f*f, hfsq);

  VIP_ENCBOOL _pred9 = (k==0);
  retval = VIP_CMOV(!_returned && _pred8 && _pred9, f-(hfsq-s*(hfsq+R)), retval);
  _returned = _returned || (_pred8 && _pred9);

  retval = VIP_CMOV(!_returned && _pred8 && !_pred9, dk*ln2_hi-((hfsq-(s*(hfsq+R)+dk*ln2_lo))-f), retval);
  _returned = _returned || (_pred8 && !_pred9);

  retval = VIP_CMOV(!_returned && !_pred8 && _pred9, f-s*(f-R), retval);
  _returned = _returned || (!_pred8 && _pred9);

  retval = VIP_CMOV(!_returned && !_pred8 && !_pred9, dk*ln2_hi-((s*(f-R)-dk*ln2_lo)-f), retval);
  _returned = _returned || (!_pred8 && !_pred9);

  return retval;
#else /* VIP_DO_MODE */
  EXTRACT_WORDS(hx,lx,x);
  k=0;
  if (hx < 0x00100000) {      /* x < 2**-1022  */
      if (((hx&0x7fffffff)|lx)==0)
    return -two54/zero;   /* log(+-0)=-inf */
      if (hx<0) return (x-x)/zero;  /* log(-#) = NaN */
      k -= 54; x *= two54; /* subnormal number, scale up x */
      GET_HIGH_WORD(hx,x);
  }
  if (hx >= 0x7ff00000) return x+x;
  k += (hx>>20)-1023;
  hx &= 0x000fffff;
  i = (hx+0x95f64)&0x100000;
  SET_HIGH_WORD(x,hx|(i^0x3ff00000)); /* normalize x or x/2 */
  k += (i>>20);
  f = x-1.0;
  if((0x000fffff&(2+hx))<3) { /* -2**-20 <= f < 2**-20 */
      if(f==zero) {
    if(k==0) {
        return zero;
    } else {
        dk=(double)k;
        return dk*ln2_hi+dk*ln2_lo;
    }
      }
      R = f*f*(0.5-0.33333333333333333*f);
      if(k==0) return f-R; else {dk=(double)k;
             return dk*ln2_hi-((R-dk*ln2_lo)-f);}
  }
  s = f/(2.0+f);
  dk = (double)k;
  z = s*s;
  i = hx-0x6147a;
  w = z*z;
  j = 0x6b851-hx;
  t1= w*(Lg2+w*(Lg4+w*Lg6));
  t2= z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7)));
  i |= j;
  R = t2+t1;
  if(i>0) {
      hfsq=0.5*f*f;
      if(k==0) return f-(hfsq-s*(hfsq+R)); else
         return dk*ln2_hi-((hfsq-(s*(hfsq+R)+dk*ln2_lo))-f);
  } else {
      if(k==0) return f-s*(f-R); else
         return dk*ln2_hi-((s*(f-R)-dk*ln2_lo)-f);
  }
#endif /* !VIP_DO_MODE */
}
#endif /* notdef */


#ifdef TEST
# define myM_El   2.7182818284590452353602874713526625L  /* e */

int
main(void)
{
  fprintf(stderr, "mysqrt(64.0) = %lf (should be 8.0)\n", mysqrt(64.0));
  fprintf(stderr, "mysqrt(4.0) = %lf (should be 2.0) \n", mysqrt(4.0));
  fprintf(stderr, "mysqrt(34232.6) = %lf (should be 185.0205394)\n", mysqrt(34232.6));
  fprintf(stderr, "\n");
  fprintf(stderr, "mysin(0.0) = %lf (should be 0.0)\n", mysin(0.0));
  fprintf(stderr, "mysin(pi/6) = %lf (should be 0.5)\n", mysin(0.52359877559829887307710723054658383L));
  fprintf(stderr, "mysin(-pi/6) = %lf (should be 0.5)\n", mysin(-0.52359877559829887307710723054658383L));
  fprintf(stderr, "mysin(pi/2) = %lf (should be 1.0)\n", mysin(1.5707963267948966192313216916397514L));
  fprintf(stderr, "mysin(-pi/2) = %lf (should be -1.0)\n", mysin(-1.5707963267948966192313216916397514L));
  fprintf(stderr, "mysin(0.7) = %lf (should be 0.64421768723769)\n", mysin(0.7L));
  fprintf(stderr, "\n");
  fprintf(stderr, "mycos(0.0) = %lf (should be 1.0)\n", mycos(0.0));
  fprintf(stderr, "mycos(2 * pi/6) = %lf (should be 0.5)\n", mycos(2.0L * 0.52359877559829887307710723054658383L));
  fprintf(stderr, "mycos(4 * pi/6) = %lf (should be -0.5)\n", mycos(4.0L * 0.52359877559829887307710723054658383L));
  fprintf(stderr, "mycos(pi/2) = %lf (should be 0.0)\n", mycos(1.5707963267948966192313216916397514L));
  fprintf(stderr, "mycos(0.7) = %lf (should be 0.764842187284)\n", mycos(0.7L));
  fprintf(stderr, "\n");
  fprintf(stderr, "mypow(2.0, 0) = %lf (should be 1.0)\n", mypow(2.0, 0));
  fprintf(stderr, "mypow(2.0, 1) = %lf (should be 2.0)\n", mypow(2.0, 1));
  fprintf(stderr, "mypow(2.0, 10) = %lf (should be 1024.0)\n", mypow(2.0, 10));
  fprintf(stderr, "\n");
  fprintf(stderr, "myfloor(7.0) = %lf (should be 7.0)\n", myfloor(7.0L));
  fprintf(stderr, "myfloor(7.1) = %lf (should be 7.0)\n", myfloor(7.1L));
  fprintf(stderr, "myfloor(7.9) = %lf (should be 7.0)\n", myfloor(7.9L));
  fprintf(stderr, "myfloor(-7.0) = %lf (should be -7.0)\n", myfloor(-7.0L));
  fprintf(stderr, "myfloor(-7.1) = %lf (should be -8.0)\n", myfloor(-7.1L));
  fprintf(stderr, "myfloor(-7.9) = %lf (should be -8.0)\n", myfloor(-7.9L));
  fprintf(stderr, "\n");
  fprintf(stderr, "mylog(1.0) = %lf (should be 0.0)\n", mylog(1.0L));
  fprintf(stderr, "mylog(e) = %lf (should be 1.0)\n", mylog(myM_El));
  fprintf(stderr, "mylog(1.0 / e) = %lf (should be -1.0)\n", mylog(1.0L / myM_El));
  fprintf(stderr, "mylog(2.0) = %lf (should be 0.693147)\n", mylog(2.0L));
  fprintf(stderr, "mylog(10.0) = %lf (should be 2.302585)\n", mylog(10.0L));
  fprintf(stderr, "mylog(0.7) = %lf (should be -0.356675)\n", mylog(0.7L));

  return 0;
}
#endif /* TEST */
