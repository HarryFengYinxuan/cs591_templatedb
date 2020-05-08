# CS 591 A1: Data Systems Architecture - TemplateDB


## About

TemplateDB is a simple template for you, the student, to use for the systems
implementation project in CS 591. Note that this is a simple template, it is
not extensive, rather it is meant to help guide you on what we expect when
you implement the LSM tree. You can use this as base code or start from
scratch.

## Notice

Using ctest sometimes gives segmentation fault on OSX.
I can run the tests with lldb debugger on VS Code.
It sometimes give bad memory access error. However, 
since the tests are deterministic, it seems to due to
something other than my code. Usually the error happens
when the B+ tree tries to access a child that it has 
pointer to, but the child is already released. Running
the test multiple times would solve the problem. The 
real reason to the error remains unknown at this time.


## Requirements

You will need the following on your system (or alternatively develop on the
CSA machines)

    1. CMake
    2. C++ Compiler
    3. GoogleTest (Auto compiled and fetched with CMake)



## Usage

To compile, first create a build directory.


```bash
mkdir build
cd build
```

Afterwards, build using cmake.


```bash
cmake ..
cmake --build .
```

An example executable should be located in the `build/example` folder. The
benchmark simply takes in two files, a data file and a workload file and
measures the time it takes to execute the workload over the dataset. Use the
`-h` flag to see the proper syntax.

Additionally we have provided some examples of unit test in C++ using gtest.
This source is located in the `tests/basic_test.cpp`, whith the executable
will be located in `build/tests` directory. We highly recommend when building
your system to continue to expand on unit test. If you want to run all test,
you may use the following command while you are in the build directory.

```bash
ctest
```

Both the basic test and persistence test will go through.


## Building Workloads and Datasets

In the `tools` folder we have included two scripts `gen_data.py` and
`gen_workload.py`. They generate random datasets and workloads respectively.
By default they have a maximum range of values that can be randomly
generated, I assume everyone knows some python and can edit the scripts to
increase the range if needed. Generate workloads and data with the following
syntax

```bash
gen_data.py <rows> <dim_per_value> <folder>
gen_workload.py <rows> <dim_per_value> <max_key> <folder>
```

Data is generated with commas separating each item

Number of Keys | Dimensions of each Object
---------------|--------------------------
Key 1 | Value 1
Key 2 | Value 2
... | ...
Key N | Value N

While workloads follow the format of 

```
OPERATOR,KEY,ARGS
```

with the first line being the number of total operations.

## Contact

If you have any questions please feel free to see Andy in office hours, or
email me at ndhuynh@bu.edu.
