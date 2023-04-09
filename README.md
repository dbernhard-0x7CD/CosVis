
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
 * create filter which scales the points to their actual location (via cosmological scale factor 'a') [paraview]
 * create histogram for the types of particles [paraview]
 * create filter which takes data from UI and filters particles based on their type [CPP]
    * first create particle filter in CPP
 
# DONE
 * create slider to select timestep [CPP]
 * create filter which takes data from UI and filters particles based on their type [paraview]
 * create filter to compute temperature [paraview]

