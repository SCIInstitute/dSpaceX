"""pyvista_mesh_renderer

Render a mesh from a set of vertices and corresponding faces using PyVista
(which wraps vtk).  Like vtk, this uses opengl and must therefore be called from
main thread (e.g, when called from c++)

NOTE: This is the most up-to-date mesh [thumbnail] renderer.
- thumbnail_utils.generate_image_from_vertices_and_faces was the first
 o but it uses pyrender, which only works on Linux
- vtk_mesh_renderer was created to work across platforms
 o first to be successfully tied in with the C++ server using pybind11
 o results of rendering needed improvement (camera pos, material, lighting, etc)
 o used Jupyter notebooks (w/ an itkWidgets cameo) to accompish this
 o the vtk API is cumbersome, and PyVista was discovered while looking for help
- PyVista wraps vtkRenderer with a modern graphics API
 o it made it possible to achieve much better results with far less code
 o the other two renderers are now deprecated and will be removed

Author: Cameron Christensen
Created: 2020.10.21

"""

import pyvista
import vtk
import numpy

"""pvMeshRenderer

Renders a 2d image (a "screenshot") of a mesh.

- vertices can be independently updated, ex: for models with corresponding faces
- resolution can be specified when requesting a screehshot
- produce images showing mesh with optional axis-aligned silouettes along bottom
- camera position for main view can be specified
- RGB color for mesh in main view can be specified

"""
class pvMeshRenderer:
    def __init__(self, default_mesh = '1.ply', scale = 1.3,
                 onscreen = False, singleview = False):
        """
        :param offscreet: don't show a window
        :param singleview: render mesh only without silouettes along bottom

        """
        
        self.scale = scale
        shape = (1,1) if singleview else "1/3"

        # Create a polydata and load the default mesh.
        self.polydata = vtk.vtkPolyData()
        self.plotter = pyvista.Plotter(notebook=False, off_screen=not onscreen,
                                       shape=shape, border=True)
        self.plotter.set_background([1,1,1])

        if default_mesh:
            self.loadNewMesh(default_mesh)

        # show interactive view if onscreen
        if onscreen:
            self.plotter.show()

    def loadNewMesh(self, filename, color = [1.0, 0.766, 0.336]):
        """
        Loads a new mesh and updates self.polydata.
        :param filename: mesh to load
        :param color: color of mesh to be rendered
        """

        # make sure file can be read
        if (not vtk.vtkPLYReader().CanReadFile(filename)):
            print("ERROR: cannot load " + filename)
            return
            
        reader = vtk.vtkPLYReader()
        reader.SetFileName(filename)
        reader.Update() # when this function is called from c++, reader.Update() causes:
            # dSpaceX[24453:12650119] dynamic_cast error 1: Both of the following type_info's should have public visibility.
            #                                               At least one of them is hidden.
            #               NSt3__113basic_istreamIcNS_11char_traitsIcEEEE,
            #               NSt3__114basic_ifstreamIcNS_11char_traitsIcEEEE.
            # Super verbose `python -vvv` doesn't produce this error, so something with pybind11? Ignoring for now.

        self.polydata.SetPolys(reader.GetOutput().GetPolys())
        self.polydata.SetPoints(reader.GetOutput().GetPoints())

        # axis-aligned silouette views
        if self.plotter.shape == (4,):
            # xz view
            self.plotter.subplot(0)
            self.plotter.add_mesh(self.polydata, color=[0,0,0], name="sample")

            # xy view
            self.plotter.subplot(1)
            self.plotter.add_mesh(self.polydata, color=[0,0,0], name="sample")

            # yz view
            self.plotter.subplot(2)
            self.plotter.add_mesh(self.polydata, color=[0,0,0], name="sample")

            # isometric view
            self.plotter.subplot(3)

        # since it uses smooth shading, it creates a copy of polydata, so it
        # must be re-added when vertices are updated (so save the color)
        self.color = color
        self.plotter.add_mesh(self.polydata, color=self.color, specular=0.5,
                              specular_power=15, smooth_shading=True, name="sample")

        self.setCameraPos()

    def updateVertices(self, vertices = []):
        """
        Updates vertices of the current mesh (self.polydata).
        """

        if len(vertices) > 0:
            # ensure vertices has the right shape
            vertices = vertices.reshape((-1, 3))

            # initialize points
            points = self.polydata.GetPoints()
            points.SetNumberOfPoints(len(vertices))
            for i in range(len(vertices)):
                points.SetPoint(i, vertices[i])

        self.plotter.add_mesh(self.polydata, color=self.color, specular=0.5,
                              specular_power=15, smooth_shading=True, name="sample")
        self.setCameraPos()

    #def updateFaces(self, faces = []):
    #   TODO? not sure this would ever make sense versus simply loading a new mesh
                
    def getCameraPos(self):
        return self.plotter.camera_position

    def setCameraPos(self, camera_pos = None):
        """
        Sets camera position in main view.
        By default, use a position slightly closer than isometric_view.
        """

        # reset camera pos in silhouette views
        if self.plotter.shape == (4,):
            # xz view
            self.plotter.subplot(0)
            self.plotter.view_xz(negative=True)

            # xy view
            self.plotter.subplot(1)
            self.plotter.view_xy(negative=True)

            # yz view
            self.plotter.subplot(2)
            self.plotter.view_yz(negative=True)

            # isometric view
            self.plotter.subplot(3)

        # set camera pos in main view
        if not camera_pos:
            self.plotter.isometric_view()
            lf = numpy.asarray(self.plotter.camera_position[0])
            la = numpy.asarray(self.plotter.camera_position[1])
            vup = numpy.asarray(self.plotter.camera_position[2])
            pos = (lf - la) / self.scale
            self.plotter.camera_position = (pos, la, vup)
        else:
            self.plotter.camera_position = camera_pos    

    def getImage(self, resolution = [300,300]):
        """
        Returns a numpy array of the current view.
        """

        img = self.plotter.screenshot(return_img=True, window_size=resolution)

        # must return a copy or it gets lost on the way to the c++ caller
        return img.copy()


