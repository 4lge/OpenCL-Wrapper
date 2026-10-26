  __kernel void distance_matrix(__global real_ocl* output, __global const real_ocl* input, const int N, const int DIM) {
                size_t flat_id = get_global_id(0);
                size_t i = flat_id % N;
                size_t j = flat_id / N;
                if (i < N && j < N) {
                    if (i == j) { output[j * N + i] = (real_ocl)0.0; return; }
                    if (j < i) {
                        real_ocl tmpRes = (real_ocl)0.0;
                        for (int k = 0; k < DIM; ++k) {
                            real_ocl diff = input[i + k * N] - input[j + k * N];
                            tmpRes += diff * diff;
                        }
                        tmpRes = sqrt(tmpRes);
                        output[j * N + i] = tmpRes;
                        output[i * N + j] = tmpRes;
                    }
                }
           }
        
