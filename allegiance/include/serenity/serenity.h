#pragma once
#pragma push_macro("slots")
#pragma push_macro("signals")
#pragma push_macro("emit")
#pragma push_macro("foreach")
#undef slots
#undef signals
#undef emit
#undef foreach

#include <Serenity/gui/triangle_bounding_volume.h>
#include <Serenity/gui/window_extent_watcher.h>
#include <KDGui/gui_application.h>
#include <stdexcept>
#include <Serenity/gui/forward_renderer/stereo_forward_algorithm.h>
#include <Serenity/core/ecs/aspect_engine.h>
#include <KDGpu/vulkan/vulkan_graphics_api.h>
#include <Serenity/gui/render/render_aspect.h>
#include <Serenity/gui/shader_program.h>
#include <Serenity/gui/light.h>
#include <Serenity/gui/mesh_generators.h>
#include <Serenity/gui/mesh_renderer.h>
#include <Serenity/gui/mesh_loader.h>
#include <Serenity/gui/texture.h>
#include <Serenity/core/ecs/transform.h>
#include <Serenity/core/ecs/camera.h>
#include <Serenity/logic/frame_action.h>
#include <Serenity/logic/logic_aspect.h>

#pragma pop_macro("slots")
#pragma pop_macro("signals")
#pragma pop_macro("emit")
#pragma pop_macro("foreach")

#include <QApplication>
#include <QEvent>
#include <QWindow>
#include <QScreen>



namespace all
{
    class SerenityUpdateEvent : public QEvent
    {
    public:
        SerenityUpdateEvent()
            : QEvent(static_cast<QEvent::Type>(SerenityUpdateEvent::m_type))
        {
        }

        static int eventType() { return m_type; }

    private:
        static inline const int m_type = QEvent::registerEventType();
    };

    class SerenityGuiApplication : public QApplication
    {
    public:
        explicit SerenityGuiApplication(int& ac, char** av)
            : QApplication(ac, av)
        {
            loop();
        }

    protected:
        bool event(QEvent* e) override
        {
            if (e->type() == SerenityUpdateEvent::eventType()) {
                m_serenityApp.processEvents();
                loop();
            }

            // We want to have the qApp call app->processEvent periodically
            return QApplication::event(e);
        }

        void loop()
        {
            postEvent(this, new SerenityUpdateEvent());
        }

    private:
        KDGui::GuiApplication m_serenityApp;
    };

    class QWindowExtentWatcher : public Serenity::WindowExtentWatcher
    {
    public:
        explicit QWindowExtentWatcher(QWindow* w)
            : m_window(w), m_screen(w->screen())
        {
            qApp->setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
        }

        uint32_t width() const override
        {
            return std::ceil(m_window->width() * m_window->screen()->devicePixelRatio());
        }

        uint32_t height() const override
        {
            return std::ceil(m_window->height() * m_window->screen()->devicePixelRatio());
        }

    private:
        QWindow* m_window = nullptr;
        QScreen* m_screen = nullptr;
        qreal m_dpi_scale = 1.0;
    };
}

