# HTMBench

Quick Start
============
1. Download this benchmark repository.

1. Download the input package from [here](https://drive.google.com/open?id=1vf_HbWKNQROHI5XIXIAFfHHfkYJYe1yu) and uncompress it.

1. Open the `set_env` file and modify the value of `TSX_INPUT` depending on where you put the input directory.

1. Load the environmental variables and build the RTM library.
   ```
   $ source set_env
   $ cd lib && make
   ```

1. Run the application.
   Use ```launch_applications.py --list``` to see the whole list of applications.
   If you would like to run one application, say intruder, run ```launch_applications.py intruder```.
   Add ```--verbose``` to see what is executed and you may customize it for your own use from here.

Citation
============

If you use this benchmark suite, please cite this paper:

Qingsen Wang, Pengfei Su, Milind Chabbi, and Xu Liu. 2019. Lightweight Hardware Transactional Memory Profiling. In _24th ACMSIGPLAN Symposium on Principles and Practice of Parallel Programming
(PPoPP ’19), February 16–20, 2019, Washington, DC, USA_. ACM, New York, NY, USA, 15 pages. https://doi.org/10.1145/3293883.3295728
