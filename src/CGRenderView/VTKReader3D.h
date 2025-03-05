#ifndef _VTKRENDER3D_H_
#define _VTKRENDER3D_H_

template <class T>
class vtkSmartPointer;
class vtkRenderer;
namespace CGRenderView
{
	class VTKRender3D
	{
	public:
		VTKRender3D(vtkSmartPointer<vtkRenderer>render);
		~VTKRender3D();
	public:
		bool loadFile(const char* filename, int x, int y, int z, float Spacing = 1.);

		bool slice(int x, int y, int z, int direction);

		bool changeColor(int time);
	private:
		struct PrivateData;
		PrivateData* m_priv;
	};
}

#endif // !_VTKRENDER3D_H_
