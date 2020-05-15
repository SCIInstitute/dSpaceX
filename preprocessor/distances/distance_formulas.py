import numpy as np


def l1_distance_formula(x1, x2):
    """
    Calculates the l1 or manhattan distance between two ndarrays.
    sum(|x1 - x2|)
    https://en.wikipedia.org/wiki/Taxicab_geometry
    :param x1: (ndarray)
    :param x2: (ndarray)
    :return: (scalar) manhattan distance
    """
    return np.sum(np.absolute(x1 - x2))


def l2_distance_formula(x1, x2):
    """
    Calculates the l2 or euclidean distance between two ndarrays.
    sqrt(sum((x1 - x2)^2))
    https://en.wikipedia.org/wiki/Euclidean_distance
    :param x1: (ndarray)
    :param x2: (ndarray)
    :return: (scalar) euclidean distance
    """
    return np.sqrt(np.sum((x1 - x2)**2))