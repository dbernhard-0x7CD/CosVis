import vtk
import numpy as np
from vtk.util.numpy_support import vtk_to_numpy
from sklearn.cluster import DBSCAN
from sklearn.neighbors import NearestNeighbors
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from kneed import KneeLocator
from matplotlib.widgets import CheckButtons, Button


# Load the vtk file
reader = vtk.vtkXMLPolyDataReader()
reader.SetFileName("../data/Full.cosmo.624.vtp")
reader.Update()

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

# Plot the elbow curve with the knee and epsilon marked
fig, ax = plt.subplots()
ax.plot(range(len(sorted_distances)), sorted_distances)
ax.set_xlabel('Point index')
ax.set_ylabel(f'{k}-th nearest neighbor distance')
ax.set_title('Elbow plot for DBSCAN')
plt.show()
#epsilon = 2.846058422977199 #for debugging

# Set the DBSCAN hyperparameters
min_samples = 750 # minimum number of points required to form a dense region

# Initialize the DBSCAN clustering algorithm
dbscan = DBSCAN(eps=epsilon, min_samples=min_samples)

# Perform the DBSCAN clustering
labels = dbscan.fit_predict(coordinates)

# Print the cluster labels assigned to each point
n_clusters = len(set(labels)) - (1 if -1 in labels else 0)
print("Cluster labels:", labels)
print("Number of clusters:", len(np.unique(labels)))
print("Number of noise points:", np.sum(labels == -1))
print("Cluster sizes:", np.sort(np.bincount(labels + 1))[::-1])

# Create a 3D plot
fig = plt.figure()
ax = fig.add_subplot(projection='3d')
#ax.set_facecolor('black')
scatter = {}
cluster_checkboxes = []
# Plot the points with different colors for each cluster
for label in np.unique(labels):
    if label == -1:
        # Plot noise points in black and less opaque
        color = 'k'
        alpha = 0.05
        s = 0.125
    else:
        # Assign a unique color to each cluster
        color = plt.cm.nipy_spectral(label / float(len(np.unique(labels))))
        alspha = 0.2
        s = 0.25

    # Plot the points belonging to the current cluster
    scatter[label+1] = ax.scatter(coordinate_array[labels == label, 0], 
                    coordinate_array[labels == label, 1], 
                    coordinate_array[labels == label, 2], 
                    c=color,
                    s=s, 
                    alpha=alpha,
                    marker='.', picker=True)
    rax = fig.add_axes([0.05, 0.9-(label+2)*0.025, 0.05, 0.025])
    if label == -1:
        label = 'Noise'
    cluster_checkboxes.append(CheckButtons(ax=rax,labels=[label],actives=[True]))

button_ax = fig.add_axes([0.05, 0.9-(len(np.unique(labels))+2)*0.025, 0.05, 0.025])
button = Button(ax=button_ax, label='All', color='lightgray', hovercolor='darkgray')
#checkboxes = CheckButtons(ax=rax,labels=[f'C{i}' for i in range(-1,n_clusters)],actives=[True]*(n_clusters+1))
# Function to update the plot based on the selected checkboxes
def update_plot(label):
    scatter[label].set_visible(cluster_checkboxes[label].get_status()[0])
    fig.canvas.draw()
    
def check_all(event):
    for l in scatter:
        if not cluster_checkboxes[l].get_status()[0]: 
            cluster_checkboxes[l].set_active(0)
            scatter[l].set_visible(True)
    fig.canvas.draw()

# Add a callback to the CheckButtons
#checkboxes.on_clicked(update_plot)
for i, checkbox in enumerate(cluster_checkboxes):
    checkbox.on_clicked(lambda event, label=i: update_plot(label))
button.on_clicked(check_all)



# Set the plot title and labels
ax.set_title('DBSCAN Clustering')
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

plt.show()