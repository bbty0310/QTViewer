#include "QTViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTViewer w;
    w.show();
    return a.exec();
}
