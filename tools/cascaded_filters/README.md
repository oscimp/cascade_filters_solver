Simulate multiple cascaded filters
==================================

The purpose of this program is to simulate a cascade of filters.
To do this, it takes as a parameter a binary file containing the raw data and the different filters to be applied.

## Example
```sh
./cascaded-filters data_prn.bin simu_stage.bin filters/firls/firls_003_int03 15 4 filters/firls/firls_035_int11 0 15 filters/firls/firls_019_int07 0 22
```

The first parameter is the binary file with the raw data (a sample file could be found [here](https://github.com/oscimp/cascade_filters_solver/tree/master/tools/cascaded_filters/data_prn.bin)).

The second parameter is the name of ouput binary file which contains the result of simulation.

The next parameters indicate the composition of cascade filters: the coefficient filter, the output data size after the filter and the number of bit shifted.

To generate all filter files, you can execute the Octave script located in [filters/](https://github.com/oscimp/cascade_filters_solver/tree/master/tools/cascaded_filters/filters/generate_filters.m).

In order to work with our cascade filters solver, you need to follow some steps:
```sh
cd $PATH_TO_PROJECT/build
ln -s ../tools/cascaded_filters/filters .
ln -s ../tools/cascaded_filters/plot_data.m .
```

After, you can execute directly the commande provide by the solver to run the simulation.
