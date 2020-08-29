"""
This is just a little file I built for myself to make testing
the different reconstructions fro models easy!

What I do is grab a single reconstruction and then make sure it is similar to
the original design.
"""
import numpy as np
from PIL import Image
import trimesh
from skimage import measure

from data.thumbnails.thumbnail_utils import generate_image_from_vertices_and_faces

output_directory = '/Users/kylimckay-bishop/Temporary/reconstructed_designs/'

# RECONSTRUCTING IMAGES
image_crystal_directory = '/Users/kylimckay-bishop/dSpaceX/CantileverBeam-1/processed_data/ms_partitions/max_stress_pca_model/persistence-4/crystal-0/'

image_latent = np.genfromtxt(image_crystal_directory + 'z.csv', delimiter=',')
image_W = np.genfromtxt(image_crystal_directory + 'W.csv', delimiter=',')
image_w0 = np.genfromtxt(image_crystal_directory + 'w0.csv', delimiter=',')

image_reconstructed_samples = np.matmul(image_latent, image_W) + image_w0

reconstructed_image = image_reconstructed_samples[0].reshape((40, 80))
reconstructed_image = Image.fromarray(reconstructed_image).convert('L').save(output_directory + 'reconstructed_image.png')

# RECONSTRUCTING MESH
mesh_crystal_directory = '/Volumes/External_Drive/dSpaceX/nanoparticles_mesh/processed_data/ms_partitions/avg_field_pca_model/persistence-21/crystal-0/'

mesh_latent = np.genfromtxt(mesh_crystal_directory + 'z.csv', delimiter=',')
mesh_W = np.genfromtxt(mesh_crystal_directory + 'W.csv', delimiter=',')
mesh_w0 = np.genfromtxt(mesh_crystal_directory + 'w0.csv', delimiter=',')

mesh_reconstructed_samples = np.matmul(mesh_latent, mesh_W) + mesh_w0

reconstructed_mesh = mesh_reconstructed_samples[0].reshape((-1, 3))

original_mesh_directory = '/Volumes/External_Drive/dSpaceX/nanoparticles_mesh/unprocessed_data/shape_representations/1.ply'
original_mesh = trimesh.load_mesh(original_mesh_directory, process=False)

mesh_image = generate_image_from_vertices_and_faces(reconstructed_mesh, original_mesh.faces)
mesh_image.save(output_directory + 'reconstructed_mesh.png')

# RECONSTRUCTED VOLUME
volume_crystal_directory = '/Volumes/External_Drive/dSpaceX/nanoparticles_volume/processed_data/ms_partitions/scattering_field_pca_model/persistence-4/crystal-0/'

volume_latent = np.genfromtxt(volume_crystal_directory + 'z.csv', delimiter=',')
volume_W = np.genfromtxt(volume_crystal_directory + 'W.csv', delimiter=',')
volume_w0 = np.genfromtxt(volume_crystal_directory + 'w0.csv', delimiter=',')

volume_reconstructed_samples = np.matmul(volume_latent, volume_W) + volume_w0

reconstructed_volume = volume_reconstructed_samples[0].reshape((100, 100, 100))
vertices, faces, _, _, = measure.marching_cubes(reconstructed_volume, 0)
vertices = vertices / 10
volume_image = generate_image_from_vertices_and_faces(vertices, faces)
volume_image.save(output_directory + 'reconstructed_volume.png')

