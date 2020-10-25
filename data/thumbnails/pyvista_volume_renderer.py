"""pyvista_volume_renderer

Render a volume using PyVista (which wraps vtk).  Like vtk, this uses opengl and
must therefore be called from main thread (e.g, when called from c++)

- This follows the interface of pyvista_mesh_renderer and is the most up-to-date
  volume [thumbnail] renderer.
- PyVista wraps vtkRenderer with a modern graphics API
 o it enables much better results with far less code

Author: Cameron Christensen
Created: 2020.10.23

"""

import pyvista
import nrrd
import numpy

"""pvVolumeRenderer

Renders a 2d image (a "screenshot") of a volume.

- resolution can be specified when requesting a screehshot
- produce images showing volume with optional axis-aligned silouettes along bottom
- camera position for main view can be specified
- RGB color for volume in main view can be specified

"""
class pvVolumeRenderer:
    def __init__(self, default = '1.nrrd', color = [1.0, 0.766, 0.336], scale = 1.3,
                 onscreen = False, singleview = False):
        """
        :param offscreet: don't show a window
        :param singleview: render volume only without silouettes along bottom

        """
        
        self.color = color
        self.scale = scale
        shape = (1,1) if singleview else "1/3"

        # Create plotter and load the default volume.
        self.plotter = pyvista.Plotter(notebook=False, off_screen=not onscreen,
                                       shape=shape, border=True)
        self.plotter.set_background([1,1,1])

        if default:
            self.loadNewVolume(default)

        # show interactive view if onscreen
        if onscreen:
            self.plotter.show()

    def loadNewVolume(self, filename):
        """
        Loads a new volume and updates self.polydata.
        :param filename: volume to load
        :param color: color of volume to be rendered
        """

        try:
            vol = nrrd.read(filename)
        except:
            print("ERROR: cannot load " + filename)
            return
            
        # axis-aligned silouette views
        if self.plotter.shape == (4,):
            # xz view
            self.plotter.subplot(0)
            self.plotter.add_volume(vol[0], name="sample")
            #self.plotter.add_volume(self.polydata, color=[0,0,0], name="sample")

            # xy view
            self.plotter.subplot(1)
            self.plotter.add_volume(vol[0], name="sample")
            #self.plotter.add_volume(self.polydata, color=[0,0,0], name="sample")

            # yz view
            self.plotter.subplot(2)
            self.plotter.add_volume(vol[0], name="sample")
            #self.plotter.add_volume(self.polydata, color=[0,0,0], name="sample")

            # isometric view
            self.plotter.subplot(3)

        self.plotter.add_volume(vol[0], name="sample")
        # self.plotter.add_volume(self.polydata, color=self.color, specular=0.5,
        #                       specular_power=15, smooth_shading=True, name="sample")

        self.setCameraPos()

    def update(self, param = []):
        self.updateVolume(param)

    def updateVolume(self, vol = []):
        """
        Updates vertices of the current volume (self.polydata).
        """

        if len(vol) == 0:
            print("ERROR: empty volume, ignoring")
            return

        self.plotter.add_volume(vol[0], name="sample")
        # self.plotter.add_volume(self.polydata, color=self.color, specular=0.5,
        #                       specular_power=15, smooth_shading=True, name="sample")
        self.setCameraPos()
                
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


