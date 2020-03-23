# Cascade of Filters Solver
This software is a solver to choose the best configuration of FIR filters to
perform a cascade of filters. Assuming a set of filters has been characterized and a cost
function has been computed for each filter (number of coefficients and number of bits
describing each coefficient), then the software will identify the optimum set of filters meeting a target, either of performance or resource usage.

## Dependency
- [Gurobi](https://www.gurobi.com/) v8.0.1

## Coefficients file structure
To add your coefficients set, your file must be in binary format with:
- unsigned int 16 bits for the number of bits
- unsigned int 16 bits for the number of coefficients
- floating point double precision for the rejection

We provide the GNU Octave scripts to generate our filters coefficients in [tools/](https://github.com/oscimp/cascade_filters_solver/tree/master/tools).

## CMake option
- LP_FIX_REJECTION_CONSTRAINT: Fix the rejection constraints (see Notes).

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
./fir-solver --max-rej 5 500 ../fir_data/filters.json example
```
Will produce the results into example folder for 5 stages of filters with 500 a.u. of area.


```
# To minimize area
# ./fir-solver NUMBER_STAGE REJECTION_MIN FILTERS_JSON OUTPUT_FORMAT
./fir-solver --min-area  5 80 ../fir_data/filters.json example
```
Will be produce the results into example folder for 5 stages of filters with 80 dB of rejection.

## Notes
- Our solver can be produce some wrong
results when the optimal number of stages is lower than the limits. Just take the best
previous solution.
You can also compile our project with LP_FIX_REJECTION_CONSTRAINT option but the solver will
be twice slower.
