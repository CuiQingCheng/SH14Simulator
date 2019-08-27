#include "mainwindow.h"
#include <QApplication>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QFile file(":/qss/main.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = file.readAll();
    w.setStyleSheet(styleSheet);

    w.show();

    return a.exec();
}
