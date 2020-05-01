import numpy as np


def create_shape_matrix_from_list(_list):
    shapes = [np.reshape(design['shape'], (1, -1)) for design in _list]
    return np.stack(shapes).reshape((len(_list), -1))