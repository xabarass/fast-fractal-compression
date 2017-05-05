import os
import time
from git import Repo

os.chdir("..")

test_images = range(1, 15)
TEST_IMAGE_FOLDER = "test_images"
TEST_IMAGE_SUB_FOLDER = "benchmark"
THRESHOLD = 100
PHASES = 5

LOG_FOLDER = "test_framework"
LOG_SUB_FOLDER = "transformation_log"
LOG_REF_FOLDER = "standards"
OUTPUT_FOLDER = "test_framework"
OUTPUT_SUB_FOLDER = "image_log"
OUTPUT_REF_FOLDER = "reference_images"


def generate_reference(name, image_name):
	log_path = os.path.join(LOG_FOLDER, LOG_REF_FOLDER, "{}.txt".format(name+image_name))
	output_path = os.path.join(OUTPUT_FOLDER, OUTPUT_REF_FOLDER, name+image_name)
	command = "-t {} -p {} -o {} -f {} {}".format(THRESHOLD, PHASES, output_path, log_path)
	return command


def run_test(name):
	log_path = os.path.join(LOG_FOLDER, LOG_SUB_FOLDER, "{}.txt".format(name+image_name))
	output_path = os.path.join(OUTPUT_FOLDER, OUTPUT_SUB_FOLDER, name+image_name)
	command = "-t {} -p {} -o {} -f {}".format(THRESHOLD, PHASES, output_path, log_path)
	return command

def compare_transformation_diff(name, image_name):
	ref_path = os.path.join(LOG_FOLDER, LOG_REF_FOLDER, "{}.txt".format(name+image_name))
	log_path = os.path.join(LOG_FOLDER, LOG_SUB_FOLDER, "{}.txt".format(name+image_name))
	command = "diff -u {} {}".format(ref_path, log_path)
	output = subprocess.checkout(command.split(' '))
	result = output.decode("utf-8")
	print(result)