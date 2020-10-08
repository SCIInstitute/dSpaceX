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
from platform import system

from data.thumbnails.thumbnail_utils import generate_image_from_vertices_and_faces,vtkRenderMesh,writeVtkImage

data_directory = '/Users/cam/data/dSpaceX/latest/'
output_directory = '/tmp/'

print("# RECONSTRUCTING IMAGES")
image_crystal_directory = data_directory + 'CantileverBeam-1/processed_data/ms_partitions/max_stress_pca_model/persistence-4/crystal-0/'

image_latent = np.genfromtxt(image_crystal_directory + 'z.csv', delimiter=',')
image_W = np.genfromtxt(image_crystal_directory + 'W.csv', delimiter=',')
image_w0 = np.genfromtxt(image_crystal_directory + 'w0.csv', delimiter=',')

image_reconstructed_samples = np.matmul(image_latent, image_W) + image_w0

reconstructed_image = image_reconstructed_samples[0].reshape((40, 80))
reconstructed_image = Image.fromarray(reconstructed_image).convert('L').save(output_directory + 'reconstructed_image.png')

print("# RECONSTRUCTING MESH")
mesh_crystal_directory = data_directory + 'nanoparticles_mesh/processed_data/ms_partitions/avg_field_pca_model/persistence-21/crystal-0/'

mesh_latent = np.genfromtxt(mesh_crystal_directory + 'z.csv', delimiter=',')
mesh_W = np.genfromtxt(mesh_crystal_directory + 'W.csv', delimiter=',')
mesh_w0 = np.genfromtxt(mesh_crystal_directory + 'w0.csv', delimiter=',')

mesh_reconstructed_samples = np.matmul(mesh_latent, mesh_W) + mesh_w0

reconstructed_mesh = mesh_reconstructed_samples[0].reshape((-1, 3))

if system() != 'Darwin':  # pyrender doesn't work on osx
    original_mesh_directory = data_directory + 'nanoparticles_mesh/unprocessed_data/shape_representations/1.ply'
    original_mesh = trimesh.load_mesh(original_mesh_directory, process=False)
    mesh_image = generate_image_from_vertices_and_faces(reconstructed_mesh, original_mesh.faces)
    mesh_image.save(output_directory + 'reconstructed_mesh.png')

vtk_mesh_image = vtkRenderMesh(reconstructed_mesh, [])
writeVtkImage(vtk_mesh_image, output_directory + 'vtk-rendered_reconstructed_mesh.png')

print("# RECONSTRUCTED VOLUME")
volume_crystal_directory = data_directory + 'nanoparticles_volume/processed_data/ms_partitions/scattering_field_pca_model/persistence-4/crystal-0/'

volume_latent = np.genfromtxt(volume_crystal_directory + 'z.csv', delimiter=',')
volume_W = np.genfromtxt(volume_crystal_directory + 'W.csv', delimiter=',')
volume_w0 = np.genfromtxt(volume_crystal_directory + 'w0.csv', delimiter=',')

volume_reconstructed_samples = np.matmul(volume_latent, volume_W) + volume_w0

reconstructed_volume = volume_reconstructed_samples[0].reshape((100, 100, 100))
vertices, faces, _, _, = measure.marching_cubes(reconstructed_volume, 0)
vertices = vertices / 10
if system() != 'Darwin':  # pyrender doesn't work on osx
    volume_image = generate_image_from_vertices_and_faces(vertices, faces)
    volume_image.save(output_directory + 'reconstructed_volume.png')

vtk_mesh_image = vtkRenderMesh(vertices, faces)
writeVtkImage(vtk_mesh_image, output_directory + 'vtk-rendered_reconstructed_volume.png')

