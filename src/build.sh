#!/bin/bash

# script to run cmake or just recompile
#  - if the command build or rebuild is given this will build the  
#    clean (or create) the build dir and (re)run cmake 
#  - otherwise, it will assume cmake has been run already and
#    just re-make the code

builddir=../build
thisdir=`pwd`

if [ "$#" -gt "0" ]; then
  # re-run cmake and then remake
  if [ "$1" = "rebuild" ] || [ "$1" = "build" ] || [ "$1" = "debug" ]; then
    if [ ! -d "$builddir" ]; then
      mkdir $builddir
      cd $builddir
    else
      cd $builddir
      rm -rf *
    fi
    if [ "$1" = "debug" ]; then
      cmake -DCMAKE_BUILD_TYPE=Debug ..
    else
      cmake ..
    fi
    make
    cd $thisdir
  else
    echo "Use \"build\", \"rebuild\" or \"debug\" as arguments to re-run cmake"
  fi
else
  # just re-run make
  cd $builddir
  make
  cd $thisdir
fi

