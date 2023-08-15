#pragma once
#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include <QPalette>
#include <window.h>

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DInput;
using namespace Qt3DLogic;
using namespace Qt3DExtras;

class App
{
public:
	App(int& argc, char** argv)
		:app(argc, argv), wnd({ 1920,1080 })
	{
		// Basic setup of the application
		QCoreApplication::setApplicationName(QStringLiteral("Schneider Demo"));
		QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
		app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
		app.setWindowIcon(QIcon{ QStringLiteral(":/tlr.ico") });

		// Style definition (darkmode)
		QPalette darkPalette;
		darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::WindowText, Qt::white);
		darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
		darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
		darkPalette.setColor(QPalette::ToolTipText, Qt::white);
		darkPalette.setColor(QPalette::Text, Qt::white);
		darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::ButtonText, Qt::white);
		darkPalette.setColor(QPalette::BrightText, Qt::red);
		darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::HighlightedText, Qt::black);
		app.setPalette(darkPalette);
		MakeScene();

		wnd.SetScene(root.get());
		wnd.show();
	}
public:
	int Start()noexcept { return app.exec(); }

private:
	void MakeScene()
	{
		root = std::make_unique<Qt3DCore::QEntity>();

		QViewport* viewport = new QViewport(root.get());
		viewport->setNormalizedRect(QRectF(0, 0, 1, 1));

		QRenderTargetOutput *output_left = new QRenderTargetOutput(viewport);
		output_left->setAttachmentPoint(QRenderTargetOutput::Left);

		QRenderTarget* renderTarget = new QRenderTarget(viewport);
		renderTarget->addOutput(output_left);

		QRenderTargetSelector* selector = new QRenderTargetSelector(viewport);
		selector->setTarget(renderTarget);

		QClearBuffers* clearBuffers = new QClearBuffers(selector);
		clearBuffers->setClearColor(Qt::red);

		QNoDraw* noDraw1 = new QNoDraw(clearBuffers);

		QRenderTargetOutput* output_right = new QRenderTargetOutput(viewport);
		output_right->setAttachmentPoint(QRenderTargetOutput::Right);

		QRenderTarget* renderTargetR = new QRenderTarget(viewport);
		renderTargetR->addOutput(output_right);

		QRenderTargetSelector* selector2 = new QRenderTargetSelector(viewport);
		selector2->setTarget(renderTargetR);

		QClearBuffers* clearBuffers2 = new QClearBuffers(selector2);
		clearBuffers2->setClearColor(Qt::blue);

		QNoDraw* noDraw2 = new QNoDraw(clearBuffers2);
	}

private:
	QApplication app;
	Window wnd;
	std::unique_ptr<Qt3DCore::QEntity> root;
};

