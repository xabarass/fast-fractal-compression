# Fractal compression project 

Project uses `CMake` as a build system. There are different flags that can be used to generate project with different level of optimizations:
1. `USE_FMA` - use Fused multiply–add instructions. (default ON)
2. `USE_VECTORIZE` - Use SIMD instructions for computation. (default ON)
3. `USE_CACHE_OPT` - Use cache friendly implementation of the algorithm, using precomputation. (default ON)
4. `USE_BULK_COMPUTE` - Use special functions for getting average pixels that returns values for 2 levels of recursion at once. Can be used only as an addition to cache optimization. (default ON)
5. `RDTSC_FAILBACK` - Use RDTSC hardware counters for measuring performance. If disabled Intel PCM is used, requires aditional kernel modules and sudo root access to the machine. (default ON)

Building a project:
```sh
$ cmake -DCMAKE_BUILD_TYPE=Release -DUSE_FMA=ON -DRDTSC_FAILBACK=ON -DGENERATE_FLOP_COUNT=ON -DUSE_VECTORIZE=ON -DUSE_CACHE_OPT=ON -DUSE_BULK_COMPUTE=OFF
$ make
$ ./fractal-compression <path_to_image.bmp>
```

### Useful links

* [Intel Intrinsics](https://software.intel.com/sites/landingpage/IntrinsicsGuide/) - Intrinsics references
* [Agner's table](http://www.agner.org/optimize/instruction_tables.pdf) - Agners instruction table
* [Manual](http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf) - Intel's software optimization manual

### Original implementation
Implementation used as a reference: (https://github.com/kennberg/fractal-compression)
