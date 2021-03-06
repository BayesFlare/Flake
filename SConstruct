import os

AddOption('--debug-mode', dest='debug', action='store_true', default=False,
          help='Install with minimal optimisation and GDB debugging enabled') 

AddOption('--enable-sse2', dest='sse', action='store_true', default=False,
          help='Install with SSE2 support enabled')

env = Environment()

# set the compiler
env['CC'] = 'g++'

# set the compiler flags
if GetOption('debug'):
  env.Append(CCFLAGS=['-std=c++11', '-O0', '-Wall', '-Wextra', '-pedantic', '-g', '-pthread'])
elif GetOption('sse'):
  env.Append(CCFLAGS=['-std=c++11', '-msse2', '-O3', '-Wall', '-Wextra', '-m64', '-ffast-math', '-fno-finite-math-only', '-flto', '-march=native', '-funroll-loops', '-pthread'])
else:
  env.Append(CCFLAGS=['-std=c++11', '-O3', '-Wall', '-Wextra', '-m64', '-ffast-math', '-fno-finite-math-only', '-flto', '-march=native', '-funroll-loops', '-pthread', '-mno-sse2']) # note the explicit disabling of SSE2 so the __SSE2__ macro does not get set due to default enabling on many machines

# set potential library paths
env.Append(LIBPATH=['/usr/lib', '/usr/local/lib', '/usr/lib/x86_64-linux-gnu'])

# set potential include paths
env.Append(CPPPATH=['/usr/include/', '/usr/local/include/'])

conf = Configure(env)

# check for DNest4
dnest4headers = ['dnest4/DNest4.h', 'dnest4/RJObject/RJObject.h', 'dnest4/RJObject/ConditionalPriors/ConditionalPrior.h', 'dnest4/Distributions/ContinuousDistribution.h']
if not conf.CheckLibWithHeader('dnest4', dnest4headers, 'c++'):
  print("Error... could not find the DNest4 library")
  Exit(1)

# add include paths for headers
incpaths = env['CPPPATH']
extradnest4paths = []
for ip in incpaths:
  for dh in dnest4headers:
    extradnest4paths.append(os.path.join(ip, os.path.dirname(dh)))
env.Append(CPPPATH=extradnest4paths)

# check for CFITSIO library
if not conf.CheckLibWithHeader('cfitsio', 'fitsio.h', 'c'):
  print("Error... could not find CFITSIO library")
  Exit(1)

# check for CCFits library
if not conf.CheckLibWithHeader('CCfits', 'CCfits/CCfits.h', 'c++'):
  print("Error... could not find CCfits library")
  Exit(1)

# check for GSL/CBLAS library
if not conf.CheckLibWithHeader('gsl', 'gsl/gsl_cdf.h', 'c'):
  print("Error... could not find GSL library")
  Exit(1)

if not conf.CheckLib('gslcblas'):
  print("Error... could not find GSLcblas library")
  Exit(1)

# check for pthread
if not conf.CheckLib('pthread'):
  print("Error... could not find pthread library")
  Exit(1)

# check for boost library (for JSON file reading)
if not conf.CheckLib('boost_iostreams'):
  print("Error... could not find boost_iostreams library")
  Exit(1)

if not conf.CheckLib('boost_system'):
  print("Error... could not find boost_system library")
  Exit(1)

if not conf.CheckLib('boost_filesystem'):
  print("Error... could not find boost_filesystem")
  Exit(1)

env = conf.Finish()

# build flake executable
env.Program('flake', Glob('src/*.cpp'))

