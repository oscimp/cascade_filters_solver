# Cascade of Filters Solver
This software is a solver to choose the best configuration of FIR filters to
perform a cascade of filters. Assuming a set of filters has been characterized and a cost
function has been computed for each filter (number of coefficients and number of bits
describing each coefficient), then the software will identify the optimum set of filters meeting a target, either of performance or resource usage.

## Dependency
- [Digital Signal Processing Simulator](https://github.com/oscimp/libdsps) *Only for build tools*
- [Gurobi](https://www.gurobi.com/) v8.0.1 & 9.0.1 (Download and Licences: Gurobi Optimizer)

As described at https://www.gurobi.com/documentation/8.1/quickstart_linux/software_installation_guid.html, define the appropriate environment variables:

```sh
export GUROBI_HOME=/somewhere/gurobi901/linux64
export PATH=$PATH:$GUROBI_HOME/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$GUROBI_HOME/lib
```

then compile manually the static library

```sh
cd $GUROBI_HOME/src/build
make
cp libgurobi_c++.a ../../lib
```

## Coefficients file structure
To add your coefficients set, your file must be in binary format with:
- unsigned int 16 bits for the number of bits
- unsigned int 16 bits for the number of coefficients
- floating point double precision for the rejection

We provide the GNU Octave scripts to generate our filters coefficients in [tools/](https://github.com/oscimp/cascade_filters_solver/tree/master/tools).

## CMake option
- LP_FIX_REJECTION_CONSTRAINT: Fix the rejection constraints (see Notes).
- LP_TOOLS: Compile the tool to simulate a cascaded filter

## Compilation
```sh
git clone https://github.com/oscimp/cascade_filters_solver.git
git submodule update --init
mkdir build
cd build
cmake ..
make
```

## Execution
```
# To maximize rejection
# ./fir-solver NUMBER_STAGE AREA_MAX FILTERS_JSON OUTPUT_FORMAT
./fir-solver --max_rej 3 500 ../fir_data/filters.json example
```
Will produce the results into example folder for 3 stages of filters with 500 a.u. of area.


```
# To minimize area
# ./fir-solver NUMBER_STAGE REJECTION_MIN FILTERS_JSON OUTPUT_FORMAT
./fir-solver --min_area 3 80 ../fir_data/filters.json example
```
Will be produce the results into example folder for 3 stages of filters with 80 dB of rejection.

The resuting files are
```sh
example.m example.sh example.tcl gurobi.lp sol.txt
```
with most significantly the Vivado tcl script for synthesizing the resulting FIR cascade using the
OscimpDigital tools, the GNU/Octave file for simulating the filter cascade behaviour, and
the bash script for automating the synthesis and running the resulting bitstream on a 14-bit
Red Pitaya.

## Notes
- The solver can produce some pessimistic result when the optimal number of stages is lower
than the upper limit of considered stages. In such a case, take the best previous solution.
You can also compile our project with LP_FIX_REJECTION_CONSTRAINT option but the solver will
be twice slower.
