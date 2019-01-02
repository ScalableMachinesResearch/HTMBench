#!/usr/bin/env python2
import ConfigParser
import os
import shlex
import subprocess
import sys
from optparse import OptionParser

g_vars = {}


def log_print(s):
	if g_vars["verbose"]:
		print(s)

def execute_command(command, shell_flag=False):
        log_print("EXECUTE: {}".format(command))
	if g_vars["dry_run"]:
		return "", "", 0
	child = subprocess.Popen(shlex.split(command), shell=shell_flag,stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	out,err = child.communicate()
	return out, err, child.returncode


def parse_arguments():
        global g_vars
        usage = "usage: %prog [options] [run_file,default.run]"
        parser = OptionParser(usage)
        parser.add_option("--show-output", action="store_true", dest="show_output", default=False, help="Whether to show the launched program's output")
        parser.add_option("--verbose", action="store_true", dest="verbose", default=False, help="Print every command executed")
	parser.add_option("-t", type="int", dest="num_threads", default=1, help="The number of threads to launch the program if configurable (not applicable to PARSEC)")
	parser.add_option("--dry-run", action="store_true", dest="dry_run", default=False, help="Force dry run")

	(options, args) =parser.parse_args()
	if len(args) < 1:
		g_vars["run_file"] = "default.run"
	else:
		g_vars["run_file"] = args[0]

	g_vars["show_output"] = options.show_output
	g_vars["verbose"] = options.verbose or options.dry_run
	g_vars["num_threads"] = options.num_threads
	g_vars["dry_run"] = options.dry_run

def strip_quotes(s):
	if s.startswith('"') and s.endswith('"'):
		s = s[1:-1]
	return s

def fill_environment_variables(s):
	'''Replace the variables within $$'''
	segments = s.split("$")
	assert len(segments)%2 == 1
	for i in [ 2*x+1 for x in range(0, (len(segments)-1)/2)]:
		assert segments[i] in os.environ
		segments[i] = os.environ[segments[i]]
	return "".join(segments)

def parse_run_file():
	config = ConfigParser.ConfigParser()
	config.optionxform = str
	config.read(g_vars["run_file"])

	if config.has_section("environment"):
		g_vars["environ_list"] = config.items('environment')
	
	g_vars["exe"] = fill_environment_variables(config.get("information", "exe"))
	if config.has_option("information", "launch_script"):
		g_vars["launch_script"] = fill_environment_variables(config.get("information", "launch_script"))
		g_vars["launch_script"] = strip_quotes(g_vars["launch_script"])
		g_vars["use_script"] = True
	else:
		g_vars["use_script"] = False

	g_vars["app_shown_name"] = config.get("information", "shown_name")
	g_vars["args"] = fill_environment_variables(strip_quotes(config.get("information", "arguments")))

	
def run(cmd):
	out, err, _ = execute_command(cmd)
	if g_vars["show_output"]:
		print(out)
		print(err)


def main():
	parse_arguments()

	# set environmental variables
	os.environ["THREADS"] = str(g_vars["num_threads"])
	os.environ['OMP_WAIT_POLICY'] = "ACTIVE"
	os.environ['OMP_NUM_THREADS'] = str(g_vars["num_threads"])

	os.environ["HTM_TRETRY"] = "5"
	os.environ["HTM_PRETRY"] = "1"
	os.environ["HTM_GRETRY"] = "5"

	parse_run_file()

	if "environ_list" in g_vars:
		for (key, value) in g_vars["environ_list"]:
			os.environ[key] = fill_environment_variables(value)

	# construct running cmd
        if g_vars["use_script"]:
		cmd = g_vars["launch_script"]
        else:
                cmd = g_vars["exe"]

        if not os.path.isabs(cmd):
		cmd = os.path.join(os.getcwd(),cmd)
	if not g_vars["use_script"]:
		cmd = cmd+ " "+g_vars["args"]

	# run
	run(cmd)


main()
