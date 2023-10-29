import platform

env = Environment()
env['SYSTEM'] = platform.system().lower()

if env['SYSTEM'] in ['linux', 'darwin']:
    env.Append(CCFLAGS=["-fopenmp"])
    env.Append(LINKFLAGS=['-fopenmp'])

env.Program(['render.cc', 'carbon.h'])
