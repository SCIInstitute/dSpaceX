import functools
from glob import glob
import numpy as np
from PIL import Image
import re
from sklearn.metrics import pairwise_distances

def calculate_l2_distance_png(directory):
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
    file_1_id = list(map(int, re.findall(r'\d+', file_1)))[-1]
    file_2_id = list(map(int, re.findall(r'\d+', file_2)))[-1]
    return file_1_id - file_2_id
