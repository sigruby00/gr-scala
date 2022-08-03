#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir="/home/rubysig00/Documents/gr-scala/python"
export GR_CONF_CONTROLPORT_ON=False
export PATH="/home/rubysig00/Documents/gr-scala/build/python":$PATH
export LD_LIBRARY_PATH="":$LD_LIBRARY_PATH
export PYTHONPATH=/home/rubysig00/Documents/gr-scala/build/swig:$PYTHONPATH
/usr/bin/python3 /home/rubysig00/Documents/gr-scala/python/qa_detector.py 
