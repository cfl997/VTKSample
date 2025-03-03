#include "CGWindowsWindow.h"
#include "VTKInclude.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkImageReader.h>
#include <vtkMarchingCubes.h>
#include <vtkStripper.h>
#include <vtkOutlineFilter.h>


using namespace CGRenderView;

struct CGWindowsWindow::PrivateData
{
	vtkSmartPointer<vtkRenderWindowInteractor> interactor;
	vtkSmartPointer<vtkRenderWindow> renderWindow;
	vtkSmartPointer<vtkRenderer> renderer;


	QVTKOpenGLNativeWidget* vtkWidget;
	HWND hwnd;

	/*
	* pushbutton
	*/
	vtkSmartPointer<vtkRenderer>rendererOne;
};

CGWindowsWindow::CGWindowsWindow(HWND hwnd, unsigned int width, unsigned int height) : m_priv(new PrivateData)
{
	auto& d = *m_priv;
	d.hwnd = hwnd;

	// 绑定 VTK 渲染窗口到 Windows 窗口 (HWND)
	d.vtkWidget = new QVTKOpenGLNativeWidget();
	d.renderWindow = d.vtkWidget->renderWindow();


	d.renderWindow->SetSize(width, height);

	// 创建 VTK 渲染器
	d.renderer = vtkSmartPointer<vtkRenderer>::New();
	d.renderWindow->AddRenderer(d.renderer);

	// 创建交互器
	d.interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	d.interactor->SetRenderWindow(d.renderWindow);

	// 设置背景色
	d.renderer->SetBackground(1, 0.2, 0.4);

	// 添加球体
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->SetCenter(0, 0, 0);
	sphereSource->SetRadius(0.5);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(sphereSource->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	d.renderer->AddActor(actor);

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	d.interactor->SetInteractorStyle(style);

	// 初始化渲染和交互器
	d.renderWindow->Render();
	d.interactor->Initialize();




	std::cout << "Render window initialized. Width: " << width << ", Height: " << height << std::endl;
	std::cout << "Background color: " << d.renderer->GetBackground()[0] << ", "
		<< d.renderer->GetBackground()[1] << ", "
		<< d.renderer->GetBackground()[2] << std::endl;

	/*
	* pushbutton
	*/
	d.rendererOne = vtkSmartPointer<vtkRenderer>::New();
	{

		vtkSmartPointer<vtkImageReader> reader = vtkSmartPointer<vtkImageReader>::New();
		reader->SetFileName("E:/A/program/vtk/Render3D/bin64/data/mri_woman_256x256x109_uint16.raw");
		reader->SetFileDimensionality(3);
		reader->SetDataScalarType(VTK_UNSIGNED_SHORT);
		reader->SetDataExtent(0, 255, 0, 255, 0, 108);
		reader->SetDataSpacing(0.9, 0.9, 2);
		reader->SetDataOrigin(0.0, 0.0, 0.0);
		reader->Update();

		if (!reader->GetOutput()) {
			std::cerr << "Error: Failed to load RAW file!" << std::endl;
			assert(0);
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

		d.rendererOne->AddActor(actor);
		d.rendererOne->AddActor(outlineActor);
		d.rendererOne->SetBackground(1, 1, 1);


	}

}

CGWindowsWindow::~CGWindowsWindow()
{
	delete m_priv;
}

void* CGRenderView::CGWindowsWindow::getRenderWindow()
{
	auto& d = *m_priv;
	return d.renderWindow->GetGenericWindowId();
}

void* CGRenderView::CGWindowsWindow::getQVTKOpenGLNativeWidget()
{
	auto& d = *m_priv;
	return d.vtkWidget;
}

void CGRenderView::CGWindowsWindow::start()
{
	auto& d = *m_priv;

	static bool first = true;
	if (first)
	{
		first = false;
		vtkNew<vtkNamedColors> colors;

		// 创建一个圆
		vtkNew<vtkRegularPolygonSource> polygonSource;
		polygonSource->GeneratePolygonOff();
		polygonSource->SetNumberOfSides(50);
		polygonSource->SetRadius(5);
		polygonSource->SetCenter(0, 0, 0);

		vtkNew<vtkPolyDataMapper> mapper;
		mapper->SetInputConnection(polygonSource->GetOutputPort());

		vtkNew<vtkActor> actor;
		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(colors->GetColor3d("Cornsilk").GetData());

		// 绑定到现有渲染器
		d.renderer->AddActor(actor);
		d.renderer->SetBackground(colors->GetColor3d("DarkGreen").GetData());

		// 渲染
		d.renderWindow->Render();
		d.interactor->Start();  // 进入交互模式
	}
	else
	{
		d.renderWindow->Render();
		d.interactor->ProcessEvents();
	}
}

void CGRenderView::CGWindowsWindow::pbone()
{
	auto& d = *m_priv;
	d.renderWindow->RemoveRenderer(d.renderer);
	d.renderWindow->AddRenderer(d.rendererOne);

	d.renderWindow->Render();
	d.rendererOne->ResetCamera();
	d.rendererOne->ResetCameraClippingRange();
}