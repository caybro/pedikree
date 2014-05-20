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

#ifndef RELATIONDIALOG_H
#define RELATIONDIALOG_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class RelationDialog;
}

class RelationDialog: public QDialog
{
    Q_OBJECT

public:
    explicit RelationDialog(QWidget *parent = 0, int relationID = -1);
    ~RelationDialog();

protected:
    void changeEvent(QEvent *e);

private slots:
    void save();
    void popupCalendar();

private:
    void populateControls();
    Ui::RelationDialog *ui;
    int m_relationID;
    QSqlQueryModel * m_placesModel;
    QSqlQueryModel * m_peopleModel;
};

#endif // RELATIONDIALOG_H
