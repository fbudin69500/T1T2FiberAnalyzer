#include "include/T1T2FiberAnalyzer.h"
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    T1T2FiberAnalyzer w;
    w.show();
    w.initializePyPath();

    return a.exec();
}
