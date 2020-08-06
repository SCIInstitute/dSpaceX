from glob import glob
import nrrd
import re
from skimage import measure

from thumbnails.thumbnail_utils import generate_image_from_vertices_and_faces


def generate_volume_thumbnails(shape_directory, output_directory, scale=10):
    # Get all possible meshes
    shapes = glob(shape_directory + '*.nrrd')

    # For each mesh format generate thumbnail
    for index, shape_file in enumerate(shapes):
        shape_id = list(map(int, re.findall(r'\d+', shape_file)))[-1]
        print('Thumbnail generation %.2f percent complete. Generating thumbnail %i of %i. ' %
              ((100 * index / len(shapes)), index, len(shapes)), end='\r')

        data, _ = nrrd.read(shape_file)
        vertices, faces, _, _, = measure.marching_cubes(data)
        vertices = vertices / scale
        image = generate_image_from_vertices_and_faces(vertices, faces)
        image.save(output_directory + str(shape_id) + '.png')


in_dir = '/Users/kylimckay-bishop/Temporary/OriginalVolume/'
out_dir = '/Users/kylimckay-bishop/Temporary/thumbnails2/'
generate_mesh_thumbnails(in_dir, out_dir)