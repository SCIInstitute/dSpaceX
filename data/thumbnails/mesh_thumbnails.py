from glob import glob
import re
import PIL.Image
import thumbnails


def generate_mesh_thumbnails(shape_directory, output_directory):
    """
    Generate mesh thumbnails from .stl, .ply, or .obj files.
    :param shape_directory: Directory that contains mesh shapes
    :param output_directory: Directory to save generated thumbnails
    """
    # Get all possible meshes
    shapes = glob(shape_directory + '*.stl')
    shapes.extend(glob(shape_directory + '*.ply'))
    shapes.extend(glob(shape_directory + '*.obj'))

    # instantiate mesh renderer
    ren = thumbnails.MeshRenderer(default_mesh = '')

    # For each mesh format generate thumbnail
    for index, shape_file in enumerate(shapes):
        shape_id = list(map(int, re.findall(r'\d+', shape_file)))[-1]
        print('Thumbnail generation %.2f percent complete. Generating thumbnail %i of %i. ' %
              ((100 * index / len(shapes)), index, len(shapes)), end='\r')

        ren.loadNewMesh(shape_file)
        image = PIL.Image.fromarray(ren.getImage())
        image.save(output_directory + str(shape_id) + '.png')
        
    print('', end='\n')
