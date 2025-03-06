#ifndef _VTK_INCLUDE_H_
#define _VTK_INCLUDE_H_

#include <vtkWin32OpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <vtkNamedColors.h>
#include <vtkProperty.h>
#include <vtkImageReader.h>
#include <vtkMarchingCubes.h>
#include <vtkStripper.h>
#include <vtkOutlineFilter.h>
#include <vtkImageReslice.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkImageData.h>
#include <vtkDataSetMapper.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkMatrix4x4.h>
#include <vtkImageActor.h>
#include <vtkImageCast.h>
#include <vtkImageMapper3D.h>
#include <vtkVolumeMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkThreshold.h>
#include <vtkOBJImporter.h>
#include <vtkTexture.h>
#include <vtkLight.h>
#include <vtkVersion.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkImageReader.h>
#include <vtkMarchingCubes.h>
#include <vtkStripper.h>
#include <vtkOutlineFilter.h>


#include <vtkRegularPolygonSource.h>
#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)


#endif // !_VTK_INCLUDE_H_
