///////////////////////////////////////////////////////////////////////////////
// TSIM misc routines
///////////////////////////////////////////////////////////////////////////////

#ifndef TSIM_MISC_H
#define TSIM_MISC_H

#include <stdio.h>
#include <math.h>

#define TSIM_OKAY   1
#define TSIM_FAIL   0
#define TSIM_ERROR -1

#define tsim_i32 long
#define tsim_i64 long long
#define tsim_u32 unsigned long
#define tsim_u64 unsigned long long

#define tsim_stoi tsim_stoi32
#define tsim_itos tsim_i32tos

inline tsim_i32 tsim_stoi32( const string& s )
{
    tsim_i32 i = strtol( s.c_str(), NULL, 0 );
    return i;
}

inline string tsim_i32tos( tsim_i32 i )
{
    char temp[64];
    sprintf( temp, "%ld", i );
    return string( temp );
}

inline string tsim_i32tohs( tsim_i32 i )
{
    // convert to normal hex string
    char temp[64];
    sprintf( temp, "0x%lx", i );
    return string( temp );
}

inline string tsim_i32tozhs( tsim_i32 i )
{
    // convert to zero-padded hex string
    char temp[64];
    sprintf( temp, "0x%08lx", i );
    return string( temp );
}

inline string tsim_i32torhs( tsim_i32 i )
{
    // convert to raw hex string
    char temp[64];
    sprintf( temp, "%lx", i );
    return string( temp );
}

inline tsim_u32 tsim_stou32( const string& s )
{
    tsim_i32 i = strtoul( s.c_str(), NULL, 0 );
    return i;
}

inline string tsim_u32tos( tsim_u32 i )
{
    char temp[64];
    sprintf( temp, "%lu", i );
    return string( temp );
}

inline string tsim_u32tohs( tsim_u32 i )
{
    // convert to normal hex string
    char temp[64];
    sprintf( temp, "0x%lx", i );
    return string( temp );
}

inline string tsim_u32tozhs( tsim_u32 i )
{
    // convert to zero-padded hex string
    char temp[64];
    sprintf( temp, "0x%08lx", i );
    return string( temp );
}

inline string tsim_u32torhs( tsim_u32 i )
{
    // convert to raw hex string
    char temp[64];
    sprintf( temp, "%lx", i );
    return string( temp );
}

inline tsim_i64 tsim_stoi64( const string& s )
{
    tsim_i64 i = strtoll( s.c_str(), NULL, 0 );
    return i;
}

inline string tsim_i64tos( tsim_i64 i )
{
    char temp[64];
    sprintf( temp, "%lld", i );
    return string( temp );
}

inline string tsim_i64tohs( tsim_i64 i )
{
    // convert to normal hex string
    char temp[64];
    sprintf( temp, "0x%llx", i );
    return string( temp );
}

inline string tsim_i64tozhs( tsim_i64 i )
{
    // convert to zero-padded hex string
    char temp[64];
    sprintf( temp, "0x%016llx", i );
    return string( temp );
}

inline string tsim_i64torhs( tsim_i64 i )
{
    // convert to raw hex string
    char temp[64];
    sprintf( temp, "%llx", i );
    return string( temp );
}

inline tsim_i64 tsim_stou64( const string& s )
{
    tsim_u64 i = strtoull( s.c_str(), NULL, 0 );
    return i;
}

inline string tsim_u64tos( tsim_u64 i )
{
    char temp[64];
    sprintf( temp, "%llu", i );
    return string( temp );
}

inline string tsim_u64tohs( tsim_u64 i )
{
    // convert to normal hex string
    char temp[64];
    sprintf( temp, "0x%llx", i );
    return string( temp );
}

inline string tsim_u64tozhs( tsim_u64 i )
{
    // convert to zero-padded hex string
    char temp[64];
    sprintf( temp, "0x%016llx", i );
    return string( temp );
}

inline string tsim_u64torhs( tsim_u64 i )
{
    // convert to raw hex string
    char temp[64];
    sprintf( temp, "%llx", i );
    return string( temp );
}

// added by sibi
inline double logtwo(double x){
  return((double) (log10(x)/log10(2.0)));
}

inline string tsim_dtos( double d )
{
    char temp[64];
    sprintf( temp, "%f", d );
    return string( temp );
}

// =====================================================================
// This sub detects whether a number (int) is a power of 2. Works only 
// for positive powers of two.
// Inputs:
// num: number to test for
// Outputs:
// 1 if power of 2, 0 otherwise

// This sub was copied from wikipedia:
// http://en.wikipedia.org/wiki/Power_of_two#Fast_algorithm_to_check_if_a_number_is_a_power_of_two
// =====================================================================
inline bool tsim_is_power_of_2(int num)
{
  bool result = false;

  if (num > 0 &&
      (num & (num - 1)) == 0
      )
    {
      result = true;
    }

  return result;
}

//======================================================================
//Calculate the geometric mean of the elements of the array passed in.
//- 'numbers': the array containing numbers to be averaged.
//- 'size'   : size of 'numbers'.
//======================================================================
inline double tsim_gmean(double * numbers, int size)
{
  double sum = 0.0;

  if (!size || !numbers) {return 1.0;}
  
  for (int i = 0; i < size; i++)
    {
      sum += log(numbers[i]);
    }

  sum /= size;

  return exp(sum);
}

//======================================================================
//Calculate the arithmetic mean of the elements of the array passed in.
//- 'numbers': the array containing numbers to be averaged.
//- 'size'   : size of 'numbers'.
//======================================================================
inline double tsim_amean(double * numbers, int size)
{
  double sum = 0.0;

  if (!size || !numbers) {return 0.0;}
  
  for (int i = 0; i < size; i++)
    {
      sum += numbers[i];
    }

  sum /= size;

  return sum;
}

#endif
// TSIM_MISC_H
