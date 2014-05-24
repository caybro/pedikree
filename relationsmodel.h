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

#ifndef RELATIONSMODEL_H
#define RELATIONSMODEL_H

#include <QSqlQueryModel>
#include <QCoreApplication>

class Relations {
    Q_DECLARE_TR_FUNCTIONS(Relations)
public:
    static QMap<QString, QString> relations() { return m_relations; }
private:
    static QMap<QString, QString> m_relations;
};

class RelationsModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    RelationsModel(QObject *parent = 0);
    virtual ~RelationsModel();

    void exec();

    int idAtRow(int row) const;

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
};

#endif // RELATIONSMODEL_H
