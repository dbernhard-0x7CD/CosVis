
# Repository for the Visualization of the Cosmology dataset


# Demo
![Gif](https://github.com/dbernhard-0x7CD/CosVis/blob/main/doc/videos/Collsision.gif?raw=true)

## Setup

* Make sure that the `data/` folder contains the extracted `*.vtp` files
* Make sure that you installed VTK and have cmake

## Compiling

```
mkdir -p build && cd build && cmake .. && cmake --build .
```

## Precomputation

As we run the clustering via python you need to execute this to generate the cluster data:

```python
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python ./python_scripts/run_clustering.py
```

# Running

```
./VisCos [PATH_TO_DATA_FOLDER]
```

# TODO
* Highlight AGNs [VTK]
* animation in vtk [CPP]
* particle tracer (center of galaxies)?
 * [maybe drop] create filter which scales the points to their actual location (via cosmological scale factor 'a') [CPP]
    * fix camera position and zoom away as time progresses (scale with a) [CPP]
    * needs background image

# DONE
* SPH (or alternative)
* highlight Star forming particles 
* temperature in log [CPP]
   * does not look that interesting in paraview
* visualize phi (graphitational potential)
   * shows where the center of mass of the universe is
 * create filter which takes data from UI and filters particles based on their type [CPP]
   * first create particle filter in CPP; Selection: via keypress
 * w,a,s,d to move eyes [CPP]
 * Switch between temperature and clustering [CPP]
 * create animation of clustering [paraview]
 * try clustering from scikit-learn [paraview] 
 * navigate with arrow keys [CPP]
 * create legend for particle color [CPP]
 * create filter to compute temperature [CPP]
 * create filter which scales the points to their actual location (via cosmological scale factor 'a') [paraview]
 * create slider to select timestep [CPP]
 * create filter which takes data from UI and filters particles based on their type [paraview]
 * create filter to compute temperature [paraview]

