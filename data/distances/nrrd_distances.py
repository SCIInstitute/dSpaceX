import functools
from glob import glob
import math
from multiprocessing import Pool
import nrrd
import numpy as np
import re
from sklearn.metrics import pairwise_distances

from distances.distance_utils import sort_by_sample_id


def calculate_distance_volume(directory, metric='hamming'):
    """
    Calculates the distance between binary volumes saved as .nrrd files.
    This method is for small volumes that fit in memory.
    :param metric: The distance metric to calculate.
    Supported metrics include: cityblock, cosine, euclidean, l1, l2, manhattan, barycurtis, canberra, chebysheve, correlation,
    dice, hamming, jaccard, kulsinski, nahlanobi, minkowski, regerstandimoto, russellrao, seuclidean, sokalmichener, sokalsneath,
    sqeuclidean, yule
    :param directory: The directory that contains the volumes.
    :return: pairwise distance matrix between every volume.
    """
    shapes = glob(directory + '*.nrrd')
    shapes.sort(key=functools.cmp_to_key(sort_by_sample_id))
    array = []
    count = 1
    for s in shapes:
        count += 1
        data, header = nrrd.read(s)
        data = data.flatten()
        array.append(data)
    print('Calculating Distance', end='\n')
    array = np.array(array)
    distance = pairwise_distances(array, metric=metric)
    return distance


def calculate_distance_volume_streaming(directory, metric='hamming', number_of_blocks=15, offset=1):
    """
    Calculates the distance between binary volumes saved as .nrrd files.
    This method is for large volumes that do not fit in to memory.
    :param metric: The distance metric to calculate.
    Supported metrics include: cityblock, cosine, euclidean, l1, l2, manhattan, barycurtis, canberra, chebysheve, correlation,
    dice, hamming, jaccard, kulsinski, nahlanobi, minkowski, regerstandimoto, russellrao, seuclidean, sokalmichener, sokalsneath,
    sqeuclidean, yule
    :param directory: The directory that contains the volumes.
    :param number_of_blocks: Because volumes require significant memory this computation is done by loading
    a portion of the volumes into memory and performing the computation. The "portion of the volumes" are called blocks,
    the number_of_blocks specifies how many blocks to use to perform the calculation
    :param offset: Where the image count starts. Generally, this will be one.
    :return: pairwise distance matrix between every volume.
    """
    # get list of shapes to load and sort; code assumes sorted list to place data in
    # distances (results array) correctly
    shapes = glob(directory + '*.nrrd')
    shapes.sort(key=functools.cmp_to_key(sort_by_sample_id))

    # get shapes to load per block
    number_of_shapes = len(shapes)
    shapes_per_block = math.ceil(number_of_shapes / number_of_blocks)
    block_files = [shapes[i:i+shapes_per_block] for i in range(0, len(shapes), shapes_per_block)]

    pool = Pool(6)  # pool to facilitate multiprocessing of file loading
    distances = np.zeros((number_of_shapes, number_of_shapes))  # results
    row_block = 1  # to track progress
    for files_1 in block_files:
        block_1 = get_block(files_1, pool)
        block_1_min_index = list(map(int, re.findall(r'\d+', files_1[0])))[-1] - offset
        block_1_max_index = list(map(int, re.findall(r'\d+', files_1[-1])))[-1]
        column_block = 1
        for files_2 in block_files:
            print('Calculating distance between row block %i column block %i.' % (row_block, column_block), end='\r')
            # Don't need to load the same files twice
            if files_1 == files_2:
                distance = pairwise_distances(block_1, metric=metric)
            else:
                block_2 = get_block(files_2, pool)
                distance = pairwise_distances(block_1, block_2, metric=metric)
            block_2_min_index = list(map(int, re.findall(r'\d+', files_2[0])))[-1] - offset
            block_2_max_index = list(map(int, re.findall(r'\d+', files_2[-1])))[-1]
            distances[block_1_min_index:block_1_max_index, block_2_min_index:block_2_max_index] = distance
            column_block += 1
        row_block += 1
    pool.close()
    pool.join()
    return distances


def get_volume_data(file):
    """
    Wrapper function to return only the data in the nrrd file; ignores the header.
    :param file: File to read
    :return: Numpy ndarray of data in nrrd file.
    """
    data, _ = nrrd.read(file)
    return data


def get_block(files, pool):
    """
    Loads a block of data and generates a data matrix that is # of samples X dimension.
    The data is flattened and the dimensions equals width * height * depth.
    To decrease computation time this method takes a pool object and loads the data
    using multiple processes. The pool object is an argument for reuse this avoids the overhead of
    generating a new pool each time the function is called.
    :param files: List of files to load
    :param pool: Pool object, allows pool reuse and faster loading through multi-processing
    :return: Numpy ndarray of samples contained in block.
    """
    result_list = pool.map(get_volume_data, files)
    output = np.array(result_list)
    output = output.reshape((len(files), -1))
    return output
