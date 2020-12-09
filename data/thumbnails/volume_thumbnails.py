from glob import glob
from os import path
import re
import PIL.Image
from thumbnails import VolumeRenderer


def generate_volume_thumbnails(shape_directory, output_directory, resolution = [300,300], scale = 1.25, silouettes = True):
    """
    Generates thumbnails from voxels
    :param shape_directory: Directory where voxels are saved
    :param output_directory: Directory where thumbnails are saved
    :param scale: A scaling value so that the designs are within the thumbnails, usually the default is sufficient
    """
    # Get all possible volumees
    shapes = glob(shape_directory + '/*.nrrd')
    padding = '0%dd' % len(str(len(shapes)))

    # instantiate volume renderer
    ren = VolumeRenderer(default=shapes[0], singleview=not silouettes) # default is needed to "warm up" the [vtk] thumbnail renderer

    # For each volume format generate thumbnail
    for index, shape_file in enumerate(shapes):
        shape_id = list(map(int, re.findall(r'\d+', shape_file)))[-1]
        print('Thumbnail generation %.2f percent complete. Generating thumbnail %i of %i. ' %
              ((100 * index / len(shapes)), index, len(shapes)), end='\r')

        ren.loadNewVolume(shape_file)
        image = PIL.Image.fromarray(ren.getImage(resolution=resolution, scale=scale))
        filename = path.join(output_directory, format(shape_id, padding) + '.png')
        image.save(filename)

    # Necessary so next line prints on new line
    print('', end='\n')
