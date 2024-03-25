#pragma once
#include "ui/camera_controller.h"
#include "ui/style.h"

namespace all::qt {
class QMLNodes
{
public:
    QMLNodes()
    {
        qmlRegisterSingletonInstance("Schneider", 1, 0, "Scene", &scene);
        qmlRegisterSingletonInstance("Schneider", 1, 0, "Camera", &camera);
        qmlRegisterSingletonInstance("Schneider", 1, 0, "Cursor", &cursor);
        qmlRegisterSingletonInstance("Schneider", 1, 0, "Style", &style);
    }

public:
    SceneController scene;
    CameraController camera;
    CursorController cursor;
    AppStyle style;
};
}