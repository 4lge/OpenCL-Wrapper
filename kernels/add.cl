kernel void add_kernel(global float* A, global float* B, global float* C) { // equivalent to "for(uint n=0u; n<N; n++) {", but executed in parallel
  const uint n = get_global_id(0);
  C[n] = A[n]+B[n];
}
