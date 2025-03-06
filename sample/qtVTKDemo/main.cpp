#include <QApplication>

#include "QtDisplayWidget.h"


#include <Windows.h>

int main(int argc, char* argv[])
{
	SetConsoleOutputCP(CP_UTF8);  // 设置控制台输出为 UTF-8 编码

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)) && defined(_WIN32)
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
	QCoreApplication::addLibraryPath(".");
	QApplication* app = new QApplication(argc, argv);

	QTDisplayWidget w;
	w.show();
	return app->exec();
}