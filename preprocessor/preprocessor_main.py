from sklearn.manifold import TSNE, Isomap, MDS

from preprocessor.distances.nrrd_distances import calculate_l1_l2_distances_nrrd

# Read in user specification - for now file paths and settings are hardcoded but all these should be specified
# parameters_path = '/Volumes/External_Drive/dSpaceX_data/data/nanoparticles/nanoparticles_design_parameters.csv'
# qois_path = '/Volumes/External_Drive/dSpaceX_data/data/nanoparticles/nanoparticles_QoIs.csv'
test_nrrd_directory = '/Users/kylimckay-bishop/Temporary/nanoparticle_example_data/'
full_nrrd_directory = '/Volumes/External_Drive/dSpaceX_data/raw_data/nanoparticles/highres/BinaryVolume/'

# Calculate distances for shapes
print('Calculating Distances for .nrrd files')
l1_distance_shapes, l2_distance_shapes = calculate_l1_l2_distances_nrrd(test_nrrd_directory)

# Calculate embeddings for shapes
print('Calculating Embeddings - t-SNE, MDS, Isomap')
tsne_l1 = TSNE(n_components=2, metric='precomputed').fit_transform(l1_distance_shapes)
tsne_l2 = TSNE(n_components=2, metric='precomputed').fit_transform(l2_distance_shapes)

mds_l1 = MDS(n_components=2, dissimilarity='precomputed').fit_transform(l1_distance_shapes)
mds_l2 = MDS(n_components=2, dissimilarity='precomputed').fit_transform(l2_distance_shapes)

isomap_l1 = Isomap(n_components=2, metric='precomputed').fit_transform(l1_distance_shapes)
isomap_l2 = Isomap(n_components=2, metric='precomputed').fit_transform(l2_distance_shapes)


# print('Loading design parameters')
# df_parameters = pd.read_csv(parameters_path)
# ndarray_parameters = df_parameters.to_numpy()
#
# print('Calculating distances for parameters')
# l1_distance_parameters = pairwise_distances(ndarray_parameters, metric='l1')
# l2_distance_parameters = pairwise_distances(ndarray_parameters, metric='l2')

