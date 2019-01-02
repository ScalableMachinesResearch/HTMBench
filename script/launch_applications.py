#!/usr/bin/env python2
import argparse
import collections
import csv
import os
import shlex
import subprocess

g_vars = {}


def execute_command(command):
	if g_vars["verbose"]:
		print("EXECUTE: {}".format(command))
	child = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	out, err = child.communicate()
	return out,err

def read_info_file(file_path):
	if g_vars["verbose"]:
		print("Start to read {}".format(file_path))
	apps = collections.OrderedDict()
	with open(file_path) as f:
		reader = csv.DictReader(f)
		for row in reader:
			name = row["name"].lower()
			apps[name] = {}
			config = apps[name]
			for field in ["compile_path", "compile_command", "run_path", "run_file"]:
				config[field] = row[field]
	return apps


def run_app(name, config):

	# compile
	working_dir = os.path.join(os.environ["TSX_ROOT"], "benchmark", config["compile_path"])
	if g_vars["verbose"]:
		print("Change working directory to {}".format(working_dir))
	os.chdir(working_dir)
	print(config["compile_command"])
	execute_command(config["compile_command"])
	
	# run
	working_dir = os.path.join(os.environ["TSX_ROOT"], "benchmark", config["run_path"])
	if g_vars["verbose"]:
		print("Change working directory to {}".format(working_dir))
	os.chdir(working_dir)

	cmd = "run-benchmark.py -t {} {} --show-output".format(g_vars["num_threads"], config["run_file"])
	out, err = execute_command(cmd)
	print(out)
	print(err)

def main():
	global g_vars

	parser = argparse.ArgumentParser()
	parser.add_argument("applications", nargs='?', default=None, help="the name list of the applications (separated by ,), or \"all\" for all the available applications")
	parser.add_argument("-l", "--list", action='store_true', help="list all available applications")
	parser.add_argument("-t", "--threads", type=int, default=12, help="the number of threads the launched program will run with (not applicable to PARSEC")
	parser.add_argument("--verbose", action='store_true', help="List every step")
	args = parser.parse_args()

	g_vars["verbose"] = args.verbose
	g_vars["num_threads"] = args.threads

	# read info file
	apps = read_info_file(os.path.join(os.path.dirname(os.path.realpath(__file__)), "app-list.csv"))

	if args.list:
		print("Available applications:\n")
		print("\n".join(apps.keys()))
		return

	if args.applications is None:
		print("Please specify an application or use 'all'")
		return

	if args.applications.lower() == "all":
		names = apps.keys()
	else:
		names = map(lambda x: x.lower(), args.applications.split(","))

	if any([name not in apps.keys() for name in names]):
		print("Please specify an application or use 'all'")
		return


	for name in names:
		run_app(name, apps[name])
		
		
main()
