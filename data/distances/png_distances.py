import functools
from glob import glob
import numpy as np
from PIL import Image
from sklearn.metrics import pairwise_distances

from distances.distance_utils import sort_by_sample_id


def calculate_distance_png(directory, metric='hamming'):
    """
    Calculates the distances between images saved as .png files.
    :param metric: The distance metric to calculate.
    Supported metrics include: cityblock, cosine, euclidean, l1, l2, manhattan, barycurtis, canberra, chebysheve, correlation,
    dice, hamming, jaccard, kulsinski, nahlanobi, minkowski, regerstandimoto, russellrao, seuclidean, sokalmichener, sokalsneath,
    sqeuclidean, yule
    :param directory: The directory that contains the images.
    :return: L1 pairwise distance matrix between every image.
    """
    shapes_files = glob(directory + '*.png')
    shapes_files.sort(key=functools.cmp_to_key(sort_by_sample_id))
    shapes_list = []
    for shape in shapes_files:
        shape_image = Image.open(shape).convert('1')
        shape_array = np.array(shape_image, dtype='float64')
        shape_array = shape_array.reshape((1, -1))
        shapes_list.append(shape_array)
    all_shapes_array = np.row_stack(shapes_list)
    return pairwise_distances(all_shapes_array, metric=metric)
