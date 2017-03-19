# Fractal compression project 

Build debug version with enabled FMA in project root directory with RDTSC benchmarking falback off
```sh
$ cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_FMA=ON -DRDTSC_FAILBACK=OFF .
$ make
$ cd bin
$ ./fractal-compression 
```
Build release version without FMA in build directory release
```sh
$ cmake -DCMAKE_BUILD_TYPE=Release -DUSE_FMA=ON -Brelease -H.
$ cd release
$ make
$ cd bin
$ ./fractal-compression 
```
### Useful links

List of useful course links

* [Course webpage](https://www.inf.ethz.ch/personal/markusp/teaching/263-2300-ETH-spring17/course.html) - Contains course information and more
* [Intel Intrinsics](https://software.intel.com/sites/landingpage/IntrinsicsGuide/) - Intrinsics references
* [Agner's table](http://www.agner.org/optimize/instruction_tables.pdf) - Agners instruction table
* [Manual](http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-optimization-manual.pdf) - Intel's software optimization manual
* [Moodle](https://moodle-app2.let.ethz.ch/course/view.php?id=3122) - Homework submission


### Todos

 - Implement infrastructure classes
 - Implement testing software and design interface for testnig


**Free Software, Hell Yeah!**



