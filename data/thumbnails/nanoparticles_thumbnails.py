import math
import matplotlib.tri as mtri
import matplotlib.pyplot as plt
import numpy as np
from numpy.linalg import inv
import pandas as pd
import PIL
from PIL import Image
import pyrender
import trimesh


def generate_nano_thumbnails(parameter_csv, output_directory, add_slices=True):
    parameter_df = pd.read_csv(parameter_csv)

    # Initialize scene and set-up static objects
    gold_material = pyrender.MetallicRoughnessMaterial(baseColorFactor=[1.0, 0.766, 0.336, 1.0],
                                                       roughnessFactor=.25,
                                                       metallicFactor=1,
                                                       emissiveFactor=0)
    scene = pyrender.Scene()
    magnification = 5
    camera = pyrender.OrthographicCamera(magnification, magnification * .75)
    camera_position = 2*np.array([3.25, -4.25, 2.5])
    origin = np.array([0, 0, 0])
    up_vector = np.array([0, 0, 1])

    # for implicit resolution
    unit_cube_size = 6
    n_grid_points_per_dimension = 96

    big_img_width, big_img_height = 144, 108  # 96, 72
    small_img_size = 48  # 32

    total_img_width, total_img_height = big_img_width, big_img_height + small_img_size
    renderer = pyrender.OffscreenRenderer(big_img_width, big_img_height)

    # render all the particles
    for sample in parameter_df.itertuples():
        print('shape %i / %i' % (sample.Index, len(parameter_df.index)), end='\r')
        print('Thumbnail generation %.2f percent complete. Generating thumbnail %i of %i. ' %
              ((100 * sample.Index / len(parameter_df.index)), sample.Index, len(parameter_df.index)), end='\r')

        X, Y, Z, tri_indices = nano_formula_3d(sample.m, sample.n1, sample.n2, sample.n3, sample.a, sample.b, 100000)
        vertices = np.column_stack((X, Y, Z))

        shape_mesh = trimesh.Trimesh(vertices=vertices, faces=tri_indices)
        obj_center = shape_mesh.centroid

        # shift camera position to center object
        camera_pose, left_vector = look_at(camera_position + obj_center, origin + obj_center, up_vector)
        camera_pose = np.array(inv(camera_pose))
        scene.add(camera, pose=camera_pose)

        # lighting
        scene.ambient_light = .01
        dir_light_obj = pyrender.DirectionalLight(color=[1, 1, 1], intensity=1000)

        spotlight_positions = [[-.1, -.1, 10], np.array([0, 0, 50]) - 8*left_vector]
        headlamp_offset = 20.0
        spotlight_positions.append(camera_position + headlamp_offset * left_vector + 1.5 * up_vector)
        spotlight_positions.append(camera_position - headlamp_offset * left_vector + 1.2 * up_vector)

        for light_pos in spotlight_positions:
            matrix, left_vector = look_at(light_pos, origin, up_vector)
            light_pose = np.array(inv(matrix))
            scene.add(dir_light_obj, pose=light_pose)

        filename = str(sample.Index + 1) + '.png'

        mesh = pyrender.Mesh.from_trimesh(shape_mesh, material=gold_material)
        scene.add(mesh)

        color, depth = renderer.render(scene)

        # clean up scene since done rendering this object
        scene.clear()

        # make image big enough for slices if adding
        if add_slices:
            pil_img = Image.new('RGB', (total_img_width, total_img_height))
        else:
            pil_img = Image.new('RGB', (big_img_width, big_img_height))

        color = [(pixel[0], pixel[1], pixel[2]) for pixel in color.reshape(-1, 3)]
        pil_img.putdata(color)

        # add slices:
        if add_slices:
            slice_img = Image.new('RGB', (n_grid_points_per_dimension, n_grid_points_per_dimension))
            grid = nano_3d_implicit(unit_cube_size, n_grid_points_per_dimension, sample.a, sample.b, sample.m,
                                    sample.n1, sample.n2, sample.n3)

            ortho_views = [get_outline(grid, 0), get_outline(grid, 1), get_outline(grid, 2)]
            back_colors = [(255, 222, 222), (222, 255, 222), (222, 222, 255)]

            for index, view in enumerate(ortho_views):
                flat_data = [color_binary(x, back_colors[index]) for x in view.flatten()]
                slice_img.putdata(flat_data)
                top_left = (index * small_img_size, big_img_height)
                pil_img.paste(slice_img.resize((small_img_size, small_img_size), resample=PIL.Image.BILINEAR), top_left)

        pil_img.save(output_directory + filename)


