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

#include <QMetaEnum>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QDebug>

#include "relationsmodel.h"

RelationsModel::RelationsModel(QObject *parent) :
    QSqlQueryModel(parent)
{
    exec();
}

RelationsModel::~RelationsModel()
{
}

void RelationsModel::exec()
{
    setQuery("SELECT DISTINCT r.id, r.type, "
             "printf(\"%s %s\", p1.first_name, p1.surname) as person1_name, "
             "printf(\"%s %s\", p2.first_name, p2.surname) as person2_name, "
             "r.place, r.date, r.comment "
             "FROM Relations r, People p1, People p2 "
             "JOIN People ON p1.id=r.person1_id "
             "JOIN People ON p2.id=r.person2_id");
    qDebug() << Q_FUNC_INFO << "Query status:" << lastError().text();
}

int RelationsModel::idAtRow(int row) const
{
    return record(row).value("id").toInt();
}

int RelationsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 6;
}

QVariant RelationsModel::data(const QModelIndex &item, int role) const
{
    if (role == Qt::DisplayRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 0) {
            return rec.field("type").value();
        } else if (column == 1) {
            return rec.field("person1_name").value();
        } else if (column == 2) {
            return rec.field("person2_name").value();
        } else if (column == 3) {
            return rec.field("place").value();
        } else if (column == 4) {
            return rec.field("date").value();
        } else if (column == 5) {
            return rec.field("comment").value();
        }
    }

    return QSqlQueryModel::data(item, role);
}

QVariant RelationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Type");
        else if (section == 1)
            return tr("Person 1 Name");
        else if (section == 2)
            return tr("Person 2 Name");
        else if (section == 3)
            return tr("Place");
        else if (section == 4)
            return tr("Date");
        else if (section == 5)
            return tr("Comment");
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}


Relation::Relation(QObject *parent):
    QObject(parent)
{
}

QString Relation::relationTypeAsString(int type) const
{
    return metaObject()->enumerator(metaObject()->indexOfEnumerator("RelationType")).valueToKey(type);
}
