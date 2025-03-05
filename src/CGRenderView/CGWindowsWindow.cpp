#include "CGWindowsWindow.h"
#include "VTKInclude.h"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkImageReader.h>
#include <vtkMarchingCubes.h>
#include <vtkStripper.h>
#include <vtkOutlineFilter.h>
#include "VTKReader3D.h"

using namespace CGRenderView;

struct CGWindowsWindow::PrivateData
{
	vtkSmartPointer<vtkRenderWindowInteractor> interactor;
	vtkSmartPointer<vtkRenderWindow> renderWindow;
	vtkSmartPointer<vtkRenderer> renderer;

	VTKRender3D* render3D = nullptr;

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
		d.render3D = new CGRenderView::VTKRender3D(d.rendererOne);
		d.render3D->loadFile("E:/A/program/github/VTKSample/bin64/data/mri_woman_256x256x109_uint16.raw", 256, 256, 109);
	}

}

CGWindowsWindow::~CGWindowsWindow()
{
	if (m_priv->render3D)
	{
		delete m_priv->render3D;
		m_priv->render3D = nullptr;
	}
	delete m_priv;
	m_priv = nullptr;
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

void CGRenderView::CGWindowsWindow::pbSlice(int x, int y, int z, int direction)
{
	auto& d = *m_priv;
	d.render3D->slice(x, y, z, direction);

	d.renderWindow->Render();
	d.rendererOne->ResetCamera();
	d.rendererOne->ResetCameraClippingRange();
}

void CGRenderView::CGWindowsWindow::pbChangeColor()
{
	auto& d = *m_priv;

	d.render3D->changeColor(1);

	d.renderWindow->Render();
	d.rendererOne->ResetCamera();
	d.rendererOne->ResetCameraClippingRange();
}