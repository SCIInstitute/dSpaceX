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

"""

pvMeshRenderer

Renders a 2d image (a "screenshot") of a mesh.

- vertices can be independently updated, ex: for models with corresponding faces

- resolution can be specified when requesting a screehshot

- can produce multi-window views showing silouettes along axes along with
  traditional isometric (main) view

- camera position for main view can be specified
- RGB color for mesh in main view can be specified

"""
class pvMeshRenderer:
    def __init__(self,
                 datapath = './nanoparticles_mesh/unprocessed_data/shape_representations/',
                 default_mesh = '1.ply'):
        """
        Instantiate the mesh renderer class.
        """

        # Create a polydata and load the default mesh.
        self.polydata = vtk.vtkPolyData()
        self.loadNewMesh(datapath + default_mesh)

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

    #def updateFaces(self, faces = []):
    #   TODO? not sure this would ever make sense versus simply loading a new mesh
                
    def loadNewMesh(self, filename):
        """
        Loads a new mesh and updates self.polydata.
        """

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

    def update(self):
        print("pvRenderMesh.update (just calls renWin.Render())")
        if not hasattr(self, 'renWin'):
            self.createRenderer()
        self.renWin.Render()
        print("done!")

    def getImage(self):
        # update and return the vtk image
        self.w2if.Modified()
        self.w2if.Update()
        return self.w2if.GetOutput()
        
