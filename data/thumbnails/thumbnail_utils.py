import math
import numpy as np
from numpy.linalg import inv
from PIL import Image
import pyrender
import trimesh
from vtk.util import numpy_support
import vtk

def generate_image_from_vertices_and_faces(vertices, faces = []):
    """
    From the vertices and faces of a mesh generates a 2D thumbnail.
    :param vertices: The mesh vertices.
    :param faces: The mesh faces.
    """
    if len(faces) == 0:  # fixme: make this somehow sustainable; same with hard-coded values all over this function
        original_mesh_directory = '/Users/cam/data/dSpaceX/latest/nanoparticles_mesh/unprocessed_data/shape_representations/1.ply'
        print("faces not set, so using defaults from " + original_mesh_directory)
        original_mesh = trimesh.load_mesh(original_mesh_directory, process=False)
        faces = original_mesh.faces

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

class vtkRenderMesh:
    """
    Renders a 2d thumbnail from the vertices and faces of a mesh.
    """
    datapath = "/Users/cam/data/dSpaceX/latest/"

    def __init__(self, default_mesh = datapath + 'nanoparticles_mesh/unprocessed_data/shape_representations/1.ply'):
        self.polydata = vtk.vtkPolyData()

        # load a default mesh, setting self.vertices and self.faces to be used with polydata
        self.loadNewMesh(default_mesh)

        mapper = vtk.vtkPolyDataMapper()
        mapper.SetInputData(self.polydata)

        colors = vtk.vtkNamedColors()
        fgc = [1.0, 0.766, 0.336]
        #bgk = map(lambda x: x / 255.0, [26, 51, 102, 255])
        bkg = map(lambda x: x / 255.0, [255, 255, 255, 255])
        colors.SetColor("BkgColor", *bkg)

        actor = vtk.vtkActor()
        actor.SetMapper(mapper)
        #actor.GetProperty().SetColor(colors.GetColor3d("Tomato"))
        actor.GetProperty().SetColor(fgc)
        actor.RotateX(30.0)
        actor.RotateY(-45.0)

        # Create the graphics structure. The renderer renders into the render
        self.ren = vtk.vtkRenderer()
        self.renWin = vtk.vtkRenderWindow()
        self.renWin.AddRenderer(self.ren)
        #self.renWin.OffScreenRenderingOn()   # ***** don't open a window *****

        # Add the actors to the renderer, set the background and size
        self.ren.AddActor(actor)
        self.ren.SetBackground(colors.GetColor3d("BkgColor"))
        self.renWin.SetSize(300, 300)
        self.renWin.SetWindowName('TwistyTurnyNanoparticle')

        # We'll zoom in a little by accessing the camera and invoking a "Zoom"
        self.ren.ResetCamera()
        self.ren.GetActiveCamera().Zoom(5.0)

        # screenshot filter
        self.w2if = vtk.vtkWindowToImageFilter()
        self.w2if.SetInput(self.renWin)
        self.w2if.SetInputBufferTypeToRGB()
        self.w2if.ReadFrontBufferOff()

        # need to call this during initialization or the thread that calls updatemesh will crash opening the window
        self.update()

    def updateMesh(self, vertices = [], faces = []):
        print("welcome to vtkRenderMesh.updateMesh!")
        if len(faces) > 0:
            # TODO: process faces to produce polys for polydata
            # self.polys = ...
            None

        print("faces complete, time for vertices")

        if len(vertices) > 0:
            print("vertices: shape " + str(vertices.shape) + ", dtype " + str(vertices.dtype))
            vertices = vertices.reshape((-1, 3)) # <ctc> Controller needs to reshape before it sends to python function (TODO)
            print("vertices: shape " + str(vertices.shape) + ", dtype " + str(vertices.dtype))

            # initialize points
            self.points.SetNumberOfPoints(len(vertices))
            for i in range(len(vertices)):
                self.points.SetPoint(i, vertices[i])

        print("vertices complete, let's set self.polydata data next")

        self.polydata.SetPoints(self.points)
        self.polydata.SetPolys(self.polys)
        self.polydata.Modified()   #<ctc> may or may not be necessary... doesn't seem like it is in interpreter

        #print("finally, we'll call update, which calls self.renWin.Render()")
        #self.update() #don't do it! causes crash when called from non-main thread on server

    def loadNewMesh(self, filename):
        reader = vtk.vtkPLYReader()
        reader.SetFileName(filename)
        reader.Update() # when this function is called from c++, reader.Update() causes:
            # dSpaceX[24453:12650119] dynamic_cast error 1: Both of the following type_info's should have public visibility.
            #                                               At least one of them is hidden.
            #               NSt3__113basic_istreamIcNS_11char_traitsIcEEEE,
            #               NSt3__114basic_ifstreamIcNS_11char_traitsIcEEEE.
            # Super verbose `python -vvv` doesn't produce this error, so something with pybind11? Ignoring for now.
        self.points = reader.GetOutput().GetPoints()
        self.polys = reader.GetOutput().GetPolys()
        self.polydata.SetPolys(self.polys)
        self.polydata.SetPoints(self.points)

    def update(self):
        print("vtkRenderMesh.update (just calls renWin.Render())")
        self.renWin.Render()
        print("done!")

    def screenshot(self):
        # update and return a numpy array of the vtk image
        self.w2if.Modified()
        self.w2if.Update()
        return self.w2if.GetOutput()
        
    
def vtkRenderMesh_func(vertices, faces = []):
    """
    From the vertices and faces of a mesh generates a 2D thumbnail.
    :param vertices: The mesh vertices.
    :param faces: The mesh faces.
    """
    polydata = vtk.vtkPolyData()

    if len(faces) == 0:  # fixme: make this somehow sustainable; same with hard-coded values all over the place
        default_mesh = '/Users/cam/data/dSpaceX/latest/nanoparticles_mesh/unprocessed_data/shape_representations/1.ply'
        print("faces not set, so using defaults from " + default_mesh)
        reader = vtk.vtkPLYReader()
        reader.SetFileName(default_mesh)
        reader.Update() # when this function is called from c++, this line causes:
    # dSpaceX[24453:12650119] dynamic_cast error 1: Both of the following type_info's should have public visibility.
    #                                               At least one of them is hidden.
    #               NSt3__113basic_istreamIcNS_11char_traitsIcEEEE,
    #               NSt3__114basic_ifstreamIcNS_11char_traitsIcEEEE.
    # Super verbose `python -vvv` doesn't produce this error, so something with pybind11? Ignoring for now.
        polydata.SetPolys(reader.GetOutput().GetPolys())
    else:
        polydata.SetPolys(faces)
        
    #<ctc> let's see what that eigen matrix looks like...
    print("vertices: shape " + str(vertices.shape) + ", dtype " + str(vertices.dtype))

    # <ctc> hack since Controller needs to reshape before it sends to python function
    vertices = vertices.reshape((-1, 3))
    print("vertices: shape " + str(vertices.shape) + ", dtype " + str(vertices.dtype))

    # initialize points
    points = vtk.vtkPoints()
    points.SetNumberOfPoints(len(vertices))
    for i in range(len(vertices)):
        points.SetPoint(i, vertices[i])

    polydata.SetPoints(points)

    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputData(polydata)

    colors = vtk.vtkNamedColors()
    bkg = map(lambda x: x / 255.0, [26, 51, 102, 255])
    colors.SetColor("BkgColor", *bkg)

    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.GetProperty().SetColor(colors.GetColor3d("Tomato"))
    actor.RotateX(30.0)
    actor.RotateY(-45.0)

    # Create the graphics structure. The renderer renders into the render
    ren = vtk.vtkRenderer()
    renWin = vtk.vtkRenderWindow()
    renWin.AddRenderer(ren)
    renWin.OffScreenRenderingOn()   # ***** don't open a window *****

    # Add the actors to the renderer, set the background and size
    ren.AddActor(actor)
    ren.SetBackground(colors.GetColor3d("BkgColor"))
    renWin.SetSize(300, 300)
    renWin.SetWindowName('TwistyTurnyNanoparticle')

    # We'll zoom in a little by accessing the camera and invoking a "Zoom"
    ren.ResetCamera()
    ren.GetActiveCamera().Zoom(1.5)
    renWin.Render()

    # screenshot code:
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(renWin)
    w2if.SetInputBufferTypeToRGB()
    self.w2if.ReadFrontBufferOff()      # need to read from the back buffer for screenshots
    w2if.Update()

    # return the vtk image
    return w2if.GetOutput()

# <ctc> try this, but also just try attr("generate_image_from_vertices_and_faces").attr("pilImageToByes")() -> yay, works!
def getPILImage():
    """
    Returns the byte array of the uncompressed image.
    """
    original_img_directory = '/Users/cam/data/dSpaceX/latest/nanoparticles_mesh/processed_data/thumbnails/5.png'
    print("no image to convert, so using defaults from " + original_img_directory)
    return Image.open(original_img_directory)

def pilImageToBytes(img = None):
    """
    Returns the byte array of the uncompressed image.
    """
    if not img:
        img = getPILImage()
    return img.tobytes()

def writeVtkImage(img, filename):
    writer = vtk.vtkPNGWriter()
    writer.SetFileName(filename)
    writer.SetInputData(img)
    writer.Write()

def getVtkImage():
    pngreader = vtk.vtkPNGReader()
    pngreader.SetFileName("/tmp/1.png")
    pngreader.Update()
    return pngreader.GetOutput()

def vtkToNumpy(vtkimg = None):
    """
    Just a test: returns the byte array of a vtk image as a numpy array. Can Controller use it? Stay tuned...
    """
    if not vtkimg:
        vtkimg = getVtkImage()
    return numpy_support.vtk_to_numpy(vtkimg.GetPointData().GetScalars())

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
