#include "QtDisplayWidget.h"
#include "ui_QtDisplayWidget.h"


#include <memory>
#include <thread>
#include <qtimer.h>
#include <qfiledialog.h>
#include <iostream>

#include <Windows.h>



#include "CGRenderView.h"
#include <QVTKOpenGLNativeWidget.h>


struct QTDisplayWidget::PrivateData
{
	Ui_MainWidget ui;
	CGRenderView::CGWindowsWindow* windowsWindow = nullptr;
	QVTKOpenGLNativeWidget* vtkWidget = nullptr;
	std::thread startThread;
};

QTDisplayWidget::QTDisplayWidget(QWidget* parent)
	: QWidget(parent), m_priv(new PrivateData)
{
	auto& d = *m_priv;
	d.ui.setupUi(this);


	// ��ȡ Qt ���ڵľ��
	HWND parentHwnd = reinterpret_cast<HWND>(d.ui.w_glwindow->winId());  // ��ȡ���ھ��

	// ��ȡ Qt ���ڳߴ�
	int width = d.ui.w_glwindow->geometry().width();
	int height = d.ui.w_glwindow->geometry().height();

	// ��鴰�ھ���Ƿ���Ч
	if (parentHwnd == nullptr) {
		std::cerr << "Error: Failed to get valid HWND for the Qt window." << std::endl;
		return;
	}
	// ���� VTK ��Ⱦ����
	d.windowsWindow = new CGRenderView::CGWindowsWindow(parentHwnd, width, height);

	void* windowsid = d.windowsWindow->getQVTKOpenGLNativeWidget();
	d.vtkWidget = static_cast<QVTKOpenGLNativeWidget*>(windowsid);
	//d.ui.w_glwindow->setCentralWidget(d.vtkWidget);
	QVBoxLayout* layout = new QVBoxLayout(d.ui.w_glwindow);  // ����û�����в���
	layout->addWidget(d.vtkWidget);  // �� vtkWidget ��ӵ�������
	d.ui.w_glwindow->setLayout(layout);  // ���²���


	//ʹ�� Qt �ļ�ʱ�������� VTK ���¼�ѭ��
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [&]() {
		//d.windowsWindow->start();
		});
	timer->start(10);

	connect(d.ui.pbload, &QPushButton::clicked, this, [&]() {
		d.windowsWindow->pbone();
		});

	d.ui.cbSliceDirection->addItem("X", 1);  // "X" ��Ӧֵ 1
	d.ui.cbSliceDirection->addItem("Y", 2);  // "Y" ��Ӧֵ 2
	d.ui.cbSliceDirection->addItem("Z", 3);  // "Z" ��Ӧֵ 3

	connect(d.ui.pbSlice, &QPushButton::clicked, this, [&]() {
		d.windowsWindow->pbSlice(d.ui.leSliceX->text().toInt(), d.ui.leSliceY->text().toInt(), d.ui.leSliceZ->text().toInt(), d.ui.cbSliceDirection->currentData().toInt());
		});
	connect(d.ui.hzSlider, &QSlider::valueChanged, this, [&]() {
		d.ui.hzSlider->value();
		auto direction = d.ui.cbSliceDirection->currentData().toInt();
		if (direction == 1) {
			d.ui.leSliceX->setText(QString::number(d.ui.hzSlider->value()));
		}
		else if (direction == 2) {
			d.ui.leSliceY->setText(QString::number(d.ui.hzSlider->value()));
		}
		else if (direction == 3) {
			d.ui.leSliceZ->setText(QString::number(d.ui.hzSlider->value()));
		}
		d.windowsWindow->pbSlice(d.ui.leSliceX->text().toInt(), d.ui.leSliceY->text().toInt(), d.ui.leSliceZ->text().toInt(), d.ui.cbSliceDirection->currentData().toInt());

		});

	connect(d.ui.pbChangeColor, &QPushButton::clicked, this, [&]() {
		d.windowsWindow->pbChangeColor();
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
