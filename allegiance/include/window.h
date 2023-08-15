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

		QWidget* widget = QWidget::createWindowContainer(&view);
		setCentralWidget(widget);
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
	void SetScene(Qt3DCore::QEntity* root)
	{
		view.setRootEntity(root);
	}
signals:
	void OnClose();
	void OnLoadModel();
	void OnLoadImage();
private:
	Qt3DExtras::Qt3DWindow view;
};