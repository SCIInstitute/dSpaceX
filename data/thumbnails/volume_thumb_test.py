import nrrd
from skimage import measure
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

from data.thumbnails.thumbnail_utils import generate_image_from_vertices_and_faces

shape_file = '/Users/kylimckay-bishop/Desktop/empty_volumes/Images.0178.nrrd'
out_dir = '/Users/kylimckay-bishop/Temporary/thumbnails2/'

data, _ = nrrd.read(shape_file)
vertices, faces, _, _, = measure.marching_cubes(data, 0)
# vertices = vertices/10

fig = plt.figure(figsize=(10, 10))
ax = fig.add_subplot(111, projection='3d')

# Fancy indexing: `verts[faces]` to generate a collection of triangles
mesh = Poly3DCollection(vertices[faces])
mesh.set_edgecolor('k')
ax.add_collection3d(mesh)

ax.set_xlabel("x-axis: a = 6 per ellipsoid")
ax.set_ylabel("y-axis: b = 10")
ax.set_zlabel("z-axis: c = 16")

ax.set_xlim(0, 50)  # a = 6 (times two for 2nd ellipsoid)
ax.set_ylim(0, 50)  # b = 10
ax.set_zlim(0, 50)  # c = 16

plt.tight_layout()
plt.show()

image = generate_image_from_vertices_and_faces(vertices, faces)
image.save(out_dir + 'test.png')