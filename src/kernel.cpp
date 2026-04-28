#include "kernel.hpp" // note: unbalanced round brackets () are not allowed and string literals can't be arbitrarily long, so periodically interrupt with )+R(
string opencl_c_container() { return R( // ########################## begin of OpenCL C code ####################################################################

#if CONFIG_USE_DOUBLE

#if defined(cl_khr_fp64)  // Khronos extension available?
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define DOUBLE_SUPPORT_AVAILABLE
#elif defined(cl_amd_fp64)  // AMD extension available?
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#define DOUBLE_SUPPORT_AVAILABLE
#endif

#endif // CONFIG_USE_DOUBLE

#if defined(DOUBLE_SUPPORT_AVAILABLE)

// double
typedef double real_t;
typedef double2 real2_t;
typedef double3 real3_t;
typedef double4 real4_t;
typedef double8 real8_t;
typedef double16 real16_t;
#define PI 3.14159265358979323846

#else

// float
typedef float real_t;
typedef float2 real2_t;
typedef float3 real3_t;
typedef float4 real4_t;
typedef float8 real8_t;
typedef float16 real16_t;
#define PI 3.14159265359f

#endif

                                        
// Uniform random number generator (Mersenne Twister)
uint mt_rand(__private uint* state, __private int* idx, __private uint* mt) {
  if (*idx >= 624) {
    // Twist the state array
    for (int i = 0; i < 624; ++i) {
      uint temp = mt[i] ^ (mt[(i + 1) % 624] >> 1);
      temp ^= (temp >> 1) ^ ((temp & 1) ? 0x8EB8B000 : 0);
      mt[i] = mt[(i + 397) % 624] ^ temp;
    }
    *idx = 0;
  }
  uint x = mt[*idx];
  x ^= (x >> 11);
  x ^= (x << 7) & 0x9D2C5680;
  x ^= (x << 15) & 0xEFC60000;
  x ^= (x >> 18);
  (*idx)++;
  return x;
}
       
// Mersenne Twister scaled to [0,1]
real_t mt_rand_01(__private uint* state, __private int* idx, __private uint* mt) {
  return (real_t)mt_rand(&mt[0], &idx[0], mt) / 4294967295.0f;
}

// uniform distribution
real_t unif_rand(__private real_t lower, __private real_t upper,
		 __private int* idx, __private uint* mt) {
  return lower + (upper-lower)*mt_rand_01(&mt[0], &idx[0], mt);
}

// normal distribution
real_t norm_rand(__private real_t mu, __private real_t sigma,
                 __private int* idx, __private uint* mt) {
  return mu + sigma * sqrt(-2 * log(mt_rand_01(&mt[0], &idx[0], mt))) *
    cos(2 * 3.14159265359f * mt_rand_01(&mt[0], &idx[0], mt));
  // note we are wasting z1 = mu + sigma * sqrt(-2 * ln(u1)) * sin(2π * u2) here
}




);} // ############################################################### end of OpenCL C code #####################################################################
