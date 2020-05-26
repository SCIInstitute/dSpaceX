import functools
from glob import glob
import math
from multiprocessing import Pool
import nrrd
import numpy as np
import re
from sklearn.metrics import pairwise_distances

def calculate_l1_distance_nrrd(directory, number_of_blocks=15, offset=1):
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
    data, _ = nrrd.read(file)
    return data


def get_block(files, pool):
    result_list = pool.map(get_nrrd_data, files)
    output = np.array(result_list)
    output = output.reshape((len(files), -1))
    return output


def sort_by_sample_id(file_1, file_2):
    file_1_id = list(map(int, re.findall(r'\d+', file_1)))[-1]
    file_2_id = list(map(int, re.findall(r'\d+', file_2)))[-1]
    return file_1_id - file_2_id
