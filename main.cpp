#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QtSql>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationDisplayName(QApplication::tr("Pedikree"));
    a.setOrganizationDomain("kde.org");
    a.setOrganizationName("KDE");
    a.setApplicationVersion("0.1");
    a.setWindowIcon(QIcon::fromTheme("view-process-all-tree"));

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        QMessageBox::critical(0, QApplication::tr("No SQLLite DB support"),
                              QApplication::tr("Your Qt installation doesn't contain the required SQLLite DB plugin."));
        return -1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