def nano_formula_3d(m, n1, n2, n3, a, b, num_points):
    num_points_root = round(math.sqrt(num_points))

    theta = np.linspace(-math.pi, math.pi, endpoint=True, num=num_points_root)
    phi = np.linspace(-math.pi / 2.0, math.pi/2.0, endpoint=True, num=num_points_root)

    theta, phi = np.meshgrid(theta, phi)
    theta, phi = theta.flatten(), phi.flatten()

    r1 = nano_formula_2d(m, n1, n2, n3, a, b, theta)
    r2 = nano_formula_2d(m, n1, n2, n3, a, b, phi)

    x = r1 * r2 * np.cos(theta) * np.cos(phi)
    y = r1 * r2 * np.sin(theta) * np.cos(phi)
    z = r2 * np.sin(phi)

    tri = mtri.Triangulation(theta, phi)
    return x, y, z, tri.triangles


def nano_3d_implicit(unit_cube_size, n_grid_points_per_dimension, a, b, m, n1, n2, n3):
    x = np.linspace(-unit_cube_size, unit_cube_size, n_grid_points_per_dimension)
    y = np.linspace(-unit_cube_size, unit_cube_size, n_grid_points_per_dimension)
    z = np.linspace(-unit_cube_size, unit_cube_size, n_grid_points_per_dimension)
    [x, y, z] = np.meshgrid(x, y, z)

    theta = np.arctan2(y, x)
    r_theta = nano_formula_2d(m, n1, n2, n3, a, b, theta)
    phi2 = np.arctan(z * r_theta * np.cos(theta) / x)
    r_phi = nano_formula_2d(m, n1, n2, n3, a, b, phi2)

    full_grid = 1 - (x**2 + y**2 + (r_theta**2) * (z**2)) / ((r_theta**2) * (r_phi**2))
    return full_grid


def nano_formula_2d(m, n1, n2, n3, a, b, theta):
    r = abs((1 / a) * np.cos(m * theta / 4.0))**n2 + abs((1 / b) * np.sin(m * theta / 4.0))**n3
    return r**(-1 / n1)


def get_outline(grid, view):
    size = np.shape(grid)[0]
    out = np.array([[0 for _ in range(size)] for _ in range(size) ])
    for x in range(size):
        for y in range(size):
            for depth in range(size):
                if view == 0:
                    val = grid[x,y, depth]
                elif view == 1:
                    val = grid[depth, y, x]
                else:
                    val = grid[y, depth, x]
                if val >= 0:
                    out[x, y] = 1
                    break
    return out


def get_color(val, min_val):
    if val == 0:
        return 255, 255, 255
    if val > 0:
        return 255, 0, 0

    return 0, 0, int(255 * val / float(min_val))


def color_binary(val, back=(255, 255, 255), fore=(0, 0, 0)):
    if val <= 0:
        return back
    return fore


def plot_heatmap(grid):
    fig, ax = plt.subplots()
    ax.imshow(grid, 'hot')
    fig.tight_layout()
    plt.show()
    plt.clf()


# from (with slight modification) https://community.khronos.org/t/view-and-perspective-matrices/74154
def look_at(eye, target, up):
    eye = np.array(eye)
    target = np.array(target)
    up = np.array(up)
    look_vector = eye[:3] - target[:3]
    look_vector = normalize(look_vector)
    U = normalize(up[:3])
    left_vector = np.cross(U, look_vector)
    up_vector = np.cross(look_vector, left_vector)
    M = np.array(np.identity(4))
    M[:3,:3] = np.vstack([left_vector, up_vector, look_vector])
    T = translate(-eye)
    return np.matmul(M, T), left_vector


def translate(xyz):
    x, y, z = xyz
    return np.array([[1, 0, 0, x],
                     [0, 1, 0, y],
                     [0, 0, 1, z],
                     [0, 0, 0, 1]])


def magnitude(v):
    return math.sqrt(np.sum(v ** 2))


def normalize(v):
    m = magnitude(v)
    if m == 0:
        return v
    return v / m

