#pragma once
#include <QApplication>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>
#include <window.h>

#if ALLEGIANCE_SERENITY
#include "serenity_impl.h"
using Implementation = SerenityImpl;
using Application = all::SerenityGuiApplication;
#else
#include "qt3d_impl.h"
using Implementation = Qt3DImpl;
using Application = QApplication;
#endif


class WindowDestructionWatcher : public QObject
{
	Q_OBJECT
public:
	explicit WindowDestructionWatcher(QObject* window)
		: m_window(window)
	{
		m_window->installEventFilter(this);
	}

	bool eventFilter(QObject* obj, QEvent* event) override {
		if (event->type() == QEvent::Type::Close) {
			OnClose();
		}
		return QObject::eventFilter(obj, event);
	}
signals:
	void OnClose();
private:
	QObject* m_window;
};


class App {
public:
	App(int& argc, char** argv) : app(argc, argv), impl(std::in_place), wnd(impl->GetWindow(), { 1920, 1080 }) {
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

		QObject::connect(&watcher, &WindowDestructionWatcher::OnClose,
			[this]() {
				impl.reset();
				app.quit();
			}
		);


		impl->CreateAspects(wnd.GetCameraControl());
		wnd.show();
	}

public:
	int Start() noexcept { return app.exec(); }

private:

	void MakeScene() {}

private:
	Application app;
	std::optional<Implementation> impl;
	all::Window wnd;
	WindowDestructionWatcher watcher{ &wnd };
};
