#pragma once
#include <QMouseEvent>
#include <QQuickWidget>
#include <QQuickStyle>
#include <QQmlEngine>
#include <QVBoxLayout>

#include <applications/qt/common/qml/Schneider/controllers.h>
#include <applications/qt/common/qml/Schneider/style.h>

namespace all::qt {

class SideMenu : public QWidget
{
    Q_OBJECT
public:
    SideMenu(QWidget* parent)
        : QWidget(parent)
    {
        qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");
        QQuickStyle::setStyle("Material");

        m_layout = new QVBoxLayout(this);
        m_layout->setAlignment(Qt::AlignTop);
        m_layout->setContentsMargins(5, 5, 5, 5);

        m_engine = new QQmlEngine(this);
        m_quickWidget = new QQuickWidget(m_engine, this);
        m_quickWidget->setSource(QUrl(QStringLiteral("qrc:/qt/qml/common/qml/side_menu.qml")));
        m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
        m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // Retrieve QML Singleton instances
        m_sceneController = m_engine->singletonInstance<SceneController*>("Schneider", "Scene");
        m_cameraController = m_engine->singletonInstance<CameraController*>("Schneider", "Camera");
        m_cursorController = m_engine->singletonInstance<CursorController*>("Schneider", "Cursor");
        m_appStyle = m_engine->singletonInstance<AppStyle*>("Schneider", "Style");

        assert(m_sceneController);
        assert(m_cameraController);
        assert(m_cursorController);
        assert(m_appStyle);

        setClearColor();

        m_layout->addWidget(m_quickWidget);
    }

    void setClearColor()
    {
        m_quickWidget->setClearColor(m_appStyle->backgroundColor());
    }

    inline SceneController* sceneController() const { return m_sceneController; }
    inline CameraController* cameraController() const { return m_cameraController; }
    inline CursorController* cursorController() const { return m_cursorController; }
    inline AppStyle* appStyle() const { return m_appStyle; }

    inline QQmlEngine* qmlEngine() const { return m_engine; }

    bool onKeyPress(QKeyEvent* e)
    {
        switch (e->key()) {
        case Qt::Key_F2: {
            if (e->modifiers() & Qt::ShiftModifier)
                m_cameraController->setShowAutoFocusArea(!m_cameraController->showAutoFocusArea());
            else
                m_cameraController->setAutoFocus(!m_cameraController->autoFocus());
            return true;
        }
        case Qt::Key_F3:
        case Qt::Key_F4: {
            if (!m_cameraController->autoFocus()) {
                const bool fineIncr = (e->modifiers() & Qt::ShiftModifier);
                const float incr = fineIncr ? 1.0f : 10.0f;
                const float sign = e->key() == Qt::Key_F3 ? -1.0f : 1.0f;
                m_cameraController->setFocusDistance(m_cameraController->focusDistance() + sign * incr);
            }
            return true;
        }
        case Qt::Key_F5:
        case Qt::Key_F6: {
            const bool fineIncr = (e->modifiers() & Qt::ShiftModifier);
            const float incr = fineIncr ? 1.0f : 10.0f;
            const float sign = e->key() == Qt::Key_F5 ? -1.0f : 1.0f;
            m_cameraController->setPopOut(m_cameraController->popOut() + sign * incr);
            return true;
        }
        case Qt::Key_F7:
        case Qt::Key_F8: {
            if (!m_cameraController->separationBasedOnFocusDistance()) {
                const bool fineIncr = (e->modifiers() & Qt::ShiftModifier);
                const float sign = e->key() == Qt::Key_F7 ? -1.0f : 1.0f;
                const float incr = fineIncr ? 0.005f : 0.01f;
                m_cameraController->setEyeDistance(m_cameraController->eyeDistance() + sign * incr);
            }
            return true;
        }
        case Qt::Key_Shift: {
            m_sceneController->setShiftPressed(true);
            return true;
        }

        default:
            break;
        }
        return false;
    }

    bool onKeyRelease(QKeyEvent* e)
    {
        switch (e->key()) {
        case Qt::Key_Shift: {
            m_sceneController->setShiftPressed(false);
            return true;
        }
        default:
            break;
        }
        return false;
    }

    bool isShiftPressed() const
    {
        return m_sceneController->shiftPressed();
    }

Q_SIGNALS:
    void onLoadImage();
    void onLoadModel();
    void onClose();

private:
    QVBoxLayout* m_layout{ nullptr };
    QQuickWidget* m_quickWidget{ nullptr };
    QQmlEngine* m_engine{ nullptr };

    SceneController* m_sceneController{ nullptr };
    CameraController* m_cameraController{ nullptr };
    CursorController* m_cursorController{ nullptr };
    AppStyle* m_appStyle{ nullptr };
};
} // namespace all::qt
