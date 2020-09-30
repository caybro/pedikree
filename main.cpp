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

#include <QApplication>
#include <QMessageBox>
#include <QtSql>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);

    //qDebug() << "Qt translations in" << QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    QTranslator qtTranslator;
    qtTranslator.load(QLocale(), QStringLiteral("qt_"), QString(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator appTrans;
    appTrans.load(QLocale(), QStringLiteral("pedikree"), QStringLiteral("_"), QStringLiteral(":/translations/"));
    a.installTranslator(&appTrans);

    a.setApplicationDisplayName(QApplication::tr("Pedikree"));
    a.setOrganizationDomain(QStringLiteral("kde.org"));
    a.setOrganizationName(QStringLiteral("KDE"));
    a.setApplicationVersion(QStringLiteral("0.1"));
    a.setWindowIcon(QIcon(QStringLiteral(":/icons/appicon.png")));

    if (!QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE"))) {
        QMessageBox::critical(nullptr, QApplication::tr("No SQLLite DB support"),
                              QApplication::tr("Your Qt installation doesn't contain the required SQLLite DB plugin."));
        return EXIT_FAILURE;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
