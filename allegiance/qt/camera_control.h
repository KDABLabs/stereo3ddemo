#pragma once
#include <QMouseEvent>
#include <QQuickWidget>
#include <QQuickStyle>
#include <QQmlEngine>
#include <QVBoxLayout>

namespace all::qt {

class CameraControl : public QWidget
{
    Q_OBJECT
public:
    CameraControl(QWidget* parent)
        : QWidget(parent)
    {
        QQuickStyle::setStyle("Fusion");

        layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);
        layout->setContentsMargins(20, 20, 20, 20);

        engine = new QQmlEngine(this);
        qw = new QQuickWidget(engine, this);
        qw->setSource(QUrl(u"qrc:/resources/camera_control.qml"_qs));
        qw->setResizeMode(QQuickWidget::SizeRootObjectToView);
        qw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        layout->addWidget(qw);
    }
    void Reload()
    {
        // engine->clearComponentCache();
        // qw->setSource(QUrl::fromLocalFile(u"C:/Users/Agrae/source/repos/qt3d/allegiance/resources/camera_control.qml"_qs));
        // return;
    }
Q_SIGNALS:
    void OnEyeDisparityChanged(float value);
    void OnFocusPlaneChanged(float value);

    void OnLoadImage();
    void OnLoadModel();
    void OnToggleCursor(bool checked);
    void OnClose();

private:
    QQuickWidget* qw;
    QVBoxLayout* layout;
    QQmlEngine* engine;
};
} // namespace all::qt
