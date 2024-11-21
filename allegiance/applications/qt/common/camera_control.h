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

        m_layout = new QVBoxLayout(this);
        m_layout->setAlignment(Qt::AlignTop);
        m_layout->setContentsMargins(20, 20, 20, 20);

        m_engine = new QQmlEngine(this);
        m_quickWidget = new QQuickWidget(m_engine, this);
        m_quickWidget->setSource(QUrl(u"qrc:/qt/qml/common/qml/camera_control.qml"_qs));
        m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
        m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_layout->addWidget(m_quickWidget);
    }
    void reload()
    {
        // engine->clearComponentCache();
        // qw->setSource(QUrl::fromLocalFile(u"C:/Users/Agrae/source/repos/qt3d/allegiance/resources/camera_control.qml"_qs));
        // return;
    }

    QQmlEngine* qmlEngine() const
    {
        return m_engine;
    }

Q_SIGNALS:
    void eyeDisparityChanged(float value);
    void focusPlaneChanged(float value);

    void onLoadImage();
    void onLoadModel();
    void onToggleCursor(bool checked);
    void onClose();

private:
    QVBoxLayout* m_layout;
    QQuickWidget* m_quickWidget;
    QQmlEngine* m_engine;
};
} // namespace all::qt
