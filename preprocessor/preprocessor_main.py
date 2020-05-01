import pandas as pd
from sklearn.decomposition import PCA
from sklearn.metrics import pairwise_distances

from preprocessor.read_data import read_nrrd_files
from preprocessor.utils import create_shape_matrix_from_list


# Read in user specification - for now file paths and settings are hardcoded but all these should be specified
use_pca = True
percent_variation_pca = 0.97
parameters_path = '/Volumes/External_Drive/dSpaceX_data/data/nanoparticles/nanoparticles_design_parameters.csv'
qois_path = '/Volumes/External_Drive/dSpaceX_data/data/nanoparticles/nanoparticles_QoIs.csv'
nrrd_directory = '/Volumes/External_Drive/dSpaceX_data/raw_data/nanoparticles/lowres/BinaryVolume/'

# Load data
print('Loading .nrrd files')
list_shapes = read_nrrd_files(nrrd_directory)
ndarry_shapes = create_shape_matrix_from_list(list_shapes)

print('Loading design parameters')
df_parameters = pd.read_csv(parameters_path)
ndarray_parameters = df_parameters.to_numpy()

print('Loading quantities of interest')
df_qois = pd.read_csv(qois_path)

# Calculate distances
ndarray_for_distances_shapes = ndarry_shapes
if use_pca:
    print('Calculating PCA for shapes')
    pca = PCA(n_components=percent_variation_pca, svd_solver='full')
    ndarray_for_distances_shapes = pca.fit_transform(ndarray_for_distances_shapes)

print('Calculating distances for shapes')
l1_distance_shapes = pairwise_distances(ndarray_for_distances_shapes, metric='l1')
l2_distance_shapes = pairwise_distances(ndarray_for_distances_shapes, metric='l2')

print('Calculating distances for parameters')
l1_distance_parameters = pairwise_distances(ndarray_parameters, metric='l1')
l2_distance_parameters = pairwise_distances(ndarray_parameters, metric='l2')

# Calculate Embeddings