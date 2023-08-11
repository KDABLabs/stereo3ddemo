#pragma once
#include <QMainWindow>
#include <QMenuBar>
#include <QDockWidget>


class Window : public QMainWindow
{
	Q_OBJECT
public:
	Window(QSize size)noexcept
	{
		resize(size);
		auto central_widget = new QWidget(this);


		setCentralWidget(central_widget);
		CreateDockWidget();

		QMenu* fileMenu = menuBar()->addMenu("File");

		fileMenu->addAction("Load Model", [this]() {OnLoadModel(); });
		fileMenu->addAction("Load Image", [this]() {OnLoadImage(); });
		fileMenu->addAction("Exit", [this]() {close(); });
	}
public:
	void CreateDockWidget()
	{
		QDockWidget* dock = new QDockWidget("Camera", this);
		auto camera_control = new QWidget(dock);
		dock->setWidget(camera_control);
		addDockWidget(Qt::RightDockWidgetArea, dock);
		dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	}
signals:
	void OnClose();
	void OnLoadModel();
	void OnLoadImage();
};