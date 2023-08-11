#pragma once
#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include <QPalette>
#include <window.h>

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

		wnd.show();
	}
public:
	int Start()noexcept { return app.exec(); }

private:
	QApplication app;
	Window wnd;
};

