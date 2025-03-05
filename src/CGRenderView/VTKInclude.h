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




#include <vtkRegularPolygonSource.h>
#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)


#endif // !_VTK_INCLUDE_H_
