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

#include <QSqlRecord>
#include <QSqlField>

#include "placesmodel.h"

PlacesModel::PlacesModel(QObject *parent):
    QSqlQueryModel(parent)
{
    exec();
}

PlacesModel::~PlacesModel()
{
}

void PlacesModel::exec()
{
    setQuery("SELECT id, name, lat, lon, comment FROM Places ORDER BY name");
}

int PlacesModel::idAtRow(int row) const
{
    return record(row).value("id").toInt();
}

int PlacesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 4;
}

QVariant PlacesModel::data(const QModelIndex &item, int role) const
{
    if (role == Qt::DisplayRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 0) {
            return rec.field("name").value();
        } else if (column == 1) {
            return rec.field("lat").value();
        } else if (column == 2) {
            return rec.field("lon").value();
        } else if (column == 3) {
            return rec.field("comment").value();
        }
    }

    return QSqlQueryModel::data(item, role);
}

QVariant PlacesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Name");
        else if (section == 1)
            return tr("Latitude");
        else if (section == 2)
            return tr("Longitude");
        else if (section == 3)
            return tr("Comment");
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}


PlacesLookupModel::PlacesLookupModel(QObject *parent):
    QSqlQueryModel(parent)
{
    setQuery("SELECT id, name FROM Places");
}

PlacesLookupModel::~PlacesLookupModel()
{
}

QVariant PlacesLookupModel::data(const QModelIndex &item, int role) const
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
