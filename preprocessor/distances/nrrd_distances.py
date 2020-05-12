from glob import glob
import nrrd
import numpy as np
import re

from distances.distance_formulas import l1_distance_formula, l2_distance_formula


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
    l2_distance = np.zeros((len(shapes), len(shapes)))
    for design_1 in shapes:
        design_1_id = list(map(int, re.findall(r'\d+', design_1)))[0]
        design_1_shape, _ = nrrd.read(design_1)
        design_1_shape = design_1_shape.reshape((1, -1))
        for design_2 in shapes:
            design_2_id = list(map(int, re.findall(r'\d+', design_2)))[0]
            design_2_shape, _ = nrrd.read(design_2)
            design_2_shape = design_2_shape.reshape((1, -1))
            l2_distance[design_1_id - 1][design_2_id - 1] = l2_distance_formula(design_1_shape, design_2_shape)
    return l2_distance


def calculate_l1_l2_distances_nrrd(directory):
    shapes = glob(directory + '*.nrrd')
    l1_distance = np.zeros((len(shapes), len(shapes)))
    l2_distance = np.zeros((len(shapes), len(shapes)))
    for design_1 in shapes:
        design_1_id = list(map(int, re.findall(r'\d+', design_1)))[0]
        design_1_shape, _ = nrrd.read(design_1)
        design_1_shape = design_1_shape.reshape((1, -1))
        for design_2 in shapes:
            design_2_id = list(map(int, re.findall(r'\d+', design_2)))[0]
            design_2_shape, _ = nrrd.read(design_2)
            design_2_shape = design_2_shape.reshape((1, -1))
            l1_distance[design_1_id - 1][design_2_id - 1] = l1_distance_formula(design_1_shape, design_2_shape)
            l2_distance[design_1_id - 1][design_2_id - 1] = l2_distance_formula(design_1_shape, design_2_shape)
    return l1_distance, l2_distance
