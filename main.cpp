#include "Watermark.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Watermark w;
    w.show();

    return a.exec();
}
