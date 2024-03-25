#include "app.h"

#include <QMessageBox>
#include <signal.h>

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

int main(int argc, char** argv)
{
    signal(SIGABRT, signalHandler);
    qputenv("QT3D_RENDERER", "opengl");

    return all::qt::App{ argc, argv }.Start();
}
