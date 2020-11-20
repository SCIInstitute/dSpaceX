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
    def __init__(self, default = '', color = [1.0, 0.766, 0.336], scale = 1.3):

        # Create the importer and load a volume (gets things warmed up)
        self.dataImporter = vtk.vtkImageImport()
        if len(default) == 0:
            default = '/Users/cam/data/dSpaceX/latest/nanoparticles_volume/unprocessed_data/shape_representations/Images.0002.nrrd'
        self.loadNewVolume(default)

        # TODO: add some shadows and material properties
        # self.plotter.add_volume(evol.copy(), name="sample", show_scalar_bar=False, cmap='binary',
        #                         shade=True, diffuse=1.0, specular=0.5, specular_power=15)


        # colors
        colors = vtk.vtkNamedColors()

        # Create transfer mappings of scalar value to opacity and color.
        # from a (the only?!) vtk example, a proton field
        # opacityTransferFunction = vtk.vtkPiecewiseFunction()
        # opacityTransferFunction.AddPoint(20, 0.0)
        # opacityTransferFunction.AddPoint(255, 0.2)
        # colorTransferFunction = vtk.vtkColorTransferFunction()
        # colorTransferFunction.AddRGBPoint(0.0, 0.0, 0.0, 0.0)
        # colorTransferFunction.AddRGBPoint(64.0, 1.0, 0.0, 0.0)
        # colorTransferFunction.AddRGBPoint(128.0, 0.0, 0.0, 1.0)
        # colorTransferFunction.AddRGBPoint(192.0, 0.0, 1.0, 0.0)
        # colorTransferFunction.AddRGBPoint(255.0, 0.0, 0.2, 0.0)

        # blue to orange from ken morland's site
        # colorTransferFunction.SetColorSpaceToDiverging()
        # colorTransferFunction.AddRGBPoint(0.0, 0.06, 0.04, 0.4);
        # colorTransferFunction.AddRGBPoint(1.0, 0.811765, 0.345098, 0.113725);
        

        # volume properties
        volumeProperty = vtk.vtkVolumeProperty()

        alphaChannelFunc = vtk.vtkPiecewiseFunction()
        alphaChannelFunc.AddPoint(0, 0.0)
        alphaChannelFunc.AddPoint(255, 0.25)
        colorFunc = vtk.vtkColorTransferFunction()
        #colorFunc.AddRGBPoint(0, 0.0, 0.0, 0.0)
        colorFunc.AddRGBPoint(0, color[0]/2.0, color[1]/2.0, color[2]/2.0)
        colorFunc.AddRGBPoint(255, color[0], color[1], color[2])

        # volumeProperty.SetColor(colorTransferFunction)
        # volumeProperty.SetScalarOpacity(opacityTransferFunction)
        volumeProperty.SetColor(colorFunc)
        volumeProperty.SetScalarOpacity(alphaChannelFunc)
        volumeProperty.ShadeOn()
        volumeProperty.SetInterpolationTypeToLinear()
        volumeProperty.SetDiffuse(2.0)
        volumeProperty.SetSpecular(1.5)
        volumeProperty.SetSpecularPower(15)
        
        # self.volumeProperty.SetScalarOpacity(self.alphaChannelFunc)
        # self.volumeProperty.SetColor(self.colorFunc)

        # self.volumeProperty.SetColor(colors.GetColor("CustomColor"))


        # volume
        #volumeMapper = vtk.vtkFixedPointVolumeRayCastMapper()
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
        cam = self.ren.GetActiveCamera()
        # cam.Azimuth(-37.5)
        # cam.Elevation(30.0)
        # cam.SetPosition([1,0.5,1])
        # cam.SetFocalPoint([0,0,0])
        #cam.SetDistance(50)
        cam.SetViewUp([-1,0,0])
        cam.SetPosition([140,140,140])
        cam.SetFocalPoint([49.5,49.5,49.5])
        cam.Zoom(scale)

        print(cam.GetPosition())
        print(cam.GetFocalPoint())
        print(cam.GetViewUp())
        self.ren.ResetCameraClippingRange()
        #self.ren.ResetCamera()
        print(cam.GetPosition())
        print(cam.GetFocalPoint())
        print(cam.GetViewUp())

        self.renWin.SetWindowName('TwistyTurnyNanoparticle')

        # screenshot filter
        self.w2if = vtk.vtkWindowToImageFilter()
        self.w2if.SetInput(self.renWin)
        self.w2if.SetInputBufferTypeToRGB()
        self.w2if.ReadFrontBufferOff()

    def loadNewVolume(self, filename):
        try:
            vol = nrrd.read(filename)
        except:
            print("ERROR: cannot load " + filename)
            return
            
        self.n = 0
        self.update(vol[0])

    def update(self, param = []):
        self.updateVolume(param)

    def updateVolume(self, vol = [], shape = []):
        if len(vol) == 0:
            print("ERROR: empty volume, ignoring")
            return

        # <ctc> temp: write what we get to experiment more with rendering
        # outfile = "/tmp/" + f'{self.n:04}' + ".bin"
        # np.asarray(vol).tofile(outfile)
        # self.n += 1

        # can we do this from c++? or just pass vol dims
        if len(shape) != 0:
            print("shape passed: " + str(shape))
            evol = np.reshape(vol, shape)    # todo: just set vol=np.reshape... (just need to make sure it works with c++ passed data)
            arr = (evol * 255).astype('uint8')
        else:
            arr = (vol * 255).astype('uint8')

        print("volume shape: " + str(vol.shape))
        data_string = arr.tobytes()
        self.dataImporter.CopyImportVoidPointer(data_string, len(data_string))
        self.dataImporter.SetDataScalarTypeToUnsignedChar()
        self.dataImporter.SetNumberOfScalarComponents(1)
        self.dataImporter.SetDataExtent(0, arr.shape[0]-1, 0, arr.shape[1]-1, 0, arr.shape[2]-1)
        self.dataImporter.SetWholeExtent(0, arr.shape[0]-1, 0, arr.shape[1]-1, 0, arr.shape[2]-1)

    def getImage(self, resolution = [300,300]):
        # update and return the vtk image
        self.renWin.SetSize(resolution)
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
