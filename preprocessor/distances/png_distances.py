import functools
from glob import glob
import numpy as np
from PIL import Image
import re
from sklearn.metrics import pairwise_distances


def calculate_l2_distance_png(directory):
    """
    Calculates the l1 distance between images saved as .png files.
    For two vector p and q the l1 distance is
    l1 = sum |p_i - q_i| for i to n
    :param directory: The directory that contains the images.
    :return: L1 pairwise distance matrix between every image.
    """
    shapes_files = glob(directory + '*.png')
    shapes_files.sort(key=functools.cmp_to_key(sort_by_sample_id))
    shapes_list = []
    for shape in shapes_files:
        shape_image = Image.open(shape)
        shape_array = np.array(shape_image, dtype='float64')
        shape_array = shape_array.reshape((1, -1))
        shapes_list.append(shape_array)
    all_shapes_array = np.row_stack(shapes_list)
    return pairwise_distances(all_shapes_array)


def calculate_l1_distance_png(directory):
    """
    Calculates the l2 distance between images saved as .png files.
    For two vector p and q the l2 distance is
    l2 = sqrt(sum (p_i - q_i)^2) for i to n
    :param directory: The directory that contains the images.
    :return: L2 pairwise distance matrix between every image.
    """
    shapes_files = glob(directory + '*.png')
    shapes_files.sort(key=functools.cmp_to_key(sort_by_sample_id))
    shapes_list = []
    for shape in shapes_files:
        shape_image = Image.open(shape)
        shape_array = np.array(shape_image, dtype='float64')
        shape_array = shape_array.reshape((1, -1))
        shapes_list.append(shape_array)
    all_shapes_array = np.row_stack(shapes_list)
    return pairwise_distances(all_shapes_array, metric='l1')


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
