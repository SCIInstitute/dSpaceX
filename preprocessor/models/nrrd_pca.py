import functools
from glob import glob
import math
import numpy as np
import nrrd
from sklearn.decomposition import IncrementalPCA
import re


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


def not_sure_name_yet(shape_directory, number_of_blocks=15, n_components=20, batch_size=200):
    # get shape files
    shapes = glob(shape_directory + '*.nrrd')
    shapes.sort(key=functools.cmp_to_key(sort_by_sample_id))

    # determine number of shapes to load per block
    number_of_shapes = len(shapes)
    shapes_per_block = math.ceil(number_of_shapes / number_of_blocks)
    block_files = [shapes[i:i+shapes_per_block] for i in range(0, number_of_shapes, shapes_per_block)]

    # create transformer
    transformer = IncrementalPCA(n_components=n_components, batch_size=batch_size)

    # run incremental pca to get W
    sum_of_features = 0
    for files in block_files:
        block = get_block(files)
        sum_of_features = sum_of_features + np.sum(block, axis=0)
        transformer.partial_fit(block)

    W = transformer.components_
    w0 = sum_of_features / number_of_shapes #mean
    z = []
    for file in shapes:
        data, _ = nrrd.read(file)
        x = data.reshape((1, -1))
        z.append(np.matmul((x - w0), W.T))
    z = np.row_stack(z)
    return W, w0, z


def do_things_for_persistence_crystal(shape_directory, partition_directory, n_components=20):
    partitions = np.genfromtxt

    # code expects 2Dim ndarray, this will reshape it if there is only one persistence level
    if partitions.ndim == 1:
        partitions = partitions.reshape((1, -1))

    for p_level, crystal_memberships in enumerate(partitions):
        crystal_ids = np.unique(crystal_memberships)


test_shape_directory = '/Users/kylimckay-bishop/Temporary/nanoparticle_data/original_data_small/design_volumes/'
test_partition_directory = '/Users/kylimckay-bishop/Temporary/nanoparticle_data/processed_data/example/nrrd_minimum/nano_example_min_partitions.csv'
W_, w0_, z_ = not_sure_name_yet(test_shape_directory, number_of_blocks=2, n_components=10)