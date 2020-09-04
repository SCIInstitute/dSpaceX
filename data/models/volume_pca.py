import functools
from glob import glob
import json
import numpy as np
import nrrd
import re
from sklearn.decomposition import IncrementalPCA


def sort_by_sample_id(file_1, file_2):
    """
    This function guarantees that the files are sorted by their sample id's in their names.
    This is used wehn calling the sort function in both distance calculations.
    :param file_1: The first file for comparision.
    :param file_2: The second file for comparision.
    :return: The difference between the sample id's
    """
    file_1_id = list(map(int, re.findall(r'\d+', file_1)))[-1]
    file_2_id = list(map(int, re.findall(r'\d+', file_2)))[-1]
    return file_1_id - file_2_id


def get_data_matrix(directory):
    shape_files = glob(directory + '*.nrrd')
    shape_files.sort(key=functools.cmp_to_key(sort_by_sample_id))

    shape_list = []
    for shape in shape_files:
        data, header = nrrd.read(shape)
        data = data.flatten()
        shape_list.append(data)

    return np.row_stack(shape_list)


def generate_volume_pca_model(shape_directory, partition_directory, batch_size=20):
    data_matrix = get_data_matrix(shape_directory)

    with open(partition_directory) as json_file:
        partition_config = json.load(json_file)
    partitions = partition_config['crystalPartitions']

    all_pca_models = []
    for crystal in partitions:
        persistence_level = crystal['persistenceLevel']
        crystal_membership = crystal['crystalMembership']
        pca_model_for_persistence = {'pLevel': persistence_level, 'crystalIDs': crystal_membership, 'models': []}
        crystal_ids = np.unique(crystal_membership)
        for c_id in crystal_ids:
            crystal_samples_index = (crystal_membership == c_id)
            crystal_samples = data_matrix[crystal_samples_index]
            transformer = IncrementalPCA(batch_size=batch_size)
            transformer.fit(crystal_samples)
            W = transformer.components_
            w0 = transformer.mean_
            z = np.matmul((crystal_samples - w0), W.T)
            model = {'crystalID': c_id, 'W': W, 'w0': w0, 'z': z}
            pca_model_for_persistence['models'].append(model)
        all_pca_models.append(pca_model_for_persistence)
    return all_pca_models
