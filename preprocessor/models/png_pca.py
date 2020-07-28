from glob import glob
import functools
import numpy as np
from PIL import Image
from sklearn.decomposition import PCA
import re

from export_model import write_to_file


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


def generate_png_pca_model(shape_directory, partition_directory, n_components=0.97):
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
    partitions = np.genfromtxt(partition_directory, delimiter=',')
    # code expects 2Dim ndarray, this will reshape it if there is only one persistence level
    if partitions.ndim == 1:
        partitions = partitions.reshape((1, -1))
    # create model for each crystal
    all_pca_models = []
    for p_level, crystal_memberships in enumerate(partitions):
        pca_model_for_persistence = {'pLevel': p_level, 'crystalIDs': crystal_memberships, 'models': []}
        crystal_ids = np.unique(crystal_memberships)
        for c_id in crystal_ids:
            crystal_samples_index = (crystal_memberships == c_id)
            crystal_samples = data_matrix[crystal_samples_index]
            transformer = PCA(n_components=n_components)
            transformer.fit(crystal_samples)
            W = transformer.components_
            w0 = np.mean(crystal_samples, axis=0)
            z = np.matmul((crystal_samples - w0), W.T)
            model = {'crystalID': c_id, 'W': W, 'w0': w0, 'z': z}
            pca_model_for_persistence['models'].append(model)
        all_pca_models.append(pca_model_for_persistence)
    return all_pca_models


# Here is an example of using this code - for now it requires a human-in-the-loop
shape_directory_ = '/Users/kylimckay-bishop/Temporary/CantileverBeam-1/processed_data/images/'
partition_directory_ = '/Users/kylimckay-bishop/Temporary/CantileverBeam-1/crystal_partitions/cantilever_crystal_partitions_Max_Stress.csv'
output_directory_ = '/Users/kylimckay-bishop/Temporary/CantileverBeam-1/pca_model_results/'
output_filename_ = 'pca_model_qoi_max_stress'

out = generate_png_pca_model(shape_directory_, partition_directory_)
write_to_file(out, output_directory_, output_filename_)