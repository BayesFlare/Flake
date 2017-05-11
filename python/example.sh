#!/bin/bash

# an example running create_data.py and run_flake.py

# create a signal injection JSON file
injdata=$'{
  \"datafile\": \"inj.txt\",
  \"starttime\": 0.0,
  \"timestep\": 0.0208333,
  \"duration\": 240,
  \"backgroundoffset\": 3.0,
  \"noise\": 0.52,
  \"sinusoids\": [{\"amplitude\": 4.3,
                   \"phase\": 0.5,
                   \"period\": 1.34},
                  {\"amplitude\": 3.1,
                   \"phase\": 3.5,
                   \"period\": 0.345}],
  \"flares\": [{\"amplitude\": 4.5,
                \"time\": 1.01,
                \"risetime\": 0.0231,
                \"decaytime\": 0.0410}]
}'


injfile=inj.json
echo "$injdata" > $injfile

# create signal file
./create_data.py $injfile

# create configuration file
cfgdata=$'{
  \"SinusoidModel\": {
    \"MaxSinusoids\": 5
  },
  \"FlareModel\":{
    \"MaxFlares\": 5
  },
  \"Changepoints\":{
    \"MaxChangepoints\": 0
  },
  \"Impulses\":{
    \"MaxImpulses\": 0
  }
}'

cfgfile=config.json
echo "$cfgdata" > $cfgfile

numpost=100 # number of posterior samples to stop at
flakeexec=/home/matthew/repositories/Flake/flake
rundir=`pwd`
outname=${rundir}/example.png

./run_flake.py -e $flakeexec -r $rundir -c $cfgfile -p $numpost -i $injfile inj.txt --time-it --output-plot $outname --cluster