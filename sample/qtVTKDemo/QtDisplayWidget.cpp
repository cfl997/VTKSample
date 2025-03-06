#include "QtDisplayWidget.h"
#include "ui_QtDisplayWidget.h"


#include <memory>
#include <thread>
#include <qtimer.h>
#include <qfiledialog.h>
#include <iostream>



#include "CGRenderView.h"
#include <QVTKOpenGLNativeWidget.h>


struct QTDisplayWidget::PrivateData
{
	Ui_MainWidget ui;
	CGRenderView::CGWindowsWindow* windowsWindow = nullptr;
	QVTKOpenGLNativeWidget* vtkWidget = nullptr;
	std::thread startThread;
	void pbSlice();
};

QTDisplayWidget::QTDisplayWidget(QWidget* parent)
	: QWidget(parent), m_priv(new PrivateData)
{
	auto& d = *m_priv;
	d.ui.setupUi(this);

	// 获取 Qt 窗口的句柄
	HWND parentHwnd = reinterpret_cast<HWND>(d.ui.w_glwindow->winId());  // 获取窗口句柄

	// 获取 Qt 窗口尺寸
	int width = d.ui.w_glwindow->geometry().width();
	int height = d.ui.w_glwindow->geometry().height();

	// 检查窗口句柄是否有效
	if (parentHwnd == nullptr) {
		std::cerr << "Error: Failed to get valid HWND for the Qt window." << std::endl;
		return;
	}
	// 创建 VTK 渲染窗口
	d.windowsWindow = new CGRenderView::CGWindowsWindow(parentHwnd, width, height);

	void* windowsid = d.windowsWindow->getQVTKOpenGLNativeWidget();
	d.vtkWidget = static_cast<QVTKOpenGLNativeWidget*>(windowsid);
	QVBoxLayout* layout = new QVBoxLayout(d.ui.w_glwindow);  // 假设没有现有布局
	layout->addWidget(d.vtkWidget);  // 将 vtkWidget 添加到布局中
	d.ui.w_glwindow->setLayout(layout);  // 更新布局


	//使用 Qt 的计时器来驱动 VTK 的事件循环
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [&]() {
		//d.windowsWindow->start();
		});
	timer->start(10);

	connect(d.ui.pbload, &QPushButton::clicked, this, [&]() {
		d.windowsWindow->pbLoad();
		}, Qt::QueuedConnection);

	d.ui.cbSliceDirection->addItem("X", 1);  // "X" 对应值 1
	d.ui.cbSliceDirection->addItem("Y", 2);  // "Y" 对应值 2
	d.ui.cbSliceDirection->addItem("Z", 3);  // "Z" 对应值 3



	connect(d.ui.pbSlice, &QPushButton::clicked, this, [&]() {
		d.pbSlice();
		});
	connect(d.ui.hzSlider, &QSlider::valueChanged, this, [&]() {
		auto direction = d.ui.cbSliceDirection->currentData().toInt();
		auto fn = [&](int direction) {
			if (direction == 1) {
				d.ui.leSliceX->setText(QString::number(d.ui.hzSlider->value()));
			}
			else if (direction == 2) {
				d.ui.leSliceY->setText(QString::number(d.ui.hzSlider->value()));
			}
			else if (direction == 3) {
				d.ui.leSliceZ->setText(QString::number(d.ui.hzSlider->value()));
			}
		};
		fn(direction);
		d.pbSlice();
		});

	connect(d.ui.pbChangeColor, &QPushButton::clicked, this, [&]() {
		d.windowsWindow->pbChangeColor();
		});
	connect(d.ui.pbLoadObj, &QPushButton::clicked, this, [&]() {
		d.windowsWindow->pbLoadobj();
		});
	d.windowsWindow->start();

}

QTDisplayWidget::~QTDisplayWidget()
{
	if (m_priv)
	{
		delete m_priv;
		m_priv = nullptr;
	}
}

void QTDisplayWidget::runLoop()
{
	auto& d = *m_priv;
	d.windowsWindow->start();
}

void QTDisplayWidget::PrivateData::pbSlice()
{
	windowsWindow->pbSlice(ui.leSliceX->text().toInt(), ui.leSliceY->text().toInt(), ui.leSliceZ->text().toInt(), ui.cbSliceDirection->currentData().toInt());
}
