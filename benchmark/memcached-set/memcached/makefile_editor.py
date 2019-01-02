#!/usr/bin/env python
# This script is used to modified the assignment value in a Makefile. It only applies to the variable assignment level.
# Usage: script [current_makefile] [new_makefile] [sequence_of_assignments]
# Example: script make.old make.new "CFLAGS=-Wall" "LDFLAGS+=-lpthread"

import sys
import re
from string import maketrans

class Assignments:
	def __init__(self, seqs):
		self.variables = []
		self.operations = []
		self.values = []
		for e in seqs:
			oper  = "="
			if e.find("+=") >= 0:
				oper = "+="
			self.operations.append(oper)
			self.variables.append(e.split(oper, 1)[0].strip().rstrip())
			self.values.append(e.split(oper,1)[1])
	def has(self, var):
		if var in self.variables:
			return True
		else:
			return False
	def operation_of(self, var):
		return self.operations[self.variables.index(var)]
	def value_of(self, var):
		return self.values[self.variables.index(var)]
			
assigns = ""

def strok_line(line):
	
	line1 = line.strip().split()
	if len(line1) <= 1:
		return line1
	sObj1 = re.search(r'(\+|\:)*=', line1[0])
	if sObj1 == None:
		sObj2 = re.search(r'(\+|\:)*=', line1[1])
		if sObj2 == None or sObj2.span()[0] != 0:
			return line1
		else:
			tok = sObj2.group(0)
			ret = [ line1[0],tok ]
			ret.append(line1[1][sObj2.span()[1]:])
			if len(line1) > 2:
				ret = ret + line1[2:]
			return ret
	else:
		if sObj1.span()[0] == 0:
			return line1
		index1 = sObj1.span()[0]
		index2 = sObj1.span()[1]
		if index2 == len(line1[0]):
			ret = [line1[0][0:index1] , line1[0][index1:]]
			ret = ret + line1[1:]
			return ret
		else:
			ret = [line1[0][0:index1] , line1[0][index1:index2], line1[0][index2:]]
			ret = ret + line1[1:] 
			return ret



def process_working_lines(working_lines):
	ret = working_lines
	firstline_splits =  strok_line(working_lines[0])
	if len(firstline_splits) < 3:
		return ret
	tok = firstline_splits[0]
	signs = firstline_splits[1] 
	if signs.find("=") >= 0:
		if assigns.has(tok):
			if assigns.operation_of(tok) == "=":
				ret = []
				ret.append(tok + " = " + assigns.value_of(tok))
			elif assigns.operation_of(tok) == "+=":
				ret[0]= tok + signs + " " + assigns.value_of(tok)+" " + ' '.join(firstline_splits[2:])
			else:
				print("ERROR")
	return ret

def main():
	global assigns
	if len(sys.argv) < 4:
		print("Invalid Input")
		print("Usage: "+sys.argv[0] + " [input_file] [output_file] [sequence_of_assignments]")
		return
	assigns = Assignments(sys.argv[3:])

	working_lines = [] # Actually one line, but may be seperated by "\" signs.
	with open(sys.argv[1]) as f1 , open(sys.argv[2],'w') as f2:
		for line in f1:
			line = line.strip("\n")
			if len(line.strip()) <= 0: #empty line
				if len(working_lines) > 0:
					for l in process_working_lines(working_lines):
						f2.write(l+"\n")
					working_lines = []
				f2.write(line+"\n")
				continue;

			working_lines.append(line)
			if line[-1] != '\\':
				for l in process_working_lines(working_lines):
					f2.write(l+"\n")
				working_lines = []
	

main()
