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
	*  ���Ǵ���triangle strips��
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

	// ���֮ǰ�� actor
	d.render->RemoveAllViewProps();

	// �����������
	vtkImageData* imageData = d.reader->GetOutput();
	if (!imageData) {
		std::cerr << "����û�м������ݣ�" << std::endl;
		return false;
	}

	// ��ӡͼ��Χ��ֵ��Χ
	int* extent = imageData->GetExtent();
	double* range = imageData->GetScalarRange();
	std::cout << "ͼ��Χ: "
		<< extent[0] << " " << extent[1] << " "
		<< extent[2] << " " << extent[3] << " "
		<< extent[4] << " " << extent[5] << std::endl;
	std::cout << "ͼ��ֵ��Χ: " << range[0] << " " << range[1] << std::endl;


	// ��ȡͼ��Ļ�����Ϣ
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

	//���岻ͬ�������Ƭ����
   /*
   * 1. Sagittal (ʸ״��) ��Ƭ����
   * ��������Ƭ����ͨ���� X �� ���������Ƭ��
   */
	static double sagittalElements[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	/*
	* 2. Coronal (��״��) ��Ƭ����
	* ��ǰ����Ƭ����ͨ���� Y �� ���������Ƭ��
	*/
	static double coronalElements[16] = {
		1, 0, 0, 0,
		0, 0, 1, 0,
		0, -1, 0, 0,
		0, 0, 0, 1
	};

	/*
	* 3. Axial (�����) ��Ƭ����
	* ��������Ƭ����ͨ���� Z �� ���������Ƭ��
	*/
	static double axialElements[16] = {
		0, 0, 1, 0,
		1, 0, 0, 0,
		0, -1, 0, 0,
		0, 0, 0, 1
	};


	// ������Ƭ����
	auto resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
	resliceAxes->SetElement(0, 3, center[0]);
	resliceAxes->SetElement(1, 3, center[1]);
	resliceAxes->SetElement(2, 3, center[2]);

	// ������Ƭ�ķ�������
	double x1[3] = { 1, 0, 0 };
	double y1[3] = { 0, 1, 0 };
	double z1[3] = { 0, 0, 1 };

	// ���� vtkImageReslice ����
	vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
	reslice->SetInputConnection(d.reader->GetOutputPort());
	reslice->SetInterpolationModeToLinear();


	// ������Ƭ�ķ����λ��

	reslice->SetResliceAxesDirectionCosines(x1, y1, z1);
	reslice->SetResliceAxesOrigin(center);
	reslice->SetOutputDimensionality(2);

	// ���ò�ֵģʽ
	reslice->SetInterpolationModeToLinear();

	// ���ݷ���������Ƭ����
	switch (direction) {
	case 1: // X ����
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


	case 2: // Y ����
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
	case 3: // Z ����
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

	// ȷ����Ƭ�Ѿ�����
	reslice->Update();
	vtkImageData* reslicedImage = reslice->GetOutput();
	int* reslicedExtent = reslicedImage->GetExtent();
	std::cout << "��Ƭ��Χ: "
		<< reslicedExtent[0] << " " << reslicedExtent[1] << " "
		<< reslicedExtent[2] << " " << reslicedExtent[3] << " "
		<< reslicedExtent[4] << " " << reslicedExtent[5] << std::endl;

	// ������ɫӳ��
	auto lookupTable = vtkSmartPointer<vtkLookupTable>::New();
	lookupTable->SetRange(range[0], range[1]); // ʹ��ͼ���ʵ��ֵ��Χ
	lookupTable->SetValueRange(0.0, 1.0);
	lookupTable->SetSaturationRange(0.0, 0.0);
	lookupTable->SetRampToLinear();
	lookupTable->Build();

	vtkSmartPointer<vtkImageMapToColors> colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
	colorMap->SetInputConnection(reslice->GetOutputPort());
	colorMap->SetLookupTable(lookupTable);
	colorMap->Update();

	// ���� actor
	vtkSmartPointer<vtkImageActor> imageActor = vtkSmartPointer<vtkImageActor>::New();
	imageActor->SetInputData(colorMap->GetOutput());

	// �� actor ��ӵ���Ⱦ��
	d.render->AddActor(imageActor);
	d.render->SetBackground(1, 1, 1); // ���ñ�����ɫΪ��ɫ
	// ˢ����Ⱦ����
	d.render->ResetCamera();

	return true;
}

bool CGRenderView::VTKRender3D::changeColor(int time)
{
	auto& d = *m_priv;

	d.render->RemoveAllViewProps();  // ��յ�ǰ��ͼ��������ʾ��

	// ��ȡͼ������
	vtkSmartPointer<vtkImageData> imageData = d.reader->GetOutput();

	// ��ȡͼ�����ݵ����ֵ����Сֵ
	double range[2];
	imageData->GetScalarRange(range);  // ��ȡ���ݵ���Сֵ�����ֵ
	std::cout << "Scalar Range: " << range[0] << " - " << range[1] << std::endl;


	// ʹ��GPU���ٵ����ӳ����
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	volumeMapper->SetInputData(imageData);

	// ������ɫ���亯��������������ɫ
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->AddRGBPoint(range[0], 0.0, 0.0, 0.0);  // ��ɫ
	colorTransferFunction->AddRGBPoint(range[1], 1.0, 1.0, 0.0);  // ��ɫ
	// ���Ը�����Ҫ���Ӹ������ɫӳ���

	// ����͸���ȴ��亯������������͸����
	vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacityTransferFunction->AddPoint(range[0], 0.0);  // ͸��
	opacityTransferFunction->AddPoint(range[1], 1.0);  // ��͸��
	// ���Ը�����Ҫ����͸����ӳ���

	// �����������
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->GetProperty()->SetColor(colorTransferFunction);
	volume->GetProperty()->SetScalarOpacity(opacityTransferFunction);

	// ����͸���ȵ�λ���루����͸���ȷֲ��ġ��ܶȡ���
	volume->GetProperty()->SetScalarOpacityUnitDistance(5.0);

	d.render->AddVolume(volume);

	// ˢ����Ⱦ����
	d.render->ResetCamera();

	return false;
}

