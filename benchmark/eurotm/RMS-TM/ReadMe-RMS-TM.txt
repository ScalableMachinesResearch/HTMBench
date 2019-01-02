----------------------------------------------------------------------------
TO DOWNLOAD:
----------------------------------------------------------------------------
Go to: http://www.bscmsrc.eu/software/RMS-TM
Click on "Version 1.0" and download "RMS-TM-1.0.tar.gz"

Go to: http://cucis.ece.northwestern.edu/projects/DMS/MineBench.html
Click on "NU-MineBench-2.0" and download "NU-MineBench-2.0.data1.tar.gz"


----------------------------------------------------------------------------
TO INSTALL:
----------------------------------------------------------------------------

tar -zxvf RMS-TM-1.0.tar.gz
tar -zxvf NU-MineBench-2.0.data1.tar.gz 


----------------------------------------------------------------------------
REQUIREMENT:
----------------------------------------------------------------------------
C/C++ compiler with Transactional Memory support. In our work, Intel® C++ STM Compiler Prototype Edition 2.0 was used.
To download it, go to: http://whatif.intel.com.


----------------------------------------------------------------------------
FILE DIRECTORY DESCRIPTION:
----------------------------------------------------------------------------

RMS-TM-1.0 is a new TM benchmark suite that uses RMS applications. Currently
there are 3 applications in the suite.

Here's the files structure. Lets call $DMHOME as the home for our TM
benchmark suite RMS-TM-1.0. The following is the file organization
within the root directory $DMHOME after uncompressing RMS-TM-1.0.tar.gz file.

* $DMHOME/README.txt - this file in txt format
* $DMHOME/howtocompile/compile.txt - command line how to compile each application.
* $DMHOME/howtoexecute/execute.txt - sample of the command line for executing each application.
Currently the sample commands use the default (most standard) parameters.
The command lines can similary be extended to other options. $DMHOME is used as the home
directory for TM applications.


* $DMHOME/src/MineBench - containing all the source files of the applications which were taken 
from MineBench benchmark suite.

* $DMHOME/datasets - move "datasets" directory in uncompressed "NU-MineBench-2.0.data1.tar.gz" to $DMHOME/
                         
Within src/MineBench, the following subdirectories exist:

* Apriori - Apriori based association rule application (uses horizontal database)

* ScalParC - A decision tree based classification application.

* Utility-Mine - Utility based association rule application.
 
