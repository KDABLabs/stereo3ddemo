#pragma once
#include <QMouseEvent>
#include <QQuickWidget>
#include <QQuickStyle>
#include <QQmlEngine>
#include <QVBoxLayout>

#include <applications/qt/common/qml/Schneider/style.h>

namespace all::qt {

class CameraControl : public QWidget
{
    Q_OBJECT
public:
    CameraControl(QWidget* parent)
        : QWidget(parent)
    {
        qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");
        QQuickStyle::setStyle("Material");

        m_layout = new QVBoxLayout(this);
        m_layout->setAlignment(Qt::AlignTop);
        m_layout->setContentsMargins(5, 5, 5, 5);

        m_engine = new QQmlEngine(this);
        m_quickWidget = new QQuickWidget(m_engine, this);
        m_quickWidget->setSource(QUrl(u"qrc:/qt/qml/common/qml/camera_control.qml"_qs));
        m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
        m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        AppStyle* style = m_quickWidget->engine()->singletonInstance<AppStyle*>("Schneider", "Style");
        assert(style);
        m_quickWidget->setClearColor(style->backgroundColor());

        m_layout->addWidget(m_quickWidget);
    }

    QQmlEngine* qmlEngine() const
    {
        return m_engine;
    }

Q_SIGNALS:
    void onLoadImage();
    void onLoadModel();
    void onClose();

private:
    QVBoxLayout* m_layout;
    QQuickWidget* m_quickWidget;
    QQmlEngine* m_engine;
};
} // namespace all::qt
