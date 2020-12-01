
import numpy as np
import re
from sklearn.metrics import pairwise_distances

import models.png_pca as png_pca
import models.mesh_pca as mesh_pca
import models.volume_pca as volume_pca

    
def calculate_pca_distance_volume(directory, batch_size=20):
    """
    Calculates the distance between volumes saved as .nrrd files in the
    pca subspace. Volumes are first read then embedded in a pca subspace, 
    pairwise distances are then computed based on the pca embedding
    This method is for small volumes that fit in memory.
    :param directory: The directory that contains the volumes.
    :return: pairwise distance matrix between every volume.
    """
    data_matrix = volume_pca.get_data_matrix(directory)

    print('Calculating PCA Distance', end='\n')

    _,_,z = volume_pca.compute_pca_model(data_matrix, batch_size=batch_size)
            
    distance = pairwise_distances(z, metric='l2')
    return distance

def calculate_pca_distance_png(directory, n_components=0.97):
    """
    Calculates the distances between images saved as .png files in the
    pca subspace. Images are first read then embedded in a pca subspace, 
    pairwise distances are then computed based on the pca embedding
    :param directory: The directory that contains the images.
    :return: pairwise distance matrix between every image.
    """
    data_matrix = png_pca.get_data_matrix(directory)

    print('Calculating PCA Distance', end='\n')

    _,_,z = png_pca.compute_pca_model(data_matrix, n_components=n_components)
            
    distance = pairwise_distances(z, metric='l2')
    return distance


def calculate_pca_distance_mesh(directory, batch_size=20):
    """
    Calculates the distance between meshes saved as .csv files in the
    pca subspace. Meshes are first read then embedded in a pca subspace, 
    pairwise distances are then computed based on the pca embedding
    :param directory: The directory that contains the meshes.
    :return: pairwise distance matrix between every mesh.
    """
    data_matrix = mesh_pca.get_data_matrix(directory)

    print('Calculating PCA Distance', end='\n')

    _,_,z = mesh_pca.compute_pca_model(data_matrix, batch_size=batch_size)
            
    distance = pairwise_distances(z, metric='l2')
    return distance