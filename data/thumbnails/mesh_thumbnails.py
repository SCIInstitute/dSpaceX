from glob import glob
import math
import numpy as np
import re
import trimesh

from thumbnails.thumbnail_utils import generate_image_from_vertices_and_faces


def generate_mesh_thumbnails(shape_directory, output_directory):
    # Get all possible meshes
    shapes = glob(shape_directory + '*.stl')
    shapes.extend(glob(shape_directory + '*.ply'))
    shapes.extend(glob(shape_directory + '*.obj'))

    # For each mesh format generate thumbnail
    for index, shape_file in enumerate(shapes):
        shape_id = list(map(int, re.findall(r'\d+', shape_file)))[-1]
        print('Thumbnail generation %.2f percent complete. Generating thumbnail %i of %i. ' %
              ((100 * index / len(shapes)), index, len(shapes)), end='\r')

        shape_mesh = trimesh.load_mesh(shape_file, process=False)
        vertices = shape_mesh.vertices
        faces = shape_mesh.faces
        image = generate_image_from_vertices_and_faces(vertices, faces)
        image.save(output_directory + str(shape_id) + '.png')
