#include <TApplication.h>
#include "WindowMain.h"

int main(int argc, char** argv) {
    TApplication app("PciDAQ", &argc, argv);
    WindowMain* mainWindow = new WindowMain();
    app.Run(kTRUE);
    delete mainWindow;
    return 0;
}