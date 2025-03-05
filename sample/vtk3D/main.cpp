#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>
#include <vtkImageReader.h>
#include <vtkMarchingCubes.h>
#include <vtkStripper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkOutlineFilter.h>
#include <vtkActor.h>
#include <vtkCommand.h>  // ✅ 监听窗口关闭事件
#include <vtkCallbackCommand.h>  // ✅ 添加回调命令
#include <thread>
#include <chrono>
#include <iostream>

void ExitCallback(vtkObject* caller, unsigned long, void*, void*)
{
    std::cout << "VTK Window Closed." << std::endl;
}

int main()
{
    std::cout << "Initializing VTK..." << std::endl;

    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
    renWin->AddRenderer(ren);
    renWin->SetWindowName("VTK 3D Rendering"); // ✅ 设置窗口标题
    renWin->SetSize(800, 800); // ✅ 确保窗口大小

    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(renWin);
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    iren->SetInteractorStyle(style);

    // 创建退出事件回调
    vtkSmartPointer<vtkCallbackCommand> exitCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    exitCallback->SetCallback(ExitCallback);  // 绑定回调函数

    // 监听退出事件
    iren->AddObserver(vtkCommand::ExitEvent, exitCallback);

    // 读取 RAW 图像数据
    vtkSmartPointer<vtkImageReader> reader = vtkSmartPointer<vtkImageReader>::New();
    reader->SetFileName("E:/A/program/github/VTKSample/bin64/data/mri_woman_256x256x109_uint16.raw");
    reader->SetFileDimensionality(3);
    reader->SetDataScalarType(VTK_UNSIGNED_SHORT);
    reader->SetDataExtent(0, 255, 0, 255, 0, 108);
    reader->SetDataSpacing(1., 1., 2);
    reader->SetDataOrigin(0.0, 0.0, 0.0);
    reader->Update();

    if (!reader->GetOutput()) {
        std::cerr << "Error: Failed to load RAW file!" << std::endl;
        return -1;
    }
    std::cout << "RAW file loaded successfully!" << std::endl;

    vtkSmartPointer<vtkMarchingCubes> marchingcube = vtkSmartPointer<vtkMarchingCubes>::New();
    marchingcube->SetInputConnection(reader->GetOutputPort());
    marchingcube->ComputeNormalsOn();
    marchingcube->ComputeGradientsOn();
    marchingcube->SetValue(0, 500);

    vtkSmartPointer<vtkStripper> Stripper = vtkSmartPointer<vtkStripper>::New();
    Stripper->SetInputConnection(marchingcube->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper> polyMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    polyMapper->SetInputConnection(Stripper->GetOutputPort());
    polyMapper->ScalarVisibilityOff();

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(polyMapper);
    actor->GetProperty()->SetDiffuseColor(1, 0.19, 0.15);
    actor->GetProperty()->SetSpecular(0.1);
    actor->GetProperty()->SetSpecularPower(10);
    actor->GetProperty()->SetColor(1, 0, 0);

    vtkSmartPointer<vtkOutlineFilter> outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlinefilter->SetInputConnection(reader->GetOutputPort());

    vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());

    vtkSmartPointer<vtkActor> outlineActor = vtkSmartPointer<vtkActor>::New();
    outlineActor->SetMapper(outlineMapper);
    outlineActor->GetProperty()->SetColor(0, 0, 0);

    ren->AddActor(actor);
    ren->AddActor(outlineActor);
    ren->SetBackground(1, 1, 1);
    renWin->SetSize(800, 800);

    // 初始化交互器并启动事件循环
    iren->Initialize();
    renWin->Render(); // ✅ 先渲染一次，确保窗口显示
    ren->ResetCamera();
    ren->ResetCameraClippingRange();

    std::cout << "Starting rendering..." << std::endl;

    // 启动事件循环
    iren->Start();  // ✅ 进入交互模式，不退出

    // 这里的代码应在窗口关闭后才会执行
    std::cout << "Exited interaction loop." << std::endl;
    return 0;
}
