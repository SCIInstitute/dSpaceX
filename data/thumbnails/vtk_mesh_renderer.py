"""
Render a mesh from a set of vertices and corresponding faces using vtk.
Uses opengl and must therefore be called from main thread (e.g, when called from c++)
"""
import numpy as np
from vtk.util import numpy_support
import vtk

class vtkMeshRenderer:
    """
    Renders a 2d thumbnail from the vertices and faces of a mesh.
    """
    def __init__(self, datapath = ".", default_mesh = '/nanoparticles_mesh/unprocessed_data/shape_representations/1.ply'):
        """
        Instantiate the mesh renderer class.
        """
        print("instantiating the vtkMeshRenderer class... \n\tdatapath: " + datapath + "\n\tdefault_mesh: " + default_mesh);

        # Create a polydata and load a default mesh.
        self.polydata = vtk.vtkPolyData()
        self.loadNewMesh(datapath + default_mesh)

    def createRenderer(self):
        """
        Create the renderer, render window, etc. 
        This function is placed outside the constructor since it must be called by the main thread.
        """
        print("vtkMeshRenderer.createRenderer()...")
        
        # Create an actor and default colors with which to render.
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
        self.renWin.OffScreenRenderingOn()   # ***** don't open a window *****

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
        """
        Sets self.vertices and self.faces to be used with this instance's polydata.
        """
        print("vtkMeshRenderer.loadNewMesh(" + filename + ")...");
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
        print("done loading mesh");

    def update(self):
        print("vtkRenderMesh.update (just calls renWin.Render())")
        if not hasattr(self, 'renWin'):
            self.createRenderer()
        self.renWin.Render()
        print("done!")

    def screenshot(self):
        # update and return a numpy array of the vtk image
        self.w2if.Modified()
        self.w2if.Update()
        return self.w2if.GetOutput()
        
def readVtkImage():
    pngreader = vtk.vtkPNGReader()
    pngreader.SetFileName("/tmp/1.png")
    pngreader.Update()
    return pngreader.GetOutput()

def writeVtkImage(img, filename):
    writer = vtk.vtkPNGWriter()
    writer.SetFileName(filename)
    writer.SetInputData(img)
    writer.Write()

def vtkToNumpy(vtkimg):
    """
    Returns the byte array of a vtk image as a numpy array. Used by Controller.
    """
    if vtkimg:
        return numpy_support.vtk_to_numpy(vtkimg.GetPointData().GetScalars())
    else:
        return np.empty([1,1], dtype = np.uint8);

