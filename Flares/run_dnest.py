import shutil
import subprocess
import time as tsys
import numpy as np
import copy
import glob
import argparse
import postprocess





def extract_nlevels(filename):

    fsplit = filename.split("_")

    sdata = np.loadtxt("%s_%s_samples.txt"%(fsplit[0], fsplit[1]))

    nlevels = np.shape(sdata)[0]

    return nlevels


def find_weights(p_samples):

    print("max(p_samples): %f" %np.max(p_samples[-10:]))

    ### NOTE: logx_samples runs from 0 to -120, but I'm interested in the values of p_samples near the
    ### smallest values of X, so I need to look at the end of the list
    if np.max(p_samples[-10:]) < 1.0e-5:
        print("Returning True")
        return True
    else:
        print("Returning False")
        return False


def run_burst(filename, dnest_dir = "./", levelfilename=None, nsims=100):


    ### first run: set levels to 200

    ## run DNest
    dnest_process = subprocess.Popen(["nice", "-19", "./flake", "-t", "4", "-d", filename])

    endflag = False
    while endflag is False:
        try:
            tsys.sleep(60)
            logz_estimate, H_estimate, logx_samples, p_samples = postprocess.postprocess()
            if p_samples is None:
                endflag = False
            else:
                endflag = find_weights(p_samples)
                print("Endflag: " + str(endflag))

        except (KeyboardInterrupt, ValueError):
            break
    dnest_process.kill()
    postprocess.postprocess(plot=True)
    return


def run_all_bursts(data_dir="./", dnest_dir="./", levelfilename="test_levels.dat"):

    print("I am in run_all_bursts")
    print ("%s*_llc.fits"%data_dir)
    filenames = glob.glob("%s*_llc.fits"%data_dir)
    print(filenames)

    levelfilename = data_dir+levelfilename
    print("Saving levels in file %s"%levelfilename)

    levelfile = open(levelfilename, "w")
    levelfile.write("# data filename \t number of levels \n")
    levelfile.close()

    for f in filenames:
        print("Running on burst %s" %f)
        run_burst(f, dnest_dir=dnest_dir, levelfilename=levelfilename)

    return


def main():
    print("I am in main")
    run_all_bursts(data_dir, dnest_dir, levelfilename)
    return


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Running DNest on a number of bursts")

    parser.add_argument("-d", "--datadir", action="store", required=False, dest="data_dir",
                        default="./", help="Specify directory with data files (default: current directory)")
    parser.add_argument("-n", "--dnestdir", action="store", required=False, dest="dnest_dir",
                        default="./", help="Specify directory with DNest model implementation "
                                           "(default: current directory")
    parser.add_argument("-f", "--filename", action="store", required=False, dest="filename",
                        default="test_levels.dat", help="Define filename for file that saves the number of levels to use")

    clargs = parser.parse_args()
    data_dir = clargs.data_dir
    dnest_dir = clargs.dnest_dir
    levelfilename = clargs.filename

    main()

