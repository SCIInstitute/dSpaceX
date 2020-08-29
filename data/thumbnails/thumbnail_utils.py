import math
import numpy as np
from numpy.linalg import inv
from PIL import Image
import pyrender
import trimesh


def generate_image_from_vertices_and_faces(vertices, faces):
    """
    From the vertices and faces of a mesh generates a 2D thumbnail.
    :param vertices: The mesh vertices.
    :param faces: The mesh faces.
    """
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

    image_width, image_height = 144, 108
    renderer = pyrender.OffscreenRenderer(image_width, image_height)

    # render mesh
    shape_mesh = trimesh.Trimesh(vertices=vertices, faces=faces, process=False)
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

    mesh = pyrender.Mesh.from_trimesh(shape_mesh, material=gold_material)
    scene.add(mesh)

    color, depth = renderer.render(scene)

    # clean up scene since done rendering this object
    scene.clear()

    # make image big enough for slices if adding
    pil_img = Image.new('RGB', (image_width, image_height))

    color = [(pixel[0], pixel[1], pixel[2]) for pixel in color.reshape(-1, 3)]
    pil_img.putdata(color)

    return pil_img


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
