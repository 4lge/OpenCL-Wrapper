       // uniform kernel
       kernel void unif_rng(global real_ocl* output, global int* seed,
                     const real_ocl lower, const real_ocl upper) {
         
         // Initialize Mersenne Twister state
         uint mt[624];
         int idx = 624;
         mt[0] = *seed + get_global_id(0);
         for (int i = 1; i < 624; ++i) {
           mt[i] = (181243 * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i);
         }

         real_ocl r_unif = unif_rand(lower, upper, &idx, mt);

         output[get_global_id(0)] = r_unif;
       }
