AddOption('--debug-mode', dest='debug', action='store_true', default=False,
          help='Install with minimal optimisation and GDB debugging enabled') 

env = Environment()

# set the compiler
env['CC'] = 'g++'

# set the compiler flags
if GetOption('debug'):
  env.Append(CCFLAGS=['-std=c++11', '-O0', '-Wall', '-Wextra', '-pedantic', '-g'])
else:
  env.Append(CCFLAGS=['-std=c++11', '-O3', '-Wall', '-Wextra', '-pedantic', '-m64', '-ffast-math', '-fno-finite-math-only', '-flto', '-march=native', '-funroll-loops'])

# set potential library paths
env.Append(LIBPATH=['/usr/lib', '/usr/local/lib', '/usr/lib/x86_64-linux-gnu'])

# set potential include paths
env.Append(CPPPATH=['/usr/include/', '/usr/local/include/'])

conf = Configure(env)

# check for DNest4
if not conf.CheckLib('dnest4'):
  print("Error... could not find the DNest4 library")
  Exit(1)

#if not conf.CheckHeader('dnest4/DNest4.h', language='c++'):
#  print("Error... could not find the DNest4.h header")
#  Exit(1)

#if not conf.CheckLibWithHeader('dnest4', 'dnest4/DNest4.h', 'c++', include_quotes='<>'):
#  print("Error... could not find the DNest4 library")
#  Exit(1)

# check for CFITSIO library
if not conf.CheckLibWithHeader('cfitsio', 'fitsio.h', 'c'):
  print("Error... could not find CFITSIO library")
  Exit(1)

# check for CCFits library
if not conf.CheckLibWithHeader('CCfits', 'CCfits/CCfits.h', 'c++'):
  print("Error... could not find CCfits library")
  Exit(1)

# check for ZLIB
if not conf.CheckLib('z'):
  print("Error... could not find ZLIB")
  Exit(1)

# check for BOOST
if not conf.CheckLib('boost_thread'):
  print("Error... could not find boost_thread library")
  Exit(1)

if not conf.CheckLib('boost_system'):
  print("Error... could not find boost_system library")
  Exit(1)

if not conf.CheckLib('boost_iostreams'):
  print("Error... could not find boost_system library")
  Exit(1)

env = conf.Finish()

# build flake executable
env.Program('flake', Glob('src/*.cpp'))

