#include <app.h>

int main(int argc, char** argv)
{
    qputenv("QT3D_RENDERER", "opengl");
//    qputenv("QSG_INFO", "true");
//    qputenv("QT_LOGGING_RULES", "qt.rhi.general.debug=true");
    return App{ argc, argv }.Start();
}
