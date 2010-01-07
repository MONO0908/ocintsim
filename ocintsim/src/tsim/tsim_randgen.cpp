#ifdef CONDOR
#define _ISOC99_SOURCE
#include "tsim.h"
#include <cmath>
#else
#include "tsim.h"
#include "math.h"
#endif

tsim_randgen::tsim_randgen() {
  init(TSIM_RANDGEN_UNIFORM, 100, 0);
}

tsim_randgen::tsim_randgen( enum tsim_randgen_method m, float _f ) 
{
  init(m, _f);
}

tsim_randgen::tsim_randgen(enum tsim_randgen_method m, 
                           tsim_i64 _v) {
  init(m, _v);
}

tsim_randgen::tsim_randgen(enum tsim_randgen_method m, 
                           tsim_i64 _max,
                           tsim_i64 _min ) {
  init(m, _max, _min);
}

tsim_randgen::tsim_randgen(enum tsim_randgen_method m, 
                           tsim_i64 _max,
                           tsim_i64 _min,
                           tsim_i64 _mean) 
{
  init(m, _max, _min, _mean);
}

tsim_randgen::~tsim_randgen()
{
}

void tsim_randgen::init( enum tsim_randgen_method m,  float _f ) 
{
  // just ignore m
  method = TSIM_RANDGEN_BOOLEAN;
  false_bias = _f;
  legal_ranges = true;
}

void tsim_randgen::init( enum tsim_randgen_method m,  tsim_i64 _v ) 
{
  // just ignore m
  method = TSIM_RANDGEN_FIXED;
  fixed_value = _v;
  legal_ranges = true;
}


void tsim_randgen::init(enum tsim_randgen_method m, 
                        tsim_i64 _max,
                        tsim_i64 _min )
{
  method = m;
  max = _max + 1; // to take care of arithmetic corners
  min = _min;
  i_max = _max;
  i_min = _min;

  switch (m) {
  case TSIM_RANDGEN_EXP:
    method = TSIM_RANDGEN_EXP;
    max_a = max;
    min_a = min;
    EXP_MAX = 13;
    legal_ranges = false;
    if (max > min) legal_ranges = true;
    break;
  case TSIM_RANDGEN_UNIFORM:
    if (max >= min)
      legal_ranges = true;
    else legal_ranges = false;
    break;
  default:
    cerr << "Illegal random number generator method...dying here" << endl;
    exit(-1);
  }

}

void tsim_randgen::init(enum tsim_randgen_method m, 
                        tsim_i64 _max,
                        tsim_i64 _min,
                        tsim_i64 _mean) 
{

  max = _max+1; // to take care of arithmetic corners, since we do a trunc
  min = _min;
  mean = _mean;

  i_max = _max;
  i_min = _min;

  legal_ranges = false;
  if (max >= min) {
    if ( (mean > min) && (mean < max) )
      legal_ranges = true;
  }

  GAUSS_MAX = 3.5;
  GAUSS_MIN = -3.5;
  bounds_check = true;
  switch (m) {
  case TSIM_RANDGEN_NORMAL:
    method = TSIM_RANDGEN_NORMAL;
    if (( (_max + _min)/2) != _mean) {
      if ( (max - mean) < (mean - min) ) {
        min_a = mean - (_max - mean);
        max_a = max;
      } else {
        min_a = min;
        max_a = (mean - _min) + mean + 1;
      }
      bounds_check = false;
    } else {
      max_a = max;
      min_a = min;
    }
    break;
  default:
    cerr << "Illegal random number generator method...dying here" << endl;
    exit(-1);
  }


  //  cerr  << max_a << " " << min_a << " " << mean << endl;

}

tsim_i64 tsim_randgen::next() {
  if (!legal_ranges) return 0;
  return new_number();
}


bool tsim_randgen::next(tsim_i64 &x) {
  if (!legal_ranges) return false;
  x = new_number();
  return true;
}

float tsim_randgen::randf() {
  float f;
  f = rand()*1.0/RAND_MAX;

  return f;
}

tsim_i64 tsim_randgen::new_number() {

  switch (method) {
  case TSIM_RANDGEN_FIXED: return fixed_random();
  case TSIM_RANDGEN_UNIFORM: return uniform_random();
  case TSIM_RANDGEN_NORMAL: return normal_random();
  case TSIM_RANDGEN_EXP: return exp_random();
  case TSIM_RANDGEN_BOOLEAN: return boolean_random();
  default: return 0;
  }
}

tsim_i64 tsim_randgen::fixed_random() {
  return fixed_value;
}

tsim_i64 tsim_randgen::uniform_random() {
  float f = this->randf() * (max - min) + min;
  tsim_i64 t = (tsim_i64) trunc(f);
  if (t > i_max) t = i_max;
  if (t < i_min) t = i_min;
  return t;
}

tsim_i64 tsim_randgen::boolean_random() {
  float f = this->randf();
  if (f < false_bias) return 0; else return 1;
}


float tsim_randgen::gasdev() 
// Returns a normally distributed deviate with zero mean and unit
// variance, using ran1(idum) as the source of uniform deviates.
{ 
  static int iset=0; 
  static float gset; 
  float fac,rsq,v1,v2; 
  if (iset == 0) { // We don t have an extra deviate handy, so 
    do { 
      v1=2.0*this->randf()-1.0; // pick two uniform numbers in the square extending from -1 to +1 in each direction, 
      v2=2.0*this->randf()-1.0; 
      rsq=v1*v1+v2*v2;
    } while (rsq >= 1.0 || rsq == 0.0); // and if they are not, try again. 
    fac=sqrt(-2.0*log(rsq)/rsq); // Now make the Box-Muller transformation to get two normal deviates. Return one and save the other for next time. 
    gset=v1*fac; 
    iset=1; // Set flag. 
    return v2*fac; 
  } else { // We have an extra deviate handy, 
    iset=0; // so unset the flag, 
    return gset; //  and return it. 
  } 
}

float tsim_randgen::scale_gasdev(float f) {
  float n;
  n = (f * (max_a-min_a) / (GAUSS_MAX - GAUSS_MIN));
  n += mean;
  return n;
}

tsim_i64 tsim_randgen::normal_random() {
  float f;
  while (1) {

    while (1) {
      f = gasdev(); 
      if (!bounds_check) break;
      if ( (f < GAUSS_MAX) && (f > GAUSS_MIN) ) break;
    }
    f = scale_gasdev(f);
    if ( (f <= max) && (f >= min) ) {
      break;
    } else {
      while (1) {
        f = this->randf();
        f = f * (max - min - 1) + min;   // -1 is because we do a round
        if ( (f <= max) && (f >= min) ) break;
      }
      break;
    }
  }
  tsim_i64 t = (tsim_i64) round(f);
  if (t > i_max) t = i_max;
  if (t < i_min) t = i_min;
  return t;
}

tsim_i64 tsim_randgen::exp_random() {
  float f;

  while (1) {
    do 
      f=randf(); 
    while (f == 0.0);
    f = -log(f); 
    if (f < EXP_MAX) break;
  }

  f = (f/EXP_MAX) * (max_a - min_a) + min_a;
  tsim_i64 t = (tsim_i64) trunc(f);
  if (t > i_max) t = i_max;
  if (t < i_min) t = i_min;

  return t;
}


