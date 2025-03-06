#ifndef _VTKRENDER3D_H_
#define _VTKRENDER3D_H_

template <class T>
class vtkSmartPointer;
class vtkRenderer;
namespace CGRenderView
{
	class VTKReader3D
	{
	public:
		VTKReader3D(vtkSmartPointer<vtkRenderer>render);
		~VTKReader3D();
	public:
		bool loadFile(const char* filename, int x, int y, int z, float Spacing = 1.);

		bool slice(int x, int y, int z, int direction);

		bool changeColor(int time);

	public:
		bool loadObj(const char* obj, const char* mtl, const char* filepath);
	private:
		struct PrivateData;
		PrivateData* m_priv;
	};
}

#endif // !_VTKRENDER3D_H_
