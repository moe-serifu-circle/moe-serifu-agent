#!/usr/bin/python

#################################################################
# gendeps.py - Generates dependency graph for inclusion in the Makefile.
#
# Execute this script when the dependencies have changed in order to rebuild
# recipes for inclusion in the master Makefile.
#
# Ensure that this script is executed regularly to keep the Makefile
# up-to-date.
#
# Include paths are given in gcc-syntax (-Ipath/to/include). These are the
# locations where this script will search for included files.
# The first non-option argument is the name of the variable for the source
# directory of modules.
# The second non-option argument is the source tree being scanned. Only files
# in this directory will be included in the recipes dep list, and this is
# by default included in the include paths.
# All other non-option arguments are the modules that dependency graphs are
# to be generated for; give the paths to their implementation files relative
# to the source directory
#
# Ex: ~$ ./gendeps.sh SDIR src -Icompat agent.cpp cmd.cpp event/dispatch.cpp
#
# All generated recipies will be of the following form:
#
# $(ODIR)/modulepath.o: $(srcvar)/modulepath $(srcdir)/dep1path $(srcdir)/dep2path ... $(srcdir)/depNpath
#	$(CXX) -c -o $@ $(srcvar)/modulepath $(CXXFLAGS)
#
# This script will only analyze double-quote includes; angle bracket includes
# are considered to be for external libraries and will be ignored.
#################################################################

import sys
import os.path
import re

def validate_args(args):
	srcvar_set = False
	module_set = False
	srcdir_set = False

	for n in args:
		if n[0] != "-":
			if srcvar_set:
				if srcdir_set:
					module_set = True
					break
				else:
					srcdir_set = True
			else:
				srcvar_set = True
	return srcvar_set and module_set and srcdir_set

import pprint

def main():
	if not validate_args(sys.argv[1:]):
		sys.stderr.write("usage:  " + sys.argv[0] + " [options] srcvar srcdir mod1 [mod2 .. modN]\n")
	srcvar, srcdir, modules, include_paths = parse_args(sys.argv[1:])
	if srcvar is None:
		return
	state = {'srcvar': srcvar, 'srcdir': srcdir, 'include_paths': include_paths}
	for mod in modules:
		deps = scan_deps(state, os.path.join(state['srcdir'], mod), mod)
		pprint.pprint(deps)
	
def parse_args(args):
	include_paths = []
	modules = []
	srcvar = None
	srcdir = None
	for n in args:
		if n[0] == "-":
			if n[1] == "I":
				if len(n) < 3:
					sys.stderr.write("-I options must give path\n")
					return (None, None, None, None)
				if os.path.isdir(n[2:]):
					include_paths.append(n[2:])
				else:
					sys.stderr.write("skipping include path '" + n[2:] + "': not a readable dir\n")
			else:
				sys.stderr.write("Unknown option '" + n + "'\n")
				return (None, None, None, None)
		else:
			if srcvar is None:
				srcvar = n
			elif srcdir is None:
				if not os.path.isdir(n):
					sys.stderr.write("'" + n + "': not a directory. Abort.\n")
					return (None, None, None, None)
				srcdir = n
				include_paths.append(n)
			else:
				if os.path.isfile(os.path.join(srcdir, n)):
					modules.append(n)
				else:
					sys.stderr.write("skipping module '" + n + "' in '" + srcdir + "': not a readable file\n")
	if len(modules) < 1:
		sys.stderr.write("No valid modules given. Abort.\n")
		return (None, None, None, None)
	return (srcvar, srcdir, modules, include_paths)
	
def find_includes(path):
	print(path)
	incs = []
	f = open(path, 'r')
	for line in f:
		m = re.search(r'^\s*#include\s+"([^"]+)"\s*$', line)
		if m:
			incs.append(m.group(1))
	f.close()
	return incs
	
def scan_deps(state, scan_file, dep_file, deps=None, visited=None):
	if deps is None:
		deps = []
	if visited is None:
		visited = []
	if dep_file is not None:
		deps.append(os.path.join('$(' + state['srcvar'] + ')', dep_file))
	visited.append(scan_file)
	includes = find_includes(scan_file)
	for inc in includes:
		#search for the file in the include paths
		inc_file = None
		inc_dep = None
		for dir_path in state['include_paths']:
			if os.path.isfile(os.path.join(dir_path, inc)):
				inc_file = os.path.join(dir_path, inc)
				if dir_path == state['srcdir']:
					inc_dep = inc
		if inc_file is None:
			sys.stderr.write("cannot locate file '" + inc + "' included from '" + scan_file + "'; skipping\n")
		else:
			if inc_file not in visited:
				scan_deps(state, inc_file, inc_dep, deps, visited)
	return deps

main()

