import os
import platform

env = Environment()
env['SYSTEM'] = platform.system().lower()

if env['SYSTEM'] in ['linux', 'darwin']:
    env.Append(CCFLAGS=["-fopenmp"])
    env.Append(LINKFLAGS=['-fopenmp'])

# directory structure
SCRD = 'src'
INCLD = 'include'
BUILDD = 'build'

env.Append(CPPPATH=[INCLD])
env.VariantDir(variant_dir=BUILDD, src_dir=SCRD, duplicate=0)
env['OBJDIR'] = BUILDD

src_files = [os.path.join(SCRD, f) for f in os.listdir(SCRD) if f.endswith('.cc')]
src_files.append('main.cc')
source_files = [os.path.join(env['OBJDIR'], f) for f in src_files]

env.Program(target='carbon', source=src_files)
