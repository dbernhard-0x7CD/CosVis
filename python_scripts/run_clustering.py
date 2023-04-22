#!/bin/python

import vtk
import numpy as np
from vtk.util.numpy_support import vtk_to_numpy, numpy_to_vtk
from sklearn.cluster import DBSCAN
from sklearn.neighbors import NearestNeighbors
from kneed import KneeLocator

# Load the vtk file
reader = vtk.vtkXMLPolyDataReader()
reader.SetFileName("./data/Full.cosmo.624.vtp")
reader.Update()

writer = vtk.vtkXMLPolyDataWriter()
writer.SetFileName("./data/clusters.vtp")

# Get the points from the VTP file
points = reader.GetOutput().GetPoints()
num_points = points.GetNumberOfPoints()

# Create a numpy array to store the coordinates
coordinates = vtk.vtkFloatArray()
coordinates.SetNumberOfComponents(3)

# Extract the x,y,z coordinates of each point and store them in the array
for i in range(num_points):
    coordinate = points.GetPoint(i)
    coordinates.InsertNextTuple(coordinate)

coordinate_array = vtk_to_numpy(coordinates)

print("Number of coordinates: " + str(coordinate_array.shape[0]))
print("Number of dimensions: " + str(coordinate_array.shape[1]))

# Compute the distance to the k-th nearest neighbor for each point in the dataset
k = 10  # Choose the value of k
nbrs = NearestNeighbors(n_neighbors=k).fit(coordinates)
distances, _ = nbrs.kneighbors(coordinates)

# Sort the distances in descending order
sorted_distances = np.sort(distances[:, -1])[::-1]

# Compute the difference between consecutive distances
diff_distances = np.diff(sorted_distances)

kneedle = KneeLocator(range(1,len(sorted_distances)+1),  #x values
                      sorted_distances, # y values
                      S=1.0, #parameter suggested from paper
                      curve="convex", #parameter from figure
                      direction="decreasing") #parameter from figure
kneedle.plot_knee_normalized()
epsilon = kneedle.knee_y
print(f'Chosen epsilon: {epsilon}')

# Set the DBSCAN hyperparameters
min_samples = 750 # minimum number of points required to form a dense region

# Initialize the DBSCAN clustering algorithm
dbscan = DBSCAN(eps=epsilon, min_samples=min_samples)

# Perform the DBSCAN clustering
labels = dbscan.fit_predict(coordinates)

cluster_indices = numpy_to_vtk(labels)

output = vtk.vtkPolyData()
output.SetPoints(points)
output.GetPointData().AddArray(cluster_indices)

# print(type(labels))
writer.SetInputData(output)
writer.Write()

# Print the cluster labels assigned to each point
n_clusters = len(set(labels)) - (1 if -1 in labels else 0)
print("Cluster labels:", labels)
print("Number of clusters:", len(np.unique(labels)))
print("Number of noise points:", np.sum(labels == -1))
print("Cluster sizes:", np.sort(np.bincount(labels + 1))[::-1])
