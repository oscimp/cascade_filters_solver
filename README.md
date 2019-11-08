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

## Tags
- max_rej_fixe_area_v2: Stable version to maximize rejection for a limited area
- max_rej_fixe_area_v3: Experimental version to maximize rejection for a limited area (fix some bugs but twice as long, see Notes)
- min_area_fixe_rej_v2: Stable version to minimize area for a minimal rejection level
- min_area_fixe_rej_v3: Experimental version to minimize area for a minimal rejection level (fix some bugs but twice as long, see Notes)

## Compilation
```sh
mkdir build
cd build
cmake ..
make
```

## Execution
```
# To maximize rejection
# ./fir-solver NUMBER_STAGE AREA_MAX FIRLS_DATA FIR1_DATA OUTPUT_FORMAT
./fir-solver 5 500 ../fir_data/firls_2_22_bits_3_2_60_coeffs_jmf.bin ../fir_data/fir1_2_18_bits_3_60_coeffs_jmf.bin example
```
Will produce the results into example folder for 5 stages of filters with 500 a.u. of area.


```
# To minimize area
# ./fir-solver NUMBER_STAGE REJECTION_MIN FIRLS_DATA FIR1_DATA OUTPUT_FORMAT
./fir-solver 5 80 ../fir_data/firls_2_22_bits_3_2_60_coeffs_jmf.bin ../fir_data/fir1_2_18_bits_3_60_coeffs_jmf.bin example
```
Will be produce the results into example folder for 5 stages of filters with 80 dB of rejection.

## Notes
- The tags max_rej_fixe_area_v2 and min_area_fixe_rej_v2 can be produce some wrong
results when the optimal number of stages is lower than the limits. Just take the best
previous solution.
