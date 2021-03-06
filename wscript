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
	output1 = self.path.find_or_declare('box86stub/src/wrapped/wrappedwinapi_private.h')
	tsk1 = self.create_task('generate_wrapped', self.sources, output1)
	tsk2 = self.create_task('rebuild_wrappers', output1,
		[self.path.find_or_declare('box86stub/src/wrapped/generated/wrapper.c'),
		self.path.find_or_declare('box86stub/src/wrapped/generated/wrapper.h')])

def options(opt):
	opt.load('gas')
	opt.add_option('--enable-dllemu', action = 'store_true', dest = 'DLLEMU', default = False, help = 'enable dll loader')
	opt.add_option('--enable-x86emu', action = 'store_true', dest = 'X86EMU', default = False, help = 'enable box86 x86emu')
	opt.add_option('--enable-dynarec', action = 'store_true', dest = 'DYNAREC', default = False, help = 'enable box86 dynarec')


def configure(conf):
	conf.load('gas')
	conf.define_cond('X86EMU',conf.options.X86EMU)
	conf.define_cond('DYNAREC',conf.options.DYNAREC)

	conf.env.X86EMU = conf.options.X86EMU
	conf.env.DLLEMU = conf.options.DLLEMU
	conf.env.DYNAREC = conf.options.DYNAREC

	if not conf.env.DLLEMU:
		return
	if conf.env.DEST_SIZEOF_VOID_P != 4:
		conf.fatal('dllemu cannot be enabled on non-32bit platform')

	if conf.check_endianness() != 'little':
		conf.fatal('dllemu cannot be enabled on big endian system')

	if not conf.options.X86EMU and conf.env.DEST_CPU not in ['i486','i686','x86','x86_64']:
		conf.fatal('dllemu cannot be enabled on non-x86 without x86emu')

	if conf.options.DYNAREC:
		if conf.env.DEST_CPU == 'arm':
			conf.define('ARM',1)
		else:
			conf.fatal('dynarec only supported on arm')

	conf.env.CFLAGS_PTHREAD = []
#	conf.env.CFLAGS= ['-m32', '-O0']


def build(bld):
	if not bld.env.DLLEMU:
		return
	source = bld.path.ant_glob(['hlwrap/*.c', '*.c'])
	libs = ['M', 'DL', 'pthreads']
	includes = [ '.', '../common', '..' ]
	if bld.env.X86EMU:
		source += bld.path.ant_glob(['box86/src/emu/*.c', 'box86/src/tools/bridge.c', 'box86/src/tools/callback.c', 'box86/src/libtools/myalign.c'])
		includes += [ 'box86/src/include', 'box86/src/']

		if bld.env.DYNAREC:
			node = bld.path.find_node('box86/src/dynarec')
			dynarec_src = node.ant_glob(['arm_*.c', 'arm_*.S', 'dynablock.c', 'dynarec_arm_functions.c', 'dynarec_arm.c'])
			dynarec_pass = node.ant_glob(['dynarec_arm_helper.c','dynarec_arm_??.c','dynarec_arm_????.c'])
			source += dynarec_src
			for i in range(4):
				name = 'dynarec_pass'+str(i)
				bld.objects(source=dynarec_pass,target=name,name=name,features='c',includes=includes,cflags='-w -fPIC -DSTEP='+str(i), use='dynarec')
				libs += [name]

		source += bld.path.ant_glob(['box86/src/dynarec/dynarec.c'])
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
