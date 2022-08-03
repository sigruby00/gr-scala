/* -*- c++ -*- */

#define SCALA_API

%include "gnuradio.i"           // the common stuff

//load generated python docstrings
%include "scala_swig_doc.i"

%{
#include "scala/detector.h"
%}

%include "scala/detector.h"
GR_SWIG_BLOCK_MAGIC2(scala, detector);
