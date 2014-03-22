#!/usr/bin/python

import os,sys,glob

runlist = glob.glob('Run_??????.root')
for run in runlist :
    cmd = "./runKPlotter.sh "+str(run[4:10]).lstrip('0')+"; "
    print str(cmd)
    os.system(cmd)
