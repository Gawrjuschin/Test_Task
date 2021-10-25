#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QXmlStreamReader>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setMinimumSize({640,480});
    w.show();

    return a.exec();
}
