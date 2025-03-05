#include "VTKReader3D.h"
#include "VTKInclude.h"
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


struct CGRenderView::VTKRender3D::PrivateData
{
	vtkSmartPointer<vtkImageReader> reader;
	vtkSmartPointer<vtkRenderer> render;
	float Spacing = 1.0f;
};

CGRenderView::VTKRender3D::VTKRender3D(vtkSmartPointer<vtkRenderer> render) :m_priv(new PrivateData)
{
	auto& d = *m_priv;
	d.render = render;
}

CGRenderView::VTKRender3D::~VTKRender3D()
{
	delete m_priv;
	m_priv = nullptr;
}

bool CGRenderView::VTKRender3D::loadFile(const char* filename, int x, int y, int z, float Spacing)
{
	auto& d = *m_priv;
	d.Spacing = Spacing;
	d.reader = vtkSmartPointer<vtkImageReader>::New();
	d.reader->SetFileName(filename);
	d.reader->SetFileDimensionality(3);
	d.reader->SetDataScalarType(VTK_UNSIGNED_SHORT);
	d.reader->SetDataExtent(0, x - 1, 0, y - 1, 0, z - 1);
	d.reader->SetDataSpacing(Spacing, Spacing, Spacing * 2);
	d.reader->SetDataOrigin(0.0, 0.0, 0.0);
	d.reader->Update();

	if (!d.reader->GetOutput()) {
		std::cerr << "Error: Failed to load RAW file!" << std::endl;
		assert(0);
		return false;
	}
	std::cout << "RAW file loaded successfully!" << std::endl;

	vtkSmartPointer<vtkMarchingCubes> marchingcube = vtkSmartPointer<vtkMarchingCubes>::New();
	marchingcube->SetInputConnection(d.reader->GetOutputPort());
	marchingcube->ComputeNormalsOn();
	marchingcube->ComputeGradientsOn();
	marchingcube->SetValue(0, 500);

	/*
	*  三角带（triangle strips）
	*/
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
	outlinefilter->SetInputConnection(d.reader->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());

	vtkSmartPointer<vtkActor> outlineActor = vtkSmartPointer<vtkActor>::New();
	outlineActor->SetMapper(outlineMapper);
	outlineActor->GetProperty()->SetColor(0, 1, 0);

	d.render->AddActor(actor);
	d.render->AddActor(outlineActor);
	return true;
}

