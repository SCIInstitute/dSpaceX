import functools
from glob import glob
import math
import nrrd
import numpy as np
import re

from preprocessor.distances.distance_formulas import l1_distance_formula, l2_distance_formula


def calculate_l1_distance_nrrd(directory):
    shapes = glob(directory + '*.nrrd')
    l1_distance = np.zeros((len(shapes), len(shapes)))
    for design_1 in shapes:
        design_1_id = list(map(int, re.findall(r'\d+', design_1)))[0]
        design_1_shape, _ = nrrd.read(design_1)
        design_1_shape = design_1_shape.reshape((1, -1))
        for design_2 in shapes:
            design_2_id = list(map(int, re.findall(r'\d+', design_2)))[0]
            design_2_shape, _ = nrrd.read(design_2)
            design_2_shape = design_2_shape.reshape((1, -1))
            l1_distance[design_1_id - 1][design_2_id - 1] = l1_distance_formula(design_1_shape, design_2_shape)
    return l1_distance


def calculate_l2_distance_nrrd(directory):
    shapes = glob(directory + '*.nrrd')
    num_shapes = len(shapes)
    l2_distance = -1 * np.ones((num_shapes, num_shapes))
    count = 1.0
    for design_1 in shapes:
        design_1_id = list(map(int, re.findall(r'\d+', design_1)))[0]
        print('Distance calculation %.2f percent complete. On sample %i of %i.' % ((100 * count / num_shapes), count,
              num_shapes), end='\r')
        for design_2 in shapes:
            design_2_id = list(map(int, re.findall(r'\d+', design_2)))[0]
            if l2_distance[design_1_id - 1][design_2_id - 1] < 0:
                design_1_shape, _ = nrrd.read(design_1)
                design_1_shape = design_1_shape.reshape((1, -1))
                design_2_shape, _ = nrrd.read(design_2)
                design_2_shape = design_2_shape.reshape((1, -1))
                distance = l2_distance_formula(design_1_shape, design_2_shape)
                l2_distance[design_1_id - 1][design_2_id - 1] = distance
                l2_distance[design_2_id - 1][design_1_id - 1] = distance
        count += 1.0
    return l2_distance


def calculate_l2_distance_nrrd_multithread(directory):
    shapes = glob(directory + '*.nrrd')
    shapes = shapes.sort(key=functools.cmp_to_key(compare_files))
    # create four separate lists so we can process them simultaneously
    step = int(math.ceil(len(shapes) / 4))
    list_1 = shapes[:step]
    list_2 = shapes[step:2*step]
    list_3 = shapes[2*step:3*step]
    list_4 = shapes[3*step:]
    return shapes


def need_better_method_name(processing_list, entire_list, distance_formula=l2_distance_formula):
    data = []
    for shape_1_file in processing_list:
        row = np.zeros((1, len(entire_list)))
        shape_1_id = list(map(int, re.findall(r'\d+', shape_1_file)))[-1]
        for shape_2_file in entire_list[entire_list.index(shape_1_file):]:
            shape_2_id = list(map(int, re.findall(r'\d+', shape_2_file)))[-1]
            shape_1_data, _ = nrrd.read(shape_1_file)
            shape_2_data, _ = nrrd.read(shape_2_file)
            row[shape_1_id][shape_2_id] = distance_formula(shape_1_data, shape_2_data)
        data.append({'row_index': shape_1_id, 'distances': row})


def compare_files(file_1, file_2):
    file_1_id = list(map(int, re.findall(r'\d+', file_1)))[-1]
    file_2_id = list(map(int, re.findall(r'\d+', file_2)))[-1]
    return file_1_id - file_2_id


directory = '/Users/kylimckay-bishop/Temporary/nanoparticle_example_data/design_volumes/'
s = calculate_l2_distance_nrrd_multithread(directory)