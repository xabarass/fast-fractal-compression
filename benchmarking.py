from git import Repo
import os
import shutil
import subprocess
from os import listdir
from os.path import isfile, join
import re
import matplotlib.pyplot as plt
import matplotlib

repo=Repo('.')

bench_result_dir=os.path.join(os.getcwd(),"bench_results")
if not os.path.exists(bench_result_dir):
    os.makedirs(bench_result_dir)

class Benchmark:
    def __init__(self, branch):
        self.branch=branch
        self.compiler_flags={'CMAKE_BUILD_TYPE':'Release'}

    def add_compiler_flag(self, name, value):
        self.compiler_flags[name]=value      

re_perf=re.compile(
    r'#### PERFORMANCE RESULTS #####.Encode cycles: ([0-9]*).*?Decode cycles: ([0-9]*).*?No. of transformations: ([0-9]*).*?Image size: w: ([0-9]*) h: ([0-9]*).*?Compression ratio: ([0-9 .]*)',
    re.M | re.DOTALL
)   

class BenchResult:
    def __init__(self, branch_name, branch_index, result=None):
        self.branch_name=branch_name
        self.branch_index=branch_index
        if result:
            self.enc_cycles=result[0]
            self.dec_cycles=result[1]
            self.transf_num=result[2]
            self.img_size=(result[3],result[4])
            self.compress_rat=result[5]
        else:
            self.enc_cycles=0
            self.dec_cycles=0
            self.transf_num=0
            self.img_size=(0,0)
            self.compress_rat=0

def init_plot_data(image_results):
    branch_performance={}
    elements=0
    for img, benchs in image_results.items():
        elements=elements+1
        for bench in benchs:
            if not bench.branch_index in branch_performance:
                branch_performance[bench.branch_index]=[]

            branch_performance[bench.branch_index].append(bench)

        for k, bp in branch_performance.items():
            while len(bp)<elements:
                bp.append(BenchResult(k,k))

    return branch_performance

def draw_encode_timings(branch_performance, img_results):
    images=range(len(img_results))
    cycles=[]
    for bp in branch_performance[0]:
        cycles.append(bp.enc_cycles)

    axes = plt.gca()
    plt.grid(True)
    plt.plot(images, cycles, '-ro')

    return plt

simd_bench=Benchmark('master')
simd_bench.add_compiler_flag('USE_FMA','ON')
simd_bench.add_compiler_flag('RDTSC_FAILBACK','ON')

test_bench=Benchmark('test_git')
test_bench.add_compiler_flag('USE_FMA','OFF')
test_bench.add_compiler_flag('RDTSC_FAILBACK','ON')

benchmarks=[test_bench, simd_bench]

test_image_results={}

# repo.remotes['origin'].fetch()

index=0;
for bench in benchmarks:
    print("**** Benchmarking %s ****" %bench.branch)
    # # Switch to branch and pull changes
    # if bench.branch in repo.refs:
    #     print("Branch "+bench.branch+" already exists")
    #     repo.refs[bench.branch].checkout()
    #     repo.remotes.origin.pull()
    # else:
    #     print("Branch "+bench.branch+" doesn't exist, checking out")        
    #     repo.git.checkout('refs/remotes/origin/'+bench.branch, b=bench.branch)

    # Configure project
    print("Configuring project")

    release_dir=os.path.join(os.getcwd(),bench.branch+"_release")
    if os.path.exists(release_dir):
        shutil.rmtree(release_dir)

    os.makedirs(release_dir)

    cmake_command=['cmake']
    for flag, prop in bench.compiler_flags.items():
        cmake_command.append('-D'+flag+"="+prop)
    cmake_command.append('-B'+release_dir)
    cmake_command.append('-H.')

    cmake_res=subprocess.check_call(cmake_command)

    make_command=['make','-C',release_dir,'-j8']
    make_res=subprocess.check_call(make_command)

    image_dir=os.path.join(os.getcwd(),"test_images","benchmark")
    if not os.path.exists(image_dir):
        print("ERROR! Cannot run tests! Test images are not found on branch "+bench.branch)
        continue

    test_images = [f for f in listdir(image_dir) if isfile(join(image_dir, f))]
    for image in test_images:
        if image not in test_image_results:
            test_image_results[image]=[]

    run_command=[os.path.join(release_dir,'bin','fractal-compression')]
    
    for image in test_images:
        print("Testing image %s" %image)
        command=run_command[:]
        command.append(os.path.join(image_dir, image))

        output=subprocess.check_output(command)
        results=re_perf.findall(output.decode("utf-8"))

        assert len(results)==1
        test_image_results[image].append(BenchResult(bench.branch, index, results[0]))

    index=index+1

branch_perf=init_plot_data(test_image_results);
plt=draw_encode_timings(branch_perf, test_image_results)

plt.show()
