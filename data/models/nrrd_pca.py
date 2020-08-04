import functools
from glob import glob
import math
import numpy as np
import nrrd
from sklearn.decomposition import IncrementalPCA
import re

from export_model import write_to_file


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


def get_block(files):
    output = []
    for file in files:
        data, _ = nrrd.read(file)
        output.append(data)
    output = np.array(output)
    return output.reshape((len(files), -1))


def not_sure_name_yet(shapes_files, n_blocks=15):
    # determine number of shapes to load per block
    number_of_shapes = len(shapes_files)
    shapes_per_block = math.ceil(number_of_shapes / n_blocks)
    block_files = [shapes_files[i:i+shapes_per_block] for i in range(0, number_of_shapes, shapes_per_block)]

    # create transformer
    if len(shapes_files) > 20:
        transformer = IncrementalPCA(n_components=20)
    else:
        transformer = IncrementalPCA()

    # run incremental pca to get W, keep track of sum of features to w0, we will calculatet the mean ourselves
    sum_of_features = 0
    for files in block_files:
        block = get_block(files)
        sum_of_features = sum_of_features + np.sum(block, axis=0)
        transformer.partial_fit(block)

    W = transformer.components_
    w0 = sum_of_features / number_of_shapes #mean
    z = []
    for file in shapes_files:
        data, _ = nrrd.read(file)
        x = data.reshape((1, -1))
        z.append(np.matmul((x - w0), W.T))
    z = np.row_stack(z)
    return W, w0, z


def do_things_for_persistence_crystal(shape_directory, partition_directory, n_blocks=15, offset=1):
    partitions = np.genfromtxt(partition_directory, delimiter=',')

    # get shape files
    shapes = glob(shape_directory + '*.nrrd')
    shapes.sort(key=functools.cmp_to_key(sort_by_sample_id))

    # code expects 2Dim ndarray, this will reshape it if there is only one persistence level
    if partitions.ndim == 1:
        partitions = partitions.reshape((1, -1))

    # create model for each crystal
    all_pca_models = []
    number_of_persistence_levels = len(partitions)
    for p_level, crystal_memberships in enumerate(partitions):
        pca_model_for_persistence = {'pLevel': p_level, 'crystalIDs': crystal_memberships, 'models':[]}
        crystal_ids = np.unique(crystal_memberships)
        number_of_crystals = len(crystal_ids)
        for c_id in crystal_ids:
            print('\033[92m Calculating pca model for crystal %i of %i for persistence level %i of %i. \u001b[37m' %
                  (c_id + offset, number_of_crystals, p_level + offset, number_of_persistence_levels))
            crystal_samples_ids = np.where(crystal_memberships == c_id)[0] + offset
            # filter shapes files based on crystal_samples_ids
            crystal_shapes = [s for s in shapes if (list(map(int, re.findall(r'\d+', s)))[-1] in crystal_samples_ids)]
            W, w0, z = not_sure_name_yet(crystal_shapes, n_blocks)
            model = {'crystalID': c_id, 'W': W, 'w0': w0, 'z': z}
            pca_model_for_persistence['models'].append(model)
        all_pca_models.append(pca_model_for_persistence)
    return all_pca_models


test_shape_directory = '/Users/kylimckay-bishop/Temporary/nanoparticle_data/original_data_small/design_volumes/'
test_partition_directory = '/Users/kylimckay-bishop/Temporary/nanoparticle_data/processed_data/example/nrrd_minimum/nano_example_min_partitions.csv'
test_output_directory = '/Users/kylimckay-bishop/downloads/'
output_filename_ = 'nrrd_pca_model_test'

full_shape_directory = '/Volumes/External_Drive/dSpaceX_data/raw_data/nanoparticles/highres/BinaryVolume/'

out = do_things_for_persistence_crystal(test_shape_directory, test_partition_directory)
write_to_file(out, test_output_directory, output_filename_)