#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationDisplayName(QApplication::tr("Pedikree"));
    a.setOrganizationDomain("kde.org");
    a.setOrganizationName("KDE");
    a.setApplicationVersion("0.1");
    a.setWindowIcon(QIcon::fromTheme("view-process-all-tree"));

    MainWindow w;
    w.show();

    return a.exec();
}
