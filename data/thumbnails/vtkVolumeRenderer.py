"""
Render a volume using vtk.
"""
import numpy as np
from vtk.util import numpy_support
import vtk
import nrrd

class vtkVolumeRenderer:
    """
    Renders a 2d thumbnail from a volume
    """
    def __init__(self, default = '', color = [1.0, 0.766, 0.336]):

        # Create the importer and load a volume (gets things warmed up)
        self.dataImporter = vtk.vtkImageImport()
        if len(default) == 0:
            # a dataset's config.yaml will specify the name of the default volume and it'll be included
            default = '/Users/cam/data/dSpaceX/latest/nanoparticles_volume/unprocessed_data/shape_representations/Images.0002.nrrd'
        self.loadNewVolume(default)

        # colors
        colors = vtk.vtkNamedColors()

        # volume properties
        volumeProperty = vtk.vtkVolumeProperty()

        alphaChannelFunc = vtk.vtkPiecewiseFunction()
        alphaChannelFunc.AddPoint(0, 0.0)
        alphaChannelFunc.AddPoint(228, 0.0)
        alphaChannelFunc.AddPoint(255, 0.25)
        colorFunc = vtk.vtkColorTransferFunction()
        colorFunc.AddRGBPoint(0, color[0]/2.0, color[1]/2.0, color[2]/2.0)
        colorFunc.AddRGBPoint(255, color[0], color[1], color[2])

        volumeProperty.SetColor(colorFunc)
        volumeProperty.SetScalarOpacity(alphaChannelFunc)
        volumeProperty.ShadeOn()
        volumeProperty.SetInterpolationTypeToLinear()
        volumeProperty.SetDiffuse(2.0)
        volumeProperty.SetSpecular(1.5)
        volumeProperty.SetSpecularPower(15)
        
        # volume
        volumeMapper = vtk.vtkGPUVolumeRayCastMapper()
        volumeMapper.SetInputConnection(self.dataImporter.GetOutputPort())
        self.volume = vtk.vtkVolume()
        self.volume.SetMapper(volumeMapper)
        self.volume.SetProperty(volumeProperty)

        # renderer
        self.ren = vtk.vtkRenderer()
        self.renWin = vtk.vtkRenderWindow()
        self.renWin.AddRenderer(self.ren)
        self.renWin.OffScreenRenderingOn()   # ***** don't open a window *****

        # add volume, set background and size
        self.ren.AddVolume(self.volume)
        self.ren.SetBackground(colors.GetColor3d("Wheat"))

        # camera
        self.resetCamera()

        self.renWin.SetWindowName('TwistyTurnyNanoparticle')

        # screenshot filter
        self.w2if = vtk.vtkWindowToImageFilter()
        self.w2if.SetInput(self.renWin)
        self.w2if.SetInputBufferTypeToRGB()
        self.w2if.ReadFrontBufferOff()

    def resetCamera(self, scale = 1.25):
        self.ren.ResetCamera()
        cam = self.ren.GetActiveCamera()
        cam.Zoom(scale)
        cam.SetViewUp([-1,0,0])
        cam.SetPosition([140,140,140])
        cam.SetFocalPoint([49.5,49.5,49.5])
        self.ren.ResetCameraClippingRange()

    def loadNewVolume(self, filename):
        try:
            vol = nrrd.read(filename)
            self.shape = vol[1]['sizes']    # use this if nothing passed to update (and hope array passed is same size)
        except:
            print("ERROR: cannot load " + filename)
            return
            
        self.n = 0
        self.update(vol[0])

    def show(self):
        renderInteractor = vtk.vtkRenderWindowInteractor()
        renderInteractor.SetRenderWindow(self.renWin)

        # A simple function to be called when the user decides to quit the application.
        def exitCheck(obj, event):
            if obj.GetEventPending() != 0:
                obj.SetAbortRender(1)

        # Tell the application to use the function as an exit check.
        self.renWin.AddObserver("AbortCheckEvent", exitCheck)

        renderInteractor.Initialize()
        # Because nothing will be rendered without any input, we order the first render manually
        #  before control is handed over to the main-loop.
        renderInteractor.Start()

    def update(self, vol = [], shape = []):
        # can we do this from c++? or just pass vol dims
        if len(shape) != 0:
            vol = np.reshape(vol, shape)
        else:
            # if shape not passed use last loaded volume's dims (and hope they're the same)
            vol = np.reshape(vol, self.shape)

        self.updateVolume(vol)

    def updateVolume(self, vol = []):
        if len(vol) == 0:
            print("ERROR: empty volume, ignoring")
            return

        # <ctc> temp: write what we get to experiment more with rendering
        # outfile = "/tmp/" + f'{self.n:04}' + ".bin"
        # np.asarray(vol).tofile(outfile)
        # self.n += 1

        arr = (vol * 255).astype('uint8')

        data_string = arr.tobytes()
        self.dataImporter.CopyImportVoidPointer(data_string, len(data_string))
        self.dataImporter.SetDataScalarTypeToUnsignedChar()
        self.dataImporter.SetNumberOfScalarComponents(1)
        self.dataImporter.SetWholeExtent(0, arr.shape[0]-1, 0, arr.shape[1]-1, 0, arr.shape[2]-1)
        self.dataImporter.SetDataExtent(0, arr.shape[0]-1, 0, arr.shape[1]-1, 0, arr.shape[2]-1)

    def getImage(self, resolution = [300,300], scale = 1.25):
        # update and return the vtk image
        self.renWin.SetSize(resolution)
        self.resetCamera(scale)
        self.renWin.Render()
        self.w2if.Modified()
        self.w2if.Update()

        return vtkToNumpy(self.w2if.GetOutput(), resolution)
        
def vtkToNumpy(vtkimg, xyres = []):
    """
    Returns the byte array of a vtk image as a numpy array. Used by Controller.
    """
    if vtkimg:
        img = numpy_support.vtk_to_numpy(vtkimg.GetPointData().GetScalars())
        if (len(xyres) == 0):
            xyres = img.shape[0] / 3  # assumes rgb
            xyres = [sqrt(xyres), sqrt(xyres)]

        return img.reshape([xyres[1], xyres[0], 3])

    else:
        return np.empty([1,1], dtype = np.uint8)