bool CGRenderView::VTKRender3D::slice(int x, int y, int z, int direction)
{
	auto& d = *m_priv;

	// 清空之前的 actor
	d.render->RemoveAllViewProps();

	// 检查输入数据
	vtkImageData* imageData = d.reader->GetOutput();
	if (!imageData) {
		std::cerr << "错误：没有加载数据！" << std::endl;
		return false;
	}

	// 打印图像范围和值范围
	int* extent = imageData->GetExtent();
	double* range = imageData->GetScalarRange();
	std::cout << "图像范围: "
		<< extent[0] << " " << extent[1] << " "
		<< extent[2] << " " << extent[3] << " "
		<< extent[4] << " " << extent[5] << std::endl;
	std::cout << "图像值范围: " << range[0] << " " << range[1] << std::endl;


	// 获取图像的基本信息
	//int extent[6];
	double spacing[3];
	double origin[3];
	double center[3];

	d.reader->GetOutput()->GetExtent(extent);
	d.reader->GetOutput()->GetSpacing(spacing);
	d.reader->GetOutput()->GetOrigin(origin);

	center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
	center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
	center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);

	//定义不同方向的切片矩阵
   /*
   * 1. Sagittal (矢状面) 切片矩阵：
   * （左右切片），通常沿 X 轴 方向进行切片。
   */
	static double sagittalElements[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	/*
	* 2. Coronal (冠状面) 切片矩阵：
	* （前后切片），通常沿 Y 轴 方向进行切片。
	*/
	static double coronalElements[16] = {
		1, 0, 0, 0,
		0, 0, 1, 0,
		0, -1, 0, 0,
		0, 0, 0, 1
	};

	/*
	* 3. Axial (横断面) 切片矩阵：
	* （上面切片），通常沿 Z 轴 方向进行切片。
	*/
	static double axialElements[16] = {
		0, 0, 1, 0,
		1, 0, 0, 0,
		0, -1, 0, 0,
		0, 0, 0, 1
	};


	// 创建切片矩阵
	auto resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
	resliceAxes->SetElement(0, 3, center[0]);
	resliceAxes->SetElement(1, 3, center[1]);
	resliceAxes->SetElement(2, 3, center[2]);

	// 定义切片的方向向量
	double x1[3] = { 1, 0, 0 };
	double y1[3] = { 0, 1, 0 };
	double z1[3] = { 0, 0, 1 };

	// 创建 vtkImageReslice 对象
	vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
	reslice->SetInputConnection(d.reader->GetOutputPort());
	reslice->SetInterpolationModeToLinear();


	// 设置切片的方向和位置

	reslice->SetResliceAxesDirectionCosines(x1, y1, z1);
	reslice->SetResliceAxesOrigin(center);
	reslice->SetOutputDimensionality(2);

	// 设置插值模式
	reslice->SetInterpolationModeToLinear();

	// 根据方向设置切片矩阵
	switch (direction) {
	case 1: // X 方向
	{
		resliceAxes->DeepCopy(sagittalElements);
		reslice->SetResliceAxes(resliceAxes);

		double spacing = reslice->GetOutput()->GetSpacing()[0];
		vtkMatrix4x4* matrix1 = reslice->GetResliceAxes();
		double point[4], center[4];

		point[0] = 0.0;
		point[1] = 0.0;
		point[2] = spacing * x;
		point[3] = 1.0;

		matrix1->MultiplyPoint(point, center);
		matrix1->SetElement(0, 3, center[0]);
		matrix1->SetElement(1, 3, center[1]);
		matrix1->SetElement(2, 3, center[2]);
		break;
	}


	case 2: // Y 方向
	{
		resliceAxes->DeepCopy(coronalElements);
		reslice->SetResliceAxes(resliceAxes);

		double spacing = reslice->GetOutput()->GetSpacing()[1];
		vtkMatrix4x4* matrix1 = reslice->GetResliceAxes();
		double point[4], center[4];
		point[0] = 0.0;
		point[1] = 0.0;
		point[2] = spacing * y;
		point[3] = 1.0;

		matrix1->MultiplyPoint(point, center);
		matrix1->SetElement(0, 3, center[0]);
		matrix1->SetElement(1, 3, center[1]);
		matrix1->SetElement(2, 3, center[2]);
		break;
	}
	case 3: // Z 方向
	default:
	{
		resliceAxes->DeepCopy(axialElements);

		reslice->SetResliceAxes(resliceAxes);

		double spacing = reslice->GetOutput()->GetSpacing()[2];
		vtkMatrix4x4* matrix1 = reslice->GetResliceAxes();
		double point[4], center[4];
		point[0] = 0.0;
		point[1] = 0.0;
		point[2] = spacing * z;
		point[3] = 1.0;

		matrix1->MultiplyPoint(point, center);
		matrix1->SetElement(0, 3, center[0]);
		matrix1->SetElement(1, 3, center[1]);
		matrix1->SetElement(2, 3, center[2]);
	}

	break;
	}

	// 确保切片已经更新
	reslice->Update();
	vtkImageData* reslicedImage = reslice->GetOutput();
	int* reslicedExtent = reslicedImage->GetExtent();
	std::cout << "切片范围: "
		<< reslicedExtent[0] << " " << reslicedExtent[1] << " "
		<< reslicedExtent[2] << " " << reslicedExtent[3] << " "
		<< reslicedExtent[4] << " " << reslicedExtent[5] << std::endl;

	// 创建颜色映射
	auto lookupTable = vtkSmartPointer<vtkLookupTable>::New();
	lookupTable->SetRange(range[0], range[1]); // 使用图像的实际值范围
	lookupTable->SetValueRange(0.0, 1.0);
	lookupTable->SetSaturationRange(0.0, 0.0);
	lookupTable->SetRampToLinear();
	lookupTable->Build();

	vtkSmartPointer<vtkImageMapToColors> colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
	colorMap->SetInputConnection(reslice->GetOutputPort());
	colorMap->SetLookupTable(lookupTable);
	colorMap->Update();

	// 创建 actor
	vtkSmartPointer<vtkImageActor> imageActor = vtkSmartPointer<vtkImageActor>::New();
	imageActor->SetInputData(colorMap->GetOutput());

	// 将 actor 添加到渲染器
	d.render->AddActor(imageActor);
	d.render->SetBackground(1, 1, 1); // 设置背景颜色为白色
	// 刷新渲染窗口
	d.render->ResetCamera();

	return true;
}

bool CGRenderView::VTKRender3D::changeColor(int time)
{
	auto& d = *m_priv;

	d.render->RemoveAllViewProps();  // 清空当前视图的所有显示项

	// 获取图像数据
	vtkSmartPointer<vtkImageData> imageData = d.reader->GetOutput();

	// 获取图像数据的最大值和最小值
	double range[2];
	imageData->GetScalarRange(range);  // 获取数据的最小值和最大值
	std::cout << "Scalar Range: " << range[0] << " - " << range[1] << std::endl;


	// 使用GPU加速的体积映射器
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	volumeMapper->SetInputData(imageData);

	// 创建颜色传输函数，用于设置颜色
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->AddRGBPoint(range[0], 0.0, 0.0, 0.0);  // 黑色
	colorTransferFunction->AddRGBPoint(range[1], 1.0, 1.0, 0.0);  // 红色
	// 可以根据需要增加更多的颜色映射点

	// 创建透明度传输函数，用于设置透明度
	vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacityTransferFunction->AddPoint(range[0], 0.0);  // 透明
	opacityTransferFunction->AddPoint(range[1], 1.0);  // 不透明
	// 可以根据需要调整透明度映射点

	// 创建体积对象
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->GetProperty()->SetColor(colorTransferFunction);
	volume->GetProperty()->SetScalarOpacity(opacityTransferFunction);

	// 设置透明度单位距离（控制透明度分布的“密度”）
	volume->GetProperty()->SetScalarOpacityUnitDistance(5.0);

	d.render->AddVolume(volume);

	// 刷新渲染窗口
	d.render->ResetCamera();

	return false;
}

