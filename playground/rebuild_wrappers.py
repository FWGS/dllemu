#!/usr/bin/env python

import os
import glob
import sys

values = ['E', 'e', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', "O", "S"]
def splitchar(s):
	ret = [len(s)]
	i = 0
	for c in s:
		i = i + 1
		if i == 2:
			continue
		ret.append(values.index(c))
	return ret

def value(define):
	return define[9:-1] if define.startswith("!") else define[8:-1]

def splitdef(dnf, defines):
	cunjs = dnf.split(" || ")
	clauses = [c.split(" && ") for c in cunjs]
	
	ret = [len(cunjs)]
	
	for cunj in clauses:
		for c in cunj:
			ret.append(len(c))
	for cunj in clauses:
		for c in cunj:
			ret.append(defines.index(value(c)) * 2 + (1 if c.startswith("!") else 0))
	ret.append(0)
	return ret

def invert(define):
	return define[1:] if define.startswith("!") else ("!" + define)

def main(root, defines, ver):
	# Initialize variables: gbl for all values, vals for file-per-file values, redirects for redirections
	gbl = {}
	vals = {}
	redirects = {}
	
	# First read the files inside the headers
	for filepath in glob.glob(os.path.join(root, "src/wrapped/", "wrapped*_private.h")):
		filename = filepath.split("/")[-1]
		locval = {}
		dependants = []
		with open(filepath, 'r') as file:
			for line in file:
				ln = line.strip()
				# If the line is a `#' line (#ifdef LD80BITS/#ifndef LD80BITS/header)
				if ln.startswith("#"):
					preproc_cmd = ln[1:].strip()
					try:
						if preproc_cmd.startswith("if defined(GO)"):
							continue #if defined(GO) && defined(GOM)...
						elif preproc_cmd.startswith("if !(defined(GO)"):
							continue #if !(defined(GO) && defined(GOM)...)
						elif preproc_cmd.startswith("error"):
							continue #error meh!
						elif preproc_cmd.startswith("endif"):
							if dependants != []:
								dependants.pop()
						elif preproc_cmd.startswith("ifdef"):
							if preproc_cmd[5:].strip() not in defines:
								raise KeyError(preproc_cmd[5:].strip())
							dependants.append("defined(" + preproc_cmd[5:].strip() + ")")
						elif preproc_cmd.startswith("ifndef"):
							if preproc_cmd[5:].strip() not in defines:
								raise KeyError(preproc_cmd[5:].strip())
							dependants.append("!defined(" + preproc_cmd[6:].strip() + ")")
						elif preproc_cmd.startswith("else"):
							dependants[-1] = invert(dependants[-1])
						else:
							raise NotImplementedError("Unknown preprocessor directive: {0} ({1}:{2})".format(
								preproc_cmd.split(" ")[0], filename, line[:-1]
							))
					except KeyError as k:
						raise NotImplementedError("Unknown key: {0} ({1}:{2})".format(
							k.args[0], filename, line[:-1]
						), k)
				# If the line is a `GO...' line (GO/GOM/GO2/...)...
				elif ln.startswith("GO"):
					print(ln)
					# ... then look at the second parameter of the line
					ln = ln.split(",")[1].split(")")[0].strip()
					
					if ln[1] not in ["F", "S"]:
						raise NotImplementedError("Bad middle letter {0} ({1}:{2})".format(ln[1], filename, line[:-1]))
#					if ln[1] == 'S':
#						ln = ln[0] + 'Z' + ln[2:]
					if any(c not in values for c in ln[2:]) or (('v' in ln[2:]) and (len(ln) > 3)):
						old = ln
						# This needs more work
						acceptables = ['v', 'o', '0', '1'] + values
						if any(c not in acceptables for c in ln[2:]):
							raise NotImplementedError("{0} ({1}:{2})".format(ln[2:], filename, line[:-1]))
						# Ok, this is acceptable: there is 0, 1, stdout and void
						ln = (ln
							.replace("v", "")   # void   -> nothing
							.replace("o", "p")  # stdout -> pointer
							.replace("0", "p")  # 0      -> pointer
							.replace("1", "i")) # 1      -> integer
						redirects.setdefault(" && ".join(dependants), {})
						redirects[" && ".join(dependants)][old] = ln
					# Simply append the function name if it's not yet existing
					locval.setdefault(" && ".join(dependants), [])
					gbl.setdefault(" && ".join(dependants), [])
					if ln not in locval[" && ".join(dependants)]:
						locval[" && ".join(dependants)].append(ln)
					if ln not in gbl[" && ".join(dependants)]:
						gbl[" && ".join(dependants)].append(ln)
		
		# Sort the file local values and add it to the dictionary
		print(locval)
		for k in locval:
			locval[k].sort(key=splitchar)
		vals[filename] = locval
	
	gbl_vals = {}
	for k in gbl:
		ks = k.split(" && ")
		for v in gbl[k]:
			if k == "":
				gbl_vals[v] = []
				continue
			if v in gbl_vals:
				if gbl_vals[v] == []:
					continue
				for other_key in gbl_vals[v]:
					other_key_vals = other_key.split(" && ")
					for other_key_val in other_key_vals:
						if other_key_val not in ks:
							break
					else:
						break
				else:
					gbl_vals[v].append(k)
			else:
				gbl_vals[v] = [k]
	for v in gbl_vals:
		for k in gbl_vals[v]:
			if " && ".join([invert(v2) for v2 in k.split(" && ")]) in gbl_vals[v]:
				gbl_vals[v] = []
				break
	gbl = {}
	gbl_idxs = []
	for k in gbl_vals:
		if len(gbl_vals[k]) == 1:
			key = gbl_vals[k][0]
		else:
			key = "(" + (") || (".join(gbl_vals[k])) + ")"
		gbl[key] = gbl.get(key, []) + [k]
		if (key not in gbl_idxs) and (key != "()"):
			gbl_idxs.append(key)
	gbl_idxs.sort(key=lambda v: splitdef(v, defines))
	
	redirects_vals = {}
	for k in redirects:
		ks = k.split(" && ")
		for v in redirects[k]:
			if k == "":
				redirects_vals[(v, redirects[k][v])] = []
				continue
			if (v, redirects[k][v]) in redirects_vals:
				if redirects_vals[(v, redirects[k][v])] == []:
					continue
				for other_key in redirects_vals[(v, redirects[k][v])]:
					if other_key == "()":
						break
					other_key_vals = other_key.split(" && ")
					for other_key_val in other_key_vals:
						if other_key_val not in ks:
							break
					else:
						break
				else:
					redirects_vals[(v, redirects[k][v])].append(k)
			else:
				redirects_vals[(v, redirects[k][v])] = [k]
	redirects = {}
	redirects_idxs = []
	for k, v in redirects_vals:
		key = "(" + (") || (".join(redirects_vals[(k, v)])) + ")"
		val = redirects.get(key, {})
		val[k] = v
		redirects[key] = val
		if (key not in redirects_idxs) and (key != "()"):
			redirects_idxs.append(key)
	redirects_idxs.sort(key=lambda v: splitdef(v, defines))
	
	# Sort the table
	for k in gbl:
		gbl[k].sort(key=lambda v: splitchar(v))
	
	def length(s):
		l = len(s)
		if l < 10:
			ret = "0" + str(l)
		else:
			ret = str(l)
		return ret
	
	# Now the files rebuilding part
	# File headers and guards
	files_headers = {
		"wrapper.c": """/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v%s)
 *****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "wrapper.h"
#include "emu/x86emu_private.h"
#include "emu/x87emu_private.h"
#include "regs.h"
#include "x86emu.h"

typedef union ui64_s {
    int64_t     i;
    uint64_t    u;
    uint32_t    d[2];
} ui64_t;

extern void* my__IO_2_1_stderr_;
extern void* my__IO_2_1_stdin_ ;
extern void* my__IO_2_1_stdout_;

static void* io_convert(void* v)
{
	if(!v)
		return v;
	if(v==my__IO_2_1_stderr_)
		return stderr;
	if(v==my__IO_2_1_stdin_)
		return stdin;
	if(v==my__IO_2_1_stdout_)
		return stdout;
	return v;
}

#ifdef USE_FLOAT
#define ST0val ST0.f
#else
#define ST0val ST0.d
#endif
#ifndef NOALIGN
int of_convert(int flag) { return (flag&~0740000) | ((flag&0140000)<<2) | ((flag&0600000)>>2);}
#else
int of_convert(int flag) { return flag; }
#endif
""",
		"wrapper.h": """/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v%s)
 *****************************************************************/
#ifndef __WRAPPER_H_
#define __WRAPPER_H_
#include <stdint.h>

typedef struct x86emu_s x86emu_t;

// the generic wrapper pointer functions
typedef void (*wrapper_t)(x86emu_t* emu, uintptr_t fnc);

// list of defined wrapper
// v = void, i = int32, u = uint32, U/I= (u)int64
// l = signed long, L = unsigned long (long is an int with the size of a pointer)
// p = pointer, P = callback
// f = float, d = double, D = long double, K = fake long double
// V = vaargs, E = current x86emu struct, e = ref to current x86emu struct
// 0 = constant 0, 1 = constant 1
// o = stdout
// C = unsigned byte c = char
// W = unsigned short w = short
// O = libc O_ flags bitfield
// S = _IO_2_1_stdXXX_ pointer (or FILE*)
// Q = ...
// S8 = struct, 8 bytes

"""
	}
	files_guards = {"wrapper.c": """""",
		"wrapper.h": """
#endif //__WRAPPER_H_
"""
	}
	
	# Transform strings into arrays
	for k in gbl:
		gbl[k] = [[c for c in v] for v in gbl[k]]
	
	# Rewrite the wrapper.h file:
	with open(os.path.join(root, "src/wrapped/generated/", "wrapper.h"), 'w') as file:
		file.write(files_headers["wrapper.h"] % ver)
		for v in gbl["()"]:
			file.write("void " + ''.join(v) + "(x86emu_t *emu, uintptr_t fnc);\n")
		for k in gbl_idxs:
			if k != "()":
				file.write("\n#if " + k + "\n")
				for v in gbl[k]:
					file.write("void " + ''.join(v) + "(x86emu_t *emu, uintptr_t fnc);\n")
				file.write("#endif\n")
		file.write("\n")
		for v in redirects["()"]:
			file.write("void " + ''.join(v) + "(x86emu_t *emu, uintptr_t fnc);\n")
		for k in redirects_idxs:
			if k != "()":
				file.write("\n#if " + k + "\n")
				for v in redirects[k]:
					file.write("void " + ''.join(v) + "(x86emu_t *emu, uintptr_t fnc);\n")
				file.write("#endif\n")
		file.write(files_guards["wrapper.h"])
	
	# Rewrite the wrapper.c file:
	with open(os.path.join(root, "src/wrapped/generated/", "wrapper.c"), 'w') as file:
		file.write(files_headers["wrapper.c"] % ver)
		
		# First part: typedefs
		for v in gbl["()"]:
			#         E            e             v       c         w          i          I          C          W           u           U           f        d         D              K         l           L            p        V			O        S
			types = ["x86emu_t*", "x86emu_t**", "void", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*"]
			if len(values) != len(types):
					raise NotImplementedError("len(values) = {lenval} != len(types) = {lentypes}".format(lenval=len(values), lentypes=len(types)))
			
			file.write("typedef " + types[values.index(v[0])] + " (*" + ''.join(v) + "_t)"
				+ "(" + ', '.join(types[values.index(t)] for t in v[2:]) + ");\n")
		for k in gbl_idxs:
			if k != "()":
				file.write("\n#if " + k + "\n")
				for v in gbl[k]:
					#         E            e             v       c         w          i          I          C          W           u           U           f        d         D              K         l           L            p        V			O        S
					types = ["x86emu_t*", "x86emu_t**", "void", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*"]
					if len(values) != len(types):
							raise NotImplementedError("len(values) = {lenval} != len(types) = {lentypes}".format(lenval=len(values), lentypes=len(types)))
					
					file.write("typedef " + types[values.index(v[0])] + " (*" + ''.join(v) + "_t)"
						+ "(" + ', '.join(types[values.index(t)] for t in v[2:]) + ");\n")
				file.write("#endif\n")
		
		file.write("\n")
		# Next part: function definitions
		
		# Helper functions to write the function definitions
		def function_args(args, d=4):
			if len(args) == 0:
				return ""
			if d % 4 != 0:
				raise ValueError("{d} is not a multiple of 4. Did you try passing a V and something else?".format(d=d))
			
			if args[0] == "0":
				return "(void*)(R_ESP + {p}), ".format(p=d) + function_args(args[1:], d + 4)
			elif args[0] == "1":
				return "1, " + function_args(args[1:], d)
			
			arg = [
				"emu, ",                          # E
				"&emu, ",                         # e
				"",                               # v
				"*(int8_t*)(R_ESP + {p}), ",      # c
				"*(int16_t*)(R_ESP + {p}), ",     # w
				"*(int32_t*)(R_ESP + {p}), ",     # i
				"*(int64_t*)(R_ESP + {p}), ",     # I
				"*(uint8_t*)(R_ESP + {p}), ",     # C
				"*(uint16_t*)(R_ESP + {p}), ",    # W
				"*(uint32_t*)(R_ESP + {p}), ",    # u
				"*(uint64_t*)(R_ESP + {p}), ",    # U
				"*(float*)(R_ESP + {p}), ",       # f
				"*(double*)(R_ESP + {p}), ",      # d
				"*(long double*)(R_ESP + {p}), ", # D
				"FromLD((void*)(R_ESP + {p})), ", # K
				"*(intptr_t*)(R_ESP + {p}), ",    # l
				"*(uintptr_t*)(R_ESP + {p}), ",   # L
				"*(void**)(R_ESP + {p}), ",       # p
				"(void*)(R_ESP + {p}), ",         # V
				"of_convert(*(int32_t*)(R_ESP + {p})), ",         # O
				"io_convert(*(void**)(R_ESP + {p})), ",         # S
			]
			#         E  e  v  c  w  i  I  C  W  u  U  f  d  D   K   l  L  p  V  O  S
			deltas = [0, 0, 4, 4, 4, 4, 8, 4, 4, 4, 8, 4, 8, 12, 12, 4, 4, 4, 0, 4, 4]
			if len(values) != len(arg):
				raise NotImplementedError("len(values) = {lenval} != len(arg) = {lenarg}".format(lenval=len(values), lenarg=len(arg)))
			if len(values) != len(deltas):
				raise NotImplementedError("len(values) = {lenval} != len(deltas) = {lendeltas}".format(lenval=len(values), lendeltas=len(deltas)))
			return arg[values.index(args[0])].format(p=d) + function_args(args[1:], d + deltas[values.index(args[0])])

		def function_delta(t, args, d=-4):
			print args
			return ''
			deltas = [0, 0, 4, 4, 4, 4, 8, 4, 4, 4, 8, 4, 8, 12, 12, 4, 4, 4, 0, 4, 4]
			for a in args:
				if a in ['0','1', 'v']: continue
				d += deltas[values.index(a)]
			if d < 0:
				return ''
			return "Pop32(emu);"*(d/4) if t == 'S' else ''

		def function_writer(f, N, W, rettype, args):
			f.write("void {0}(x86emu_t *emu, uintptr_t fcn) {2} {1} fn = ({1})fcn; ".format(N, W, "{"))
			vals = [
				"\n#error Invalid return type: emulator\n",                     # E
				"\n#error Invalid return type: &emulator\n",                    # e
				"fn({0});",                                                     # v
				"R_EAX=fn({0});",                                               # c
				"R_EAX=fn({0});",                                               # w
				"R_EAX=fn({0});",                                               # i
				"ui64_t r; r.i=fn({0}); R_EAX=r.d[0]; R_EDX=r.d[1];",           # I
				"R_EAX=(unsigned char)fn({0});",                                # C
				"R_EAX=(unsigned short)fn({0});",                               # W
				"R_EAX=(uint32_t)fn({0});",                                     # u
				"ui64_t r; r.u=(uint64_t)fn({0}); R_EAX=r.d[0]; R_EDX=r.d[1];", # U
				"float fl=fn({0}); fpu_do_push(emu); ST0val = fl;",             # f
				"double db=fn({0}); fpu_do_push(emu); ST0val = db;",            # d
				"long double ld=fn({0}); fpu_do_push(emu); ST0val = ld;",       # D
				"double db=fn({0}); fpu_do_push(emu); ST0val = db;",            # K
				"R_EAX=(intptr_t)fn({0});",                                     # l
				"R_EAX=(uintptr_t)fn({0});",                                    # L
				"R_EAX=(uintptr_t)fn({0});",                                    # p
				"\n#error Invalid return type: va_list\n",                      # V
				"\n#error Invalid return type: at_flags\n",                     # O
				"\n#error Invalid return type: _io_file*\n",                    # S
			]
			if len(values) != len(vals):
				raise NotImplementedError("len(values) = {lenval} != len(vals) = {lenvals}".format(lenval=len(values), lenvals=len(vals)))
			f.write(vals[values.index(rettype)].format(function_args(args)[:-2]) + function_delta(N[1],args) +" }\n")
		for v in gbl["()"]:
			function_writer(file, ''.join(v), ''.join(v) + "_t", v[0], v[2:])
		for k in gbl_idxs:
			if k != "()":
				file.write("\n#if " + k + "\n")
				for v in gbl[k]:
					function_writer(file, ''.join(v), ''.join(v) + "_t", v[0], v[2:])
				file.write("#endif\n")
		file.write("\n")
		for v in redirects["()"]:
			function_writer(file, v, redirects["()"][v] + "_t", v[0], v[2:])
		for k in redirects_idxs:
			if k != "()":
				file.write("\n#if " + k + "\n")
				for v in redirects[k]:
					function_writer(file, v, redirects[k][v] + "_t", v[0], v[2:])
				file.write("#endif\n")
		
		file.write(files_guards["wrapper.c"])
	
	return 0

if __name__ == '__main__':
	if main(sys.argv[1], sys.argv[2:], "1.1.1.06") != 0:
		exit(2)
	exit(0)
