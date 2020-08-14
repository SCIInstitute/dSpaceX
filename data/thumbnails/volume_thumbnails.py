from glob import glob
import nrrd
from PIL import Image, ImageDraw
import re
from skimage import measure

from data.thumbnails.thumbnail_utils import generate_image_from_vertices_and_faces


def generate_volume_thumbnails(shape_directory, output_directory, scale=10):
    """
    Generates thumbnails from voxels
    :param shape_directory: Directory where voxels are saved
    :param output_directory: Directory where thumbnails are saved
    :param scale: A scaling value so that the designs are within the thumbnails, usually the default is sufficient
    """
    # Get all possible meshes
    shapes = glob(shape_directory + '*.nrrd')

    # For each mesh format generate thumbnail
    for index, shape_file in enumerate(shapes):
        shape_id = list(map(int, re.findall(r'\d+', shape_file)))[-1]
        print('Thumbnail generation %.2f percent complete. Generating thumbnail %i of %i. ' %
              ((100 * index / len(shapes)), index, len(shapes)), end='\r')

        data, _ = nrrd.read(shape_file)
        try:
            vertices, faces, _, _, = measure.marching_cubes(data, 0)
            vertices = vertices / scale
            image = generate_image_from_vertices_and_faces(vertices, faces)
        except RuntimeError:
            image = Image.new('RGB', (144, 108), color=(0, 0, 0))
            d = ImageDraw.Draw(image)
            d.text((10, 40), 'Image not generated \nfrom volume.', fill=(255, 255, 255))
            image = image.convert('LA')
        image.save(output_directory + str(shape_id) + '.png')
    # Necessary so next line prints on new line
    print('', end='\n')
