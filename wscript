#! /usr/bin/env python
# encoding: utf-8
# mittorn, 2018

from waflib import Logs
import os

top = '.'

def options(opt):
	opt.add_option('--enable-x86emu', action = 'store_true', dest = 'X86EMU', default = False, help = 'enable box86 x86emu')

def configure(conf):
	conf.define_cond('X86EMU',conf.options.X86EMU)
	conf.define_cond('HLWRAP',conf.options.X86EMU)
	conf.env.X86EMU = conf.options.X86EMU
	conf.env.CFLAGS_PTHREAD = []
#	conf.env.CFLAGS= ['-m32', '-O0']


def build(bld):
	source = bld.path.ant_glob(['*.c'])
	libs = []
	includes = [ '.', '../common', '..' ]
	if bld.env.X86EMU:
		source += bld.path.ant_glob(['hlwrap/*.c', 'box86/src/emu/*.c', 'box86/src/dynarec/dynarec.c', 'box86/src/wrapped/generated/*.c', 'box86/src/tools/bridge.c', 'box86/src/tools/callback.c', 'box86/src/libtools/*.c'])
		includes += [ 'box86/src/include', 'box86/src/', 'box86/src/wrapped/generated',]

	bld.stlib(
		source   = source,
		target   = 'dllemu',
		features = 'c',
		includes = includes,
		use      = libs,
		cflags   = '-w',
		subsystem = bld.env.MSVC_SUBSYSTEM
	)
