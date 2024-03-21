#include <app.h>

#include <QtWidgets>
#include <signal.h>

#ifdef _WIN32
void signalHandler(int signal)
{
    if (signal == SIGABRT) {
        auto eptr = std::current_exception();
        std::string err_msg = "An unknown Error occured.";
        try {
            if (eptr)
                std::rethrow_exception(eptr);
        } catch (const std::exception& e) {
            err_msg = e.what();
        }

        QMessageBox{ QMessageBox::Critical, "Schneider Demo has exited", QString{ "Demo has exited unexpectedly\n\n%1" }.arg(err_msg.c_str()) }.exec();
    } else if (signal == SIGSEGV) {
        QMessageBox{ QMessageBox::Critical, "Schneider Demo has exited", "Demo has exited unexpectedly" }.exec();
    }
}
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    signal(SIGABRT, signalHandler);
#endif

    qputenv("QT3D_RENDERER", "opengl");

    return App{ argc, argv }.Start();
}
