#include <app.h>

#include <QtWidgets>

int main(int argc, char** argv)
{
    qputenv("QT3D_RENDERER", "opengl");

    return App{ argc, argv }.Start();
}
