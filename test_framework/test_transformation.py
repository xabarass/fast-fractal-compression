import os

os.chdir("..")

test_images = range(1, 15)
TEST_IMAGE_FOLDER = "test_images"
TEST_IMAGE_SUB_FOLDER = "benchmark"
THRESHOLD = 100
PHASES = 5
LOG_FOLDER = "test_framework"
LOG_SUB_FOLDER = "standards"
OUTPUT_FOLDER = "test_framework"
OUTPUT_SUB_FOLDER = "reference_images"

for image in test_images:
	image_path = os.path.join(TEST_IMAGE_FOLDER, TEST_IMAGE_SUB_FOLDER, "{}.bmp".format(image))
	log_path = os.path.join(LOG_FOLDER, LOG_SUB_FOLDER, "{}.txt".format(image))
	output_path = os.path.join(OUTPUT_FOLDER, OUTPUT_SUB_FOLDER, "{}.bmp".format(image))
	command = "./bin/fractal-compression -t {} -p {} -o {} -f {} {}".format(THRESHOLD, PHASES, output_path, log_path, image_path)
	os.system(command)