/*
 * Copyright (C) 2014 Lukáš Tinkl <lukas@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QtSql>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //qDebug() << "Qt translations in" << QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    QTranslator qtTranslator;
    qtTranslator.load(QLocale::system(), QStringLiteral("qt_"), QString(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator appTrans;
    appTrans.load(QStringLiteral(":/translations/pedikree_") + QLocale::system().name());
    a.installTranslator(&appTrans);

    a.setApplicationDisplayName(QApplication::tr("Pedikree"));
    a.setOrganizationDomain("kde.org");
    a.setOrganizationName("KDE");
    a.setApplicationVersion("0.1");
    a.setWindowIcon(QIcon(":/icons/appicon.png"));

    if (!QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE"))) {
        QMessageBox::critical(0, QApplication::tr("No SQLLite DB support"),
                              QApplication::tr("Your Qt installation doesn't contain the required SQLLite DB plugin."));
        return -1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
