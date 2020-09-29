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

#include <QDebug>
#include <QDate>
#include <QSqlRecord>
#include <QSqlField>
#include <QIcon>

#include "peoplemodel.h"

PeopleModel::PeopleModel(QObject *parent) :
    QSqlQueryModel(parent)
{
    exec();
}

void PeopleModel::exec()
{
    setQuery(QStringLiteral("SELECT id, sex, first_name, surname, suffix, birth_date, birth_place, death_date, death_place, occupation, alive "
                            "FROM People"));
}

int PeopleModel::idAtRow(int row) const
{
    return record(row).value("id").toInt();
}

int PeopleModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 7;
}

QVariant PeopleModel::data(const QModelIndex &item, int role) const
{
    if (role == Qt::DisplayRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 0) {
            return rec.field("first_name").value();
        } else if (column == 1) {
            return QStringLiteral("%1 %2").arg(rec.field("surname").value().toString(),
                                               rec.field("suffix").value().toString().simplified());
        } else if (column == 2) {
            const QDate birthDate = rec.field("birth_date").value().toDate();
            if (birthDate.isValid()) {
                return birthDate;
            }
            return rec.field("birth_date").value();
        } else if (column == 3) {
            return rec.field("birth_place").value();
        } else if (column == 4) {
            const bool alive = rec.field("alive").value().toBool();
            if (alive)
                return tr("Alive");
            const QVariant dead = rec.field("death_date").value();
            const QDate deathDate = dead.toDate();
            if (deathDate.isValid()) {
                return deathDate;
            } else if (!dead.toString().isEmpty()) {
                return dead;
            }
            return tr("Deceased");
        } else if (column == 5) {
            return rec.field("death_place").value();
        } else if (column == 6) {
            return rec.field("occupation").value();
        }
    } else if (role == Qt::ToolTipRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 2) {
            const QDate birthDate = rec.field("birth_date").value().toDate();
            if (birthDate.isValid()) {
                return birthDate.toString(Qt::DefaultLocaleLongDate);
            }
        } else if (column == 4) {
            const QDate deathDate = rec.field("death_date").value().toDate();
            if (deathDate.isValid()) {
                return deathDate.toString(Qt::DefaultLocaleLongDate);
            }
        }
    } else if (role == Qt::DecorationRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 0) {
            const QString sex = rec.value("sex").toString();
            if (sex == "M") {
                return QIcon(":/icons/male.png");
            } else if (sex == "F") {
                return QIcon(":/icons/female.png");
            }
            return QVariant();
        }
    }

    return QSqlQueryModel::data(item, role);
}

QVariant PeopleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("First Name");
        if (section == 1)
            return tr("Surname");
        else if (section == 2)
            return tr("Birth Date");
        else if (section == 3)
            return tr("Birth Place");
        else if (section == 4)
            return tr("Death Date");
        else if (section == 5)
            return tr("Death Place");
        else if (section == 6)
            return tr("Occupation");
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}


PeopleLookupModel::PeopleLookupModel(QObject *parent):
    QSqlQueryModel(parent)
{
    setQuery(QStringLiteral("SELECT id, printf(\"%s %s %s\", first_name, surname, suffix) as name FROM People ORDER BY surname"));
}

QVariant PeopleLookupModel::data(const QModelIndex &item, int role) const
{
    const int column = item.column();

    if (role == Qt::DisplayRole) {
        const QSqlRecord rec = record(item.row());
        if (column == 0) {
            return rec.field("id").value();
        } else if (column == 1) {
            return rec.field("name").value();
        }
    } else if (role == Qt::UserRole) {
        const QSqlRecord rec = record(item.row());
        return rec.field("id").value();
    }

    return QSqlQueryModel::data(item, role);
}
