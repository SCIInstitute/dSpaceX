import numpy as np


def l1_distance_formula(x1, x2):
    return np.sum(np.absolute(x1 - x2))


def l2_distance_formula(x1, x2):
    return np.sqrt(np.sum((x1 - x2)**2))