import vtk
import numpy as np
from vtk.util.numpy_support import vtk_to_numpy
from sklearn.cluster import DBSCAN
from sklearn.neighbors import NearestNeighbors
from scipy.spatial.distance import euclidean
from sklearn import metrics
from hdbscan import HDBSCAN
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from kneed import KneeLocator
from matplotlib.widgets import CheckButtons, Button

"""
Implimentation of Density-Based Clustering Validation "DBCV"
Citation:
Moulavi, Davoud, et al. "Density-based clustering validation."
Proceedings of the 2014 SIAM International Conference on Data Mining.
Society for Industrial and Applied Mathematics, 2014.
"""

import numpy as np
from scipy.spatial.distance import euclidean, cdist
from scipy.sparse.csgraph import minimum_spanning_tree
from scipy.sparse import csgraph


def DBCV(X, labels, dist_function=euclidean):
    """
    Density Based clustering validation
    Args:
        X (np.ndarray): ndarray with dimensions [n_samples, n_features]
            data to check validity of clustering
        labels (np.array): clustering assignments for data X
        dist_dunction (func): function to determine distance between objects
            func args must be [np.array, np.array] where each array is a point
    Returns: cluster_validity (float)
        score in range[-1, 1] indicating validity of clustering assignments
    """
    graph = _mutual_reach_dist_graph(X, labels, dist_function)
    mst = _mutual_reach_dist_MST(graph)
    cluster_validity = _clustering_validity_index(mst, labels)
    return cluster_validity


def _core_dist(point, neighbors, dist_function):
    """
    Computes the core distance of a point.
    Core distance is the inverse density of an object.
    Args:
        point (np.array): array of dimensions (n_features,)
            point to compute core distance of
        neighbors (np.ndarray): array of dimensions (n_neighbors, n_features):
            array of all other points in object class
        dist_dunction (func): function to determine distance between objects
            func args must be [np.array, np.array] where each array is a point
    Returns: core_dist (float)
        inverse density of point
    """
    n_features = np.shape(point)[0]
    n_neighbors = np.shape(neighbors)[0]

    distance_vector = cdist(point.reshape(1, -1), neighbors)
    distance_vector = distance_vector[distance_vector != 0]
    numerator = ((1/distance_vector)**n_features).sum()
    core_dist = (numerator / (n_neighbors - 1)) ** (-1/n_features)
    return core_dist


def _mutual_reachability_dist(point_i, point_j, neighbors_i,
                              neighbors_j, dist_function):
    """.
    Computes the mutual reachability distance between points
    Args:
        point_i (np.array): array of dimensions (n_features,)
            point i to compare to point j
        point_j (np.array): array of dimensions (n_features,)
            point i to compare to point i
        neighbors_i (np.ndarray): array of dims (n_neighbors, n_features):
            array of all other points in object class of point i
        neighbors_j (np.ndarray): array of dims (n_neighbors, n_features):
            array of all other points in object class of point j
        dist_dunction (func): function to determine distance between objects
            func args must be [np.array, np.array] where each array is a point
    Returns: mutual_reachability (float)
        mutual reachability between points i and j
    """
    core_dist_i = _core_dist(point_i, neighbors_i, dist_function)
    core_dist_j = _core_dist(point_j, neighbors_j, dist_function)
    dist = dist_function(point_i, point_j)
    mutual_reachability = np.max([core_dist_i, core_dist_j, dist])
    return mutual_reachability


def _mutual_reach_dist_graph(X, labels, dist_function):
    """
    Computes the mutual reach distance complete graph.
    Graph of all pair-wise mutual reachability distances between points
    Args:
        X (np.ndarray): ndarray with dimensions [n_samples, n_features]
            data to check validity of clustering
        labels (np.array): clustering assignments for data X
        dist_dunction (func): function to determine distance between objects
            func args must be [np.array, np.array] where each array is a point
    Returns: graph (np.ndarray)
        array of dimensions (n_samples, n_samples)
        Graph of all pair-wise mutual reachability distances between points.
    """
    n_samples = np.shape(X)[0]
    graph = []
    counter = 0
    for row in range(n_samples):
        graph_row = []
        for col in range(n_samples):
            point_i = X[row]
            point_j = X[col]
            class_i = labels[row]
            class_j = labels[col]
            members_i = _get_label_members(X, labels, class_i)
            members_j = _get_label_members(X, labels, class_j)
            dist = _mutual_reachability_dist(point_i, point_j,
                                             members_i, members_j,
                                             dist_function)
            graph_row.append(dist)
        counter += 1
        graph.append(graph_row)
    graph = np.array(graph)
    return graph


def _mutual_reach_dist_MST(dist_tree):
    """
    Computes minimum spanning tree of the mutual reach distance complete graph
    Args:
        dist_tree (np.ndarray): array of dimensions (n_samples, n_samples)
            Graph of all pair-wise mutual reachability distances
            between points.
    Returns: minimum_spanning_tree (np.ndarray)
        array of dimensions (n_samples, n_samples)
        minimum spanning tree of all pair-wise mutual reachability
            distances between points.
    """
    mst = minimum_spanning_tree(dist_tree).toarray()
    return mst + np.transpose(mst)


def _cluster_density_sparseness(MST, labels, cluster):
    """
    Computes the cluster density sparseness, the minimum density
        within a cluster
    Args:
        MST (np.ndarray): minimum spanning tree of all pair-wise
            mutual reachability distances between points.
        labels (np.array): clustering assignments for data X
        cluster (int): cluster of interest
    Returns: cluster_density_sparseness (float)
        value corresponding to the minimum density within a cluster
    """
    indices = np.where(labels == cluster)[0]
    cluster_MST = MST[indices][:, indices]
    cluster_density_sparseness = np.max(cluster_MST)
    return cluster_density_sparseness


def _cluster_density_separation(MST, labels, cluster_i, cluster_j):
    """
    Computes the density separation between two clusters, the maximum
        density between clusters.
    Args:
        MST (np.ndarray): minimum spanning tree of all pair-wise
            mutual reachability distances between points.
        labels (np.array): clustering assignments for data X
        cluster_i (int): cluster i of interest
        cluster_j (int): cluster j of interest
    Returns: density_separation (float):
        value corresponding to the maximum density between clusters
    """
    indices_i = np.where(labels == cluster_i)[0]
    indices_j = np.where(labels == cluster_j)[0]
    shortest_paths = csgraph.dijkstra(MST, indices=indices_i)
    relevant_paths = shortest_paths[:, indices_j]
    density_separation = np.min(relevant_paths)
    return density_separation


def _cluster_validity_index(MST, labels, cluster):
    """
    Computes the validity of a cluster (validity of assignmnets)
    Args:
        MST (np.ndarray): minimum spanning tree of all pair-wise
            mutual reachability distances between points.
        labels (np.array): clustering assignments for data X
        cluster (int): cluster of interest
    Returns: cluster_validity (float)
        value corresponding to the validity of cluster assignments
    """
    min_density_separation = np.inf
    for cluster_j in np.unique(labels):
        if cluster_j != cluster:
            cluster_density_separation = _cluster_density_separation(MST,
                                                                     labels,
                                                                     cluster,
                                                                     cluster_j)
            if cluster_density_separation < min_density_separation:
                min_density_separation = cluster_density_separation
    cluster_density_sparseness = _cluster_density_sparseness(MST,
                                                             labels,
                                                             cluster)
    numerator = min_density_separation - cluster_density_sparseness
    denominator = np.max([min_density_separation, cluster_density_sparseness])
    cluster_validity = numerator / denominator
    return cluster_validity


def _clustering_validity_index(MST, labels):
    """
    Computes the validity of all clustering assignments for a
    clustering algorithm
    Args:
        MST (np.ndarray): minimum spanning tree of all pair-wise
            mutual reachability distances between points.
        labels (np.array): clustering assignments for data X
    Returns: validity_index (float):
        score in range[-1, 1] indicating validity of clustering assignments
    """
    n_samples = len(labels)
    validity_index = 0
    for label in np.unique(labels):
        fraction = np.sum(labels == label) / float(n_samples)
        cluster_validity = _cluster_validity_index(MST, labels, label)
        validity_index += fraction * cluster_validity
    return validity_index


def _get_label_members(X, labels, cluster):
    """
    Helper function to get samples of a specified cluster.
    Args:
        X (np.ndarray): ndarray with dimensions [n_samples, n_features]
            data to check validity of clustering
        labels (np.array): clustering assignments for data X
        cluster (int): cluster of interest
    Returns: members (np.ndarray)
        array of dimensions (n_samples, n_features) of samples of the
        specified cluster.
    """
    indices = np.where(labels == cluster)[0]
    members = X[indices]
    return members


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
#epsilon = 4.0 #2.846058422977199 #for debugging

# Set the DBSCAN hyperparameters
min_samples = 750 # minimum number of points required to form a dense region

# Initialize the DBSCAN clustering algorithm
dbscan = DBSCAN(eps=epsilon, min_samples=min_samples)
#hdbscan = HDBSCAN(min_cluster_size=min_samples, min_samples=min_samples, metric='euclidean', cluster_selection_epsilon=2.5)
# Perform the DBSCAN/HDBSCAN clustering
labels = dbscan.fit_predict(coordinates)
#labels = hdbscan.fit_predict(coordinates)
""" sil_score = metrics.silhouette_score(coordinate_array, labels, metric='euclidean', sample_size=1000, random_state=12)
print("Silhouette Coefficient: %0.3f"  % sil_score)
dbcv_score = DBCV(coordinate_array, labels, dist_function=euclidean) """


# Print the cluster labels assigned to each point
n_clusters = len(set(labels)) - (1 if -1 in labels else 0)
print("Cluster labels:", labels)
print("Number of clusters:", len(np.unique(labels)))
print("Number of noise points:", np.sum(labels == -1))
print("Cluster sizes:", np.sort(np.bincount(labels + 1))[::-1])
np.save('../data/cluster_labels.npy', labels)

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