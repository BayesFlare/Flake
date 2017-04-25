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

# create OPTIONS file
options=$'# File containing parameters for flake
# Put comments at the top, or at the end of the line.
5       # Number of particles
10000   # new level interval
10000   # save interval
100     # threadSteps - how many steps each thread should do independently befor
e communication
100     # maximum number of levels
10      # Backtracking scale length (lambda in the paper)
100     # Strength of effect to force histogram to equal push (beta in the paper
)
10000   # Maximum number of saves (0 = infinite)
sample.txt
sample_info.txt
levels.txt'

optionsfile=OPTIONS
echo "$options" > $optionsfile

numpost=50 # number of posterior samples to stop at
flakeexec=/home/matthew/repositories/Flake/flake
rundir=`pwd`

./run_flake.py -e $flakeexec -r $rundir -c $cfgfile -p $numpost -i $injfile inj.txt