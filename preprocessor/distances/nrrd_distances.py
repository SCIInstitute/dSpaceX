import functools
from glob import glob
import math
from multiprocessing import Pool
import nrrd
import numpy as np
import re
from sklearn.metrics import pairwise_distances
from scipy.spatial.distance import pdist, squareform


def calculate_hamming_distance_nrrd(directory):
    shapes = glob(directory + '*.nrrd')
    shapes.sort(key=functools.cmp_to_key(sort_by_sample_id))
    array = []
    count = 1
    for s in shapes:
        print('Opening volume number %i' % count)
        count += 1
        data, header = nrrd.read(s)
        data = data.flatten()
        array.append(data)
    print('Calculating Distance')
    array = np.array(array)
    # distance = squareform(pdist(array, metric='hamming'))
    distance = pairwise_distances(array, metric='hamming')
    return distance


def calculate_hamming_distance_nrrd_stream(directory, number_of_blocks=15, offset=1):
    """
    Calculates the hamming distance between binary volumes saved as .nrrd files.
    :param directory: The directory that contains the volumes.
    :param number_of_blocks: Because volumes require significant memory this computation is done by loading
    a portion of the volumes into memory and performing the computation. The "portion of the volumes" are called blocks,
    the number_of_blocks specifies how many blocks to use to perform the calculation
    :param offset: Where the image count starts. Generally, this will be one.
    :return: hamming distance matrix between every volume.
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
                distance = pairwise_distances(block_1, metric='hamming')
            else:
                block_2 = get_block(files_2, pool)
                distance = pairwise_distances(block_1, block_2, metric='hamming')
            block_2_min_index = list(map(int, re.findall(r'\d+', files_2[0])))[-1] - offset
            block_2_max_index = list(map(int, re.findall(r'\d+', files_2[-1])))[-1]
            distances[block_1_min_index:block_1_max_index, block_2_min_index:block_2_max_index] = distance
            column_block += 1
        row_block += 1
    pool.close()
    pool.join()
    return distances


def calculate_l1_distance_nrrd(directory, number_of_blocks=15, offset=1):
    """
    Calculates the l1 distance between volumes saved as .nrrd files.
    For two vector p and q the l1 distance is
    l1 = sum |p_i - q_i| for i to n
    :param directory: The directory that contains the volumes.
    :param number_of_blocks: Because volumes require significant memory this computation is done by loading
    a portion of the volumes into memory and performing the computation. The "portion of the volumes" are called blocks,
    the number_of_blocks specifies how many blocks to use to perform the calculation
    :param offset: Where the image count starts. Generally, this will be one.
    :return: L1 pairwise distance matrix between every volume.
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
                distance = pairwise_distances(block_1, metric='l1')
            else:
                block_2 = get_block(files_2, pool)
                distance = pairwise_distances(block_1, block_2, metric='l1')
            block_2_min_index = list(map(int, re.findall(r'\d+', files_2[0])))[-1] - offset
            block_2_max_index = list(map(int, re.findall(r'\d+', files_2[-1])))[-1]
            distances[block_1_min_index:block_1_max_index, block_2_min_index:block_2_max_index] = distance
            column_block += 1
        row_block += 1
    pool.close()
    pool.join()
    return distances


def calculate_l2_distance_nrrd(directory, number_of_blocks=15, offset=1):
    """
    Calculates the l2 distance between volumes saved as .nrrd files.
    For two vector p and q the l2 distance is
    l2 = sqrt(sum (p_i - q_i)^2) for i to n
    :param directory: The directory that contains the volumes.
    :param number_of_blocks: Because volumes require significant memory this computation is done by loading
    a portion of the volumes into memory and performing the computation. The "portion of the volumes" are called blocks,
    the number_of_blocks specifies how many blocks to use to perform the calculation
    :param offset: Where the image count starts. Generally, this will be one.
    :return: L2 pairwise distance matrix between every volume.
    """
    # get list of shapes to load and sort; code assumes sorted list to place data in
    # distances (results array) correctly
    shapes = glob(directory + '*.nrrd')
    shapes.sort(key=functools.cmp_to_key(sort_by_sample_id))

    # get shapes to load per block
    number_of_shapes = len(shapes)
    shapes_per_block = math.ceil(number_of_shapes / number_of_blocks)
    block_files = [shapes[i:i+shapes_per_block] for i in range(0, number_of_shapes, shapes_per_block)]

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
                distance = pairwise_distances(block_1)
            else:
                block_2 = get_block(files_2, pool)
                distance = pairwise_distances(block_1, block_2)
            block_2_min_index = list(map(int, re.findall(r'\d+', files_2[0])))[-1] - offset
            block_2_max_index = list(map(int, re.findall(r'\d+', files_2[-1])))[-1]
            distances[block_1_min_index:block_1_max_index, block_2_min_index:block_2_max_index] = distance
            column_block += 1
        row_block += 1
    pool.close()
    pool.join()
    return distances


def get_nrrd_data(file):
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
    result_list = pool.map(get_nrrd_data, files)
    output = np.array(result_list)
    output = output.reshape((len(files), -1))
    return output


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


_directory = '/Users/kylimckay-bishop/Workspace/dSpaceX/preprocessor/test/test_volumes/'
test = calculate_hamming_distance_nrrd(_directory)
