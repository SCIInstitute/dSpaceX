import functools
from glob import glob
import numpy as np
from sklearn.metrics import pairwise_distances
import trimesh

from distances.distance_utils import sort_by_sample_id


def calculate_distance_mesh(directory, metric='l2'):
    """
    Calculates the distance between meshes saved as .csv files.
    :param metric: The distance metric to calculate.
    Supported metrics include: cityblock, cosine, euclidean, l1, l2, manhattan, barycurtis, canberra, chebysheve,
    correlation, dice, hamming, jaccard, kulsinski, nahlanobi, minkowski, regerstandimoto, russellrao, seuclidean,
    sokalmichener, sokalsneath, sqeuclidean, yule
    :param directory: The directory that contains the images.
    :return: Distance matrix between every mesh.
    """
    print('Starting mesh distance calculation.')
    shapes_files = glob(directory + '*.stl')
    shapes_files.extend(glob(directory + '*.ply'))
    shapes_files.extend(glob(directory + '*.obj'))

    shapes_files.sort(key=functools.cmp_to_key(sort_by_sample_id))
    shapes_list = []
    for shape_index, shape in enumerate(shapes_files):
        print('Opening shape %i / %i' % (shape_index, len(shapes_files)), end='\r')
        shape_mesh = trimesh.load_mesh(shape, process=False)
        vertices = shape_mesh.vertices
        vertices = vertices.reshape((1, -1))
        shapes_list.append(vertices)
    all_shapes_array = np.row_stack(shapes_list)
    print('Calling pairwise distance.')
    return pairwise_distances(all_shapes_array, metric=metric)
