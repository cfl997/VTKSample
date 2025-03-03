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



#include <vtkRegularPolygonSource.h>
#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

#endif // !_VTK_INCLUDE_H_
