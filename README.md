
# Repository for the Visualization of the Cosmology data set


## Setup

* Make sure that the `data/` folder contains the extracted `*.vtp` files
* Make sure that you installed VTK and have cmake

## Compiling

```
mkdir -p build && cd build && cmake .. && cmake --build .
```

# Running

```
./VisCos
```

# TODO
 * try clustering from scikit-learn [paraview] 
    * particle tracer?
* SPH (or alternative)
 * create histogram for the types of particles [paraview]
    * visualize this histogram over time
 * create filter which takes data from UI and filters particles based on their type [CPP]
    * first create particle filter in CPP; Selection: via keypress
 * [maybe drop] create filter which scales the points to their actual location (via cosmological scale factor 'a') [CPP]
    * fix camera position and zoom away as time progresses (scale with a) [CPP]
        * WIP

# TO ASK
* 9th bit is only for dark-matter, this makes no sense
 
# DONE
 * create legend for particle color [CPP]
 * create filter to compute temperature [CPP]
 * create filter which scales the points to their actual location (via cosmological scale factor 'a') [paraview]
 * create slider to select timestep [CPP]
 * create filter which takes data from UI and filters particles based on their type [paraview]
 * create filter to compute temperature [paraview]

