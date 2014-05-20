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

#include "peoplemodel.h"

PeopleModel::PeopleModel(QObject *parent) :
    QSqlQueryModel(parent)
{
    exec();
}

PeopleModel::~PeopleModel()
{
}

void PeopleModel::exec()
{
    setQuery("SELECT id, sex, first_name, surname, birth_date, birth_place, death_date, death_place, occupation "
             "FROM People "
             "ORDER BY surname");
}

int PeopleModel::idAtRow(int row) const
{
    return record(row).value("id").toInt();
}

int PeopleModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant PeopleModel::data(const QModelIndex &item, int role) const
{
    if (role == Qt::DisplayRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 0) {
            const QString sex = rec.field("sex").value().toString();
            QString symbol;
            if (sex == "M")
                symbol = "♂";
            else if (sex == "F")
                symbol = "♀";
            return QString("%1 %2 %3").arg(symbol, rec.field("first_name").value().toString(), rec.field("surname").value().toString());
        } else if (column == 1) {
            QString result = QString::fromUtf8("✱ ");
            const QDate birthDate = rec.field("birth_date").value().toDate();
            if (birthDate.isValid()) {
                result += birthDate.toString(Qt::DefaultLocaleShortDate);
            } else {
                result += rec.field("birth_date").value().toString();
            }
            return result;
        } else if (column == 2) {
            return rec.field("birth_place").value();
        } else if (column == 3) {
            if (rec.field("death_date").isNull())
                return QVariant();
            QString result = QString::fromUtf8("✝ ");
            const QDate deathDate = rec.field("death_date").value().toDate();
            if (deathDate.isValid()) {
                result += deathDate.toString(Qt::DefaultLocaleShortDate);
            } else {
                result += rec.field("death_date").value().toString();
            }
            return result;
        } else if (column == 4) {
            return rec.field("death_place").value();
        } else if (column == 5) {
            return rec.field("occupation").value();
        }
    } else if (role == Qt::ToolTipRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 1) {
            const QDate birthDate = rec.field("birth_date").value().toDate();
            if (birthDate.isValid()) {
                return birthDate.toString(Qt::DefaultLocaleLongDate);
            }
        } else if (column == 3) {
            const QDate deathDate = rec.field("death_date").value().toDate();
            if (deathDate.isValid()) {
                return deathDate.toString(Qt::DefaultLocaleLongDate);
            }
        }
    }

    return QSqlQueryModel::data(item, role);
}

QVariant PeopleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Name");
        else if (section == 1)
            return tr("Birth Date");
        else if (section == 2)
            return tr("Birth Place");
        else if (section == 3)
            return tr("Death Date");
        else if (section == 4)
            return tr("Death Place");
        else if (section == 5)
            return tr("Occupation");
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}


PeopleLookupModel::PeopleLookupModel(QObject *parent):
    QSqlQueryModel(parent)
{
    setQuery("SELECT id, printf(\"%s %s\", first_name, surname) as name FROM People");
}

PeopleLookupModel::~PeopleLookupModel()
{
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
