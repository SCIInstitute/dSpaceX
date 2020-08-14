"""
This is just a little file I built for myself to make testing
the different reconstructions fro models easy!

What I do is grab a single reconstruction and then make sure it is similar to
the original design.
"""
import pandas as pd
import numpy as np
from skimage import measure
import trimesh

from data.thumbnails.thumbnail_utils import generate_image_from_vertices_and_faces

partition_directory = '/Users/kylimckay-bishop/Temporary/volume_example/processed_data/ms_partitions/avg_field_pca_model/ms_partitions.csv'
qois_directory = '/Users/kylimckay-bishop/Temporary/volume_example/processed_data/Nanoparticles Volume Example_QoIs.csv'

partitions = np.genfromtxt(partition_directory, delimiter=',').reshape(1, -1)
qois_df = pd.read_csv(qois_directory)

crystal_directory = '/Users/kylimckay-bishop/Temporary/volume_example/processed_data/ms_partitions/avg_field_pca_model/persistence-0/crystal-0/'

crystal_latent = np.genfromtxt(crystal_directory + 'z.csv', delimiter=',')
crystal_W = np.genfromtxt(crystal_directory + 'W.csv', delimiter=',')
crystal_w0 = np.genfromtxt(crystal_directory + 'w0.csv', delimiter=',')

reconstructed_samples = np.matmul(crystal_latent, crystal_W) + crystal_w0
sample_1 = reconstructed_samples[0].reshape((100, 100, 100))
vertices, faces, _, _, = measure.marching_cubes(sample_1, 0)
vertices = vertices / 10
image = generate_image_from_vertices_and_faces(vertices, faces)
image.save('/Users/kylimckay-bishop/Temporary/sample1.png')

# mesh = trimesh.load_mesh('/Users/kylimckay-bishop/Temporary/mesh_distance_example/unprocessed_data/shape_representations/3.ply', process=False)
# vertices = mesh.vertices
# faces = mesh.faces
#
# image = generate_image_from_vertices_and_faces(sample_1_vertices, faces)
# image.save('/Users/kylimckay-bishop/Temporary/sample_3_thumb.png')