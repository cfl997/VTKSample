#include "VTKReader3D.h"
#include "VTKInclude.h"


struct CGRenderView::VTKReader3D::PrivateData
{
	vtkSmartPointer<vtkImageReader> reader;
	vtkSmartPointer<vtkRenderer> render;
	float Spacing = 1.0f;
};

CGRenderView::VTKReader3D::VTKReader3D(vtkSmartPointer<vtkRenderer> render) :m_priv(new PrivateData)
{
	auto& d = *m_priv;
	d.render = render;
}

CGRenderView::VTKReader3D::~VTKReader3D()
{
	delete m_priv;
	m_priv = nullptr;
}

bool CGRenderView::VTKReader3D::loadFile(const char* filename, int x, int y, int z, float Spacing)
{
	auto& d = *m_priv;
	d.Spacing = Spacing;
	d.render->RemoveAllViewProps();
	if (d.reader)
	{
		d.reader = nullptr;
	}
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

bool CGRenderView::VTKReader3D::slice(int x, int y, int z, int direction)
{
	auto& d = *m_priv;

	d.render->RemoveAllViewProps();

	vtkImageData* imageData = d.reader->GetOutput();
	if (!imageData) {
		std::cerr << "错误：没有加载数据！" << std::endl;
		return false;
	}

	int* extent = imageData->GetExtent();
	double* range = imageData->GetScalarRange();
	std::cout << "图像范围: "
		<< extent[0] << " " << extent[1] << " "
		<< extent[2] << " " << extent[3] << " "
		<< extent[4] << " " << extent[5] << std::endl;
	std::cout << "图像值范围: " << range[0] << " " << range[1] << std::endl;

	// 获取图像的基本信息
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

	auto fn = [&](int Spaceindex, int xyzValue) {
		reslice->SetResliceAxes(resliceAxes);

		double spacing = reslice->GetOutput()->GetSpacing()[Spaceindex];
		vtkMatrix4x4* matrix1 = reslice->GetResliceAxes();
		double point[4], center[4];

		point[0] = 0.0;
		point[1] = 0.0;
		point[2] = spacing * xyzValue;
		point[3] = 1.0;

		matrix1->MultiplyPoint(point, center);
		matrix1->SetElement(0, 3, center[0]);
		matrix1->SetElement(1, 3, center[1]);
		matrix1->SetElement(2, 3, center[2]);
		};
	// 根据方向设置切片矩阵
	switch (direction) {
	case 1: // X 方向
	{
		resliceAxes->DeepCopy(sagittalElements);
		fn(0, x);
		break;
	}
	case 2: // Y 方向
	{
		resliceAxes->DeepCopy(coronalElements);
		fn(1, y);
		break;
	}
	case 3: // Z 方向
	default:
	{
		resliceAxes->DeepCopy(axialElements);
		fn(2, z);
	}
	break;
	}

	// 确保切片已经更新
	reslice->Update();

	vtkImageData* reslicedImage = reslice->GetOutput();
	int* reslicedExtent = reslicedImage->GetExtent();
	std::cout << "切片范围 "
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

	vtkSmartPointer<vtkImageActor> imageActor = vtkSmartPointer<vtkImageActor>::New();
	imageActor->SetInputData(colorMap->GetOutput());

	d.render->AddActor(imageActor);
	d.render->SetBackground(1, 1, 1); 

	d.render->GradientBackgroundOff();
	d.render->UseHiddenLineRemovalOff();
	d.render->UseShadowsOff();

	d.render->ResetCamera();

	return true;
}

bool CGRenderView::VTKReader3D::changeColor(int time)
{
	auto& d = *m_priv;

	d.render->RemoveAllViewProps();

	vtkSmartPointer<vtkImageData> imageData = d.reader->GetOutput();

	double range[2];
	imageData->GetScalarRange(range);
	std::cout << "Scalar Range: " << range[0] << " - " << range[1] << std::endl;


	// 使用GPU加速的体积映射器
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	volumeMapper->SetInputData(imageData);

	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->AddRGBPoint(range[0], 0.0, 0.0, 0.0);  // 黑色
	colorTransferFunction->AddRGBPoint(range[1], 1.0, 1.0, 0.0);  

	vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacityTransferFunction->AddPoint(range[0], 0.0);  // 透明
	opacityTransferFunction->AddPoint(range[1], 1.0);  // 不透明

	// 创建体积对象
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->GetProperty()->SetColor(colorTransferFunction);
	volume->GetProperty()->SetScalarOpacity(opacityTransferFunction);

	// 设置透明度单位距离（控制透明度分布的“密度”）
	volume->GetProperty()->SetScalarOpacityUnitDistance(5.0);

	d.render->SetBackground(0, 0, 0);

	d.render->AddVolume(volume);

	d.render->ResetCamera();

	return true;
}


bool CGRenderView::VTKReader3D::loadObj(const char* obj, const char* mtl, const char* filepath)
{
	auto& d = *m_priv;

	if (!d.render || !d.render->GetRenderWindow()) {
		std::cerr << "Render window is not initialized." << std::endl;
		return false;
	}

	d.render->RemoveAllViewProps();

	vtkNew<vtkOBJImporter> importer;
	importer->SetFileName((std::string(filepath) + obj).c_str());
	importer->SetFileNameMTL((std::string(filepath) + mtl).c_str());
	importer->SetTexturePath(filepath);
	importer->SetRenderWindow(d.render->GetRenderWindow());
	importer->Update();

	// 验证是否成功加载
	vtkRenderer* impRenderer = importer->GetRenderer();
	if (!impRenderer || impRenderer->GetActors()->GetNumberOfItems() == 0) {
		std::cerr << "Error: Failed to load OBJ file or no actors found: " << obj << std::endl;
		return false;
	}

	std::cout << "OBJ file loaded successfully.\n";

	// 设置背景
	vtkNew<vtkNamedColors> colors;
	d.render->SetBackground2(colors->GetColor3d("Silver").GetData());
	d.render->SetBackground(colors->GetColor3d("Gold").GetData());
	d.render->GradientBackgroundOn();
	d.render->UseHiddenLineRemovalOn();
	d.render->UseShadowsOn();
	d.render->SetAmbient(0.3, 0.3, 0.3);

	// 遍历 actors
	vtkActorCollection* actors = impRenderer->GetActors();
	actors->InitTraversal();

	vtkActor* actor = nullptr;
	while ((actor = actors->GetNextActor()) != nullptr) {
		std::cout << "Loaded actor.\n";

		// 处理纹理
		if (actor->GetTexture()) {
			std::cout << "Actor has texture\n";
			actor->GetTexture()->InterpolateOn();
			actor->GetTexture()->RepeatOn();
		}
	}

	return true;
}

