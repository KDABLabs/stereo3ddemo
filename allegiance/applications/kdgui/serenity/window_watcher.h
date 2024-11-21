#pragma once

#include <KDFoundation/object.h>
#include <KDGui/gui_events.h>

namespace all::kdgui {

class WindowEventWatcher : public KDFoundation::Object
{
public:
    KDBindings::Signal<const KDGui::MousePressEvent*> mousePressEvent;
    KDBindings::Signal<const KDGui::MouseReleaseEvent*> mouseReleaseEvent;
    KDBindings::Signal<const KDGui::MouseMoveEvent*> mouseMoveEvent;
    KDBindings::Signal<const KDGui::MouseWheelEvent*> mouseWheelEvent;
    KDBindings::Signal<const KDGui::KeyPressEvent*> keyPressEvent;
    KDBindings::Signal<const KDGui::KeyReleaseEvent*> keyReleaseEvent;

    void event(KDFoundation::EventReceiver* target, KDFoundation::Event* e)
    {
        switch (e->type()) {

        case KDFoundation::Event::Type::MousePress: {
            mousePressEvent.emit(static_cast<const KDGui::MousePressEvent*>(e));
            break;
        }
        case KDFoundation::Event::Type::MouseRelease: {
            mouseReleaseEvent.emit(static_cast<const KDGui::MouseReleaseEvent*>(e));
            break;
        }
        case KDFoundation::Event::Type::MouseMove: {
            mouseMoveEvent.emit(static_cast<const KDGui::MouseMoveEvent*>(e));
            break;
        }
        case KDFoundation::Event::Type::MouseWheel: {
            mouseWheelEvent.emit(static_cast<const KDGui::MouseWheelEvent*>(e));
            break;
        }
        case KDFoundation::Event::Type::KeyPress: {
            keyPressEvent.emit(static_cast<const KDGui::KeyPressEvent*>(e));
            break;
        }

        case KDFoundation::Event::Type::KeyRelease: {
            keyReleaseEvent.emit(static_cast<const KDGui::KeyReleaseEvent*>(e));
            break;
        }
        default:
            break;
        }
        KDFoundation::Object::event(target, e);
    }
};

} // namespace all::kdgui
