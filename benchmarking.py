#!/usr/bin/python3

from git import Repo
import os
import shutil
import subprocess
from os import listdir
from os.path import isfile, join
import re
import matplotlib.pyplot as plt
import matplotlib
from test_framework import test_transformation
import json
from pprint import pprint
import sys
import pickle
from shutil import copyfile

class Benchmark:
    def __init__(self, branch, bench_name):
        self.branch=branch
        self.compiler_flags={'CMAKE_BUILD_TYPE':'Release'}
        self.bench_name=bench_name

    def add_compiler_flag(self, name, value):
        self.compiler_flags[name]=value

re_perf=re.compile(
    r'#### PERFORMANCE RESULTS #####.Encode cycles: ([0-9]*).*?Decode cycles: ([0-9]*).*?No. of transformations: ([0-9]*).*?Image size: w: ([0-9]*) h: ([0-9]*).*?Compression ratio: ([0-9 .]*)',
    re.M | re.DOTALL
)

class BenchResult:
    def __init__(self, branch_name, result, enc_op_count, dec_op_count):
        self.branch_name=branch_name
        self.enc_cycles=result[0]
        self.dec_cycles=result[1]
        self.transf_num=result[2]
        self.img_size=(result[3],result[4])
        self.compress_rat=result[5]
        self.enc_op_count=enc_op_count
        self.dec_op_count=dec_op_count

def init_plot_data(image_results):
    image_results=sorted(image_results, key= lambda img: img.id)

    branch_performance={}
    for img in image_results:
        index=0
        for bench in img.results:
            if not index in branch_performance:
                branch_performance[index]=[]

            branch_performance[index].append(bench)
            index=index+1

    return branch_performance

def draw_encode_timings(branch_performance, img_results, value_generator, title, y_label, x_label):
    images=[x.id for x in img_results]

    colors=['ro','go','bo','yo']

    axes = plt.gca()
    plt.grid(True)

    legends=[]

    for k, branch_perf in branch_performance.items():
        cycles=[]
        color=colors.pop()
        legend_name="unknown"
        for bp in branch_perf:
            cycles.append(value_generator(bp))
            legend_name=bp.branch_name

        plt.plot(images, cycles, color)
        legends.append(legend_name)

    plt.title(title)
    plt.ylabel(y_label)
    plt.xlabel(x_label)
    plt.legend(legends)

    return plt

class TestImage:
    def __init__(self, ti):
        print("Loading image: %s id %d" %(ti["name"], ti["id"]))
        self.id=ti["id"]
        self.name=ti["name"]
        self.enc_op_count=ti["enc"]
        self.dec_op_count=ti["dec"]
        self.path=None
        self.results=[]

def get_test_images(relative_img_dir):
    tmp_image_dir=os.path.join(os.getcwd(), "benchmark_images")
    if not os.path.exists(tmp_image_dir):
        os.makedirs(tmp_image_dir)

    img_config=os.path.join(os.getcwd(), relative_img_dir, "ops.json")
    if not os.path.exists(img_config):
        raise Exception("Unknown image path!")

    try:
        with open(img_config) as data_file:    
            data = json.load(data_file)
    except FileNotFoundError:
        raise Exception("Image folder doesn't have ops.json file!")

    initialized_flag=os.path.join(os.getcwd(), relative_img_dir, "initialized.txt")
    if not os.path.exists(initialized_flag):
        raise Exception("Images are not initialized! Please run resize.sh script from test_images/benchmark to create bmp images of required size")

    img_dir=os.path.join(os.getcwd(), relative_img_dir)
    test_images=[]
    for ti in data["test_images"]:
        new_img=TestImage(ti)
        src=os.path.join(img_dir, new_img.name)
        dst=os.path.join(tmp_image_dir, new_img.name)

        copyfile(src, dst)

        new_img.path=dst
        test_images.append(new_img)
    
    return test_images


def run_tests(benchmarks, relative_img_dir):
    test_images=get_test_images(relative_img_dir)

    repo=Repo('.')
    repo.remotes['origin'].fetch()

    for bench in benchmarks:
        print("**** Benchmarking %s ****" %bench.branch)
        if bench.branch in repo.refs:
            print("Branch "+bench.branch+" already exists")
            repo.refs[bench.branch].checkout()
            repo.remotes.origin.pull()
        else:
            print("Branch "+bench.branch+" doesn't exist, checking out")
            repo.git.checkout('refs/remotes/origin/'+bench.branch, b=bench.branch)

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

        run_command=[os.path.join(release_dir,'bin','fractal-compression')]

        for image in test_images:
                
            print("Testing image %s" %image.name)

            command=run_command[:]
            command_params=test_transformation.run_test(bench.branch, image.name)
            command_elements=command_params.split(' ')
            command+=command_elements;
            command.append(image.path)
            output=subprocess.check_output(command)
            results=re_perf.findall(output.decode("utf-8"))

            test_transformation.compare_transformation_diff(bench.branch, image.name)
            test_transformation.compare_image_diff(bench.branch, image.name)

            assert len(results)==1
            image.results.append(BenchResult(bench.branch, results[0], image.enc_op_count, image.dec_op_count))

    return test_images

def load_benchmarks_from_config(config_file):
    try:
        with open(config_file) as data_file:    
            data = json.load(data_file)
    except FileNotFoundError:
        print("Config file not found!")
        exit(-1)

    img_dir=data["image_directory"]
    benchmarks=[]
    for test in data["tests"]:
        b=Benchmark(test['branch'], test['name'])
        print("Added branch "+b.bench_name)
        for k,v in test['compiler_flags'].items():
            print("Adding cf %s %s" %(k,v))
            b.add_compiler_flag(k,v)
        benchmarks.append(b)

    return benchmarks, img_dir

def load_benchmarks_from_cache(config_file):
    try:
        with open(config_file, "rb") as data_file:
            data=pickle.load(data_file)
            return data
    except FileNotFoundError:
        print("Config file not found!")
        exit(-1)

def save_benchmark_cache(data, config_file):
    try:
        with open(config_file, "wb") as data_file:
            pickle.dump(data, data_file)
    except FileNotFoundError:
        print("Cannot save data to config file!")


if len(sys.argv)<=1:
    print("You must provide name of a config file")
    exit(-1)


CONFIG_FILE_NAME="cached_bench.p"

test_image_data=None

if sys.argv[1]=='-c':
    test_image_data=load_benchmarks_from_cache(CONFIG_FILE_NAME)
else:
    benchmarks, img_dir=load_benchmarks_from_config(sys.argv[1])
    test_image_data=run_tests(benchmarks,img_dir)
    save_benchmark_cache(test_image_data, CONFIG_FILE_NAME)

branch_perf=init_plot_data(test_image_data);

def create_runtime_plot(branch_perf, test_image_data):
    return draw_encode_timings(
        branch_perf, test_image_data, 
        lambda b:int(b.enc_cycles),
        title='Encoding runtime in cycles for different sample images',
        y_label='# of cycles',
        x_label='Image number'
    )

def create_performance_plot(branch_perf, test_image_data):
    return draw_encode_timings(
        branch_perf, test_image_data, 
        lambda b:int(b.enc_op_count)/int(b.enc_cycles),
        title='Encoding performance plot, in flops/cycle',
        x_label='Image number',
        y_label='performance'
    )

plt=create_performance_plot(branch_perf,test_image_data)
# plt=create_runtime_plot(branch_perf,test_image_data)
plt.show()
