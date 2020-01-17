#! /usr/bin/env python
# encoding: utf-8
# mittorn, 2018

from waflib import Logs, Task, TaskGen
import os

top = '.'


class generate_wrapped(Task.Task):
	def run(self):
		import imp
		self.outputs[0].parent.mkdir()
		generate_wrapped = imp.load_source('generate_wrapped',self.generator.path.find_node('generate_wrapped.py').abspath())
		generate_wrapped.main(open(self.outputs[0].get_bld().abspath(),'w'),[i.abspath() for i in self.inputs])

class rebuild_wrappers(Task.Task):
	def run(self):
		import imp
		self.outputs[0].parent.mkdir()
		rebuild_wrappers = imp.load_source('rebuild_wrappers',self.generator.path.find_node('box86').find_node('rebuild_wrappers.py').abspath())
		rebuild_wrappers.main(self.generator.path.find_or_declare('box86stub').abspath(),[], None)

@TaskGen.feature('generate')
def task(self):
	tsk1 = self.create_task('generate_wrapped')
	tsk1.set_inputs(self.sources)
	output1 = self.path.find_or_declare('box86stub/src/wrapped/wrappedwinapi_private.h')
	tsk1.set_outputs(output1)
	tsk2 = self.create_task('rebuild_wrappers')
	tsk2.set_inputs(output1)
	tsk2.set_outputs([self.path.find_or_declare('box86stub/src/wrapped/generated/wrapper.c'), self.path.find_or_declare('box86stub/src/wrapped/generated/wrapper.h')])

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
		source += bld.path.ant_glob(['hlwrap/*.c', 'box86/src/emu/*.c', 'box86/src/dynarec/dynarec.c', 'box86/src/tools/bridge.c', 'box86/src/tools/callback.c', 'box86/src/libtools/myalign.c'])
		includes += [ 'box86/src/include', 'box86/src/']
		bld(features='generate', sources = bld.path.ant_glob(['hlwrap/*.c','win32.c']))
		node = bld.path.find_or_declare('box86stub/src/wrapped/generated')
		source += [node.find_or_declare('wrapper.c')]
		includes += [node]

	bld.stlib(
		source   = source,
		target   = 'dllemu',
		features = 'c',
		includes = includes,
		use      = libs,
		cflags   = '-w',
		subsystem = bld.env.MSVC_SUBSYSTEM
	)
