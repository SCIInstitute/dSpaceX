import functools
from glob import glob
import json
import numpy as np
from PIL import Image
from sklearn.decomposition import PCA
import re


def sort_by_sample_id(file_1, file_2):
    """
    This function guarantees that the files are sorted by their sample id's in their names.
    This is used when calling the sort function in both distance calculations.
    :param file_1: The first file for comparision.
    :param file_2: The second file for comparision.
    :return: The difference between the sample id's
    """
    file_1_id = list(map(int, re.findall(r'\d+', file_1)))[-1]
    file_2_id = list(map(int, re.findall(r'\d+', file_2)))[-1]
    return file_1_id - file_2_id


def get_data_matrix(directory):
    """
    Loads png data from directory into single data matrix of number of samples X pixels.
    The images are flattened
    :param directory: Directory where png images are found
    :return: data matrix
    """
    shapes_files = glob(directory + '*.png')
    shapes_files.sort(key=functools.cmp_to_key(sort_by_sample_id))
    shapes_list = []
    for shape in shapes_files:
        shape_image = Image.open(shape)
        shape_array = np.array(shape_image, dtype='float64')
        shapes_list.append(shape_array)
    all_shapes_array = np.row_stack(shapes_list)
    all_shapes_array = all_shapes_array.reshape((len(shapes_files), -1))
    return all_shapes_array

def compute_pca_model(crystal_samples, n_components=0.97):
    transformer = PCA(n_components=n_components)
    transformer.fit(crystal_samples)
    W  = transformer.components_
    w0 = transformer.mean_
    z  = np.matmul((crystal_samples - w0), W.T)
    return W, w0, z

def generate_image_pca_model(shape_directory, partition_directory, n_components=0.97):
    """
    Generates PCA model for each crystal in each persistence level.
    :param shape_directory: Directory where png images are found
    :param partition_directory: Directory where crystal partitions are found
    :param n_components: Number of components, defaults to explaining 97% of the variance
    :return: Dictionary of model containing W, w0, and z (latent representation) for each crystal
    in each persistence level.
    """
    # get data
    data_matrix = get_data_matrix(shape_directory)
    with open(partition_directory) as json_file:
        partition_config = json.load(json_file)
    partitions = partition_config['crystalPartitions']

    # create model for each crystal
    all_pca_models = []
    for crystal in partitions:
        persistence_level = crystal['persistenceLevel']
        crystal_membership = crystal['crystalMembership']
        pca_model_for_persistence = {'pLevel': persistence_level, 'crystalIDs': crystal_membership, 'models': []}
        crystal_ids = np.unique(crystal_membership)
        for c_id in crystal_ids:
            crystal_samples_index = (crystal_membership == c_id)
            crystal_samples       = data_matrix[crystal_samples_index]
            W, w0, z              = compute_pca_model(crystal_samples, n_components=n_components)
            model                 = {'crystalID': c_id, 'W': W, 'w0': w0, 'z': z}
            pca_model_for_persistence['models'].append(model)
        all_pca_models.append(pca_model_for_persistence)
    return all_pca_models
