#ifndef _CGWINDOWSWINDOW_H_
#define _CGWINDOWSWINDOW_H_
#include "CGRenderView_Export.h"
#include "Windows.h"

namespace CGRenderView
{
	class CGRENDERVIEW_API CGWindowsWindow
	{
	public:
		CGWindowsWindow(HWND hwnd, unsigned int width, unsigned int height);
		~CGWindowsWindow();

		void* getRenderWindow();
		void* getQVTKOpenGLNativeWidget();

		void start();
	public:
		void pbLoad();
		void pbSlice(int x, int y, int z, int direction);

		void pbChangeColor();
	public:
		void pbLoadobj();


	private:
		struct PrivateData;
		PrivateData* m_priv;

		CGWindowsWindow(const CGWindowsWindow&) = delete;
		CGWindowsWindow& operator=(const CGWindowsWindow&) = delete;
	};
}
#endif // _CGWINDOWSWINDOW_H_
