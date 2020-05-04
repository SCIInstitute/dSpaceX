from glob import glob
import numpy as np
from PIL import Image
import re

from preprocessor.distances.distance_formulas import l1_distance_formula, l2_distance_formula


def calculate_l2_distance_png(directory):
    shapes = glob(directory + '*.png')
    l2_distance = np.zeros((len(shapes), len(shapes)))
    for design_1 in shapes:
        design_1_id = list(map(int, re.findall(r'\d+', design_1)))[0]
        image_1 = Image.open(design_1)
        image_1 = image_1.convert(mode='L')
        design_1_shape = np.array(image_1, dtype='float64')
        design_1_shape = design_1_shape.reshape((1, -1))
        for design_2 in shapes:
            design_2_id = list(map(int, re.findall(r'\d+', design_2)))[0]
            image_2 = Image.open(design_2)
            image_2 = image_2.convert(mode='L')
            design_2_shape = np.array(image_2, dtype='float64')
            design_2_shape = design_2_shape.reshape((1, -1))
            l2_distance[design_1_id - 1][design_2_id - 1] = l2_distance_formula(design_1_shape, design_2_shape)
    return l2_distance


def calculate_l1_l2_distance_png(directory):
    shapes = glob(directory + '*.png')
    l1_distance = np.zeros((len(shapes), len(shapes)))
    l2_distance = np.zeros((len(shapes), len(shapes)))
    for design_1 in shapes:
        design_1_id = list(map(int, re.findall(r'\d+', design_1)))[0]
        image_1 = Image.open(design_1)
        image_1 = image_1.convert(mode='L')
        design_1_shape = np.array(image_1, dtype='float64')
        design_1_shape = design_1_shape.reshape((1, -1))
        for design_2 in shapes:
            design_2_id = list(map(int, re.findall(r'\d+', design_2)))[0]
            image_2 = Image.open(design_2)
            image_2 = image_2.convert(mode='L')
            design_2_shape = np.array(image_2, dtype='float64')
            design_2_shape = design_2_shape.reshape((1, -1))
            l1_distance[design_1_id - 1][design_2_id - 1] = l1_distance_formula(design_1_shape, design_2_shape)
            l2_distance[design_1_id - 1][design_2_id - 1] = l2_distance_formula(design_1_shape, design_2_shape)
    return l1_distance, l2_distance
