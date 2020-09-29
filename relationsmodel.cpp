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
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QDate>
#include <QDebug>
#include <QtGlobal>
#include <QIcon>

#include "relationsmodel.h"

RelationsModel::RelationsModel(QObject *parent) :
    QSqlQueryModel(parent)
{
    exec();
}

void RelationsModel::exec()
{
    setQuery(QStringLiteral("SELECT DISTINCT r.id, r.type, "
                            "printf(\"%s %s %s\", p1.first_name, p1.surname, p1.suffix) as person1_name, "
                            "printf(\"%s %s %s\", p2.first_name, p2.surname, p2.suffix) as person2_name, "
                            "(SELECT printf(\"%s %s %s\", first_name, surname, suffix) FROM People WHERE id=r.child_id) AS child_name, "
                            "r.child_id, r.place, r.date, r.comment "
                            "FROM Relations r, People p1, People p2 "
                            "JOIN People ON p1.id=r.person1_id "
                            "JOIN People ON p2.id=r.person2_id"));
    qDebug() << Q_FUNC_INFO << "Query status:" << lastError().text();
}

int RelationsModel::idAtRow(int row) const
{
    return record(row).value("id").toInt();
}

int RelationsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 7;
}

QVariant RelationsModel::data(const QModelIndex &item, int role) const
{
    if (role == Qt::DisplayRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 0) {
            return qApp->translate("Relations", Relations::relations().value(rec.field("type").value().toString()).toUtf8());
        } else if (column == 1) {
            return rec.field("person1_name").value();
        } else if (column == 2) {
            return rec.field("person2_name").value();
        } else if (column == 3) {
            return rec.field("child_name").value();
        } else if (column == 4) {
            return rec.field("place").value();
        } else if (column == 5) {
            const QDate date = rec.field("date").value().toDate();
            if (date.isValid()) {
                return date;
            }
            return rec.field("date").value();
        } else if (column == 6) {
            return rec.field("comment").value();
        }
    } else if (role == Qt::ToolTipRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 5) {
            const QDate date = rec.field("date").value().toDate();
            if (date.isValid()) {
                return date.toString(Qt::DefaultLocaleLongDate);
            }
        }
    } else if (role == Qt::DecorationRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 0) {
            const QString type = rec.value("type").toString();
            if (type.contains("Parent")) {
                return QIcon(":/icons/baby-32.png");
            } else if (type == "Marriage" || type == "DomesticPartnership" || type == "CivilUnion" || type == "Annulment") {
                return QIcon(":/icons/wedding_rings-32.png");
            }
            return QVariant();
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
            return tr("Child Name");
        else if (section == 4)
            return tr("Place");
        else if (section == 5)
            return tr("Date");
        else if (section == 6)
            return tr("Comment");
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}


QMap<QString,QString> Relations::m_relations = {
    {"Annulment", QT_TR_NOOP("Annulment")},
    {"CommonLawMarriage", QT_TR_NOOP("Common Law Marriage")},
    {"CivilUnion", QT_TR_NOOP("Civil Union")},
    {"DomesticPartnership", QT_TR_NOOP("Domestic Partnership")},
    {"Divorce", QT_TR_NOOP("Divorce")},
    {"DivorceFiling", QT_TR_NOOP("Divorce Filing")},
    {"Engagement", QT_TR_NOOP("Engagement")},
    {"Marriage", QT_TR_NOOP("Marriage")},
    {"MarriageBanns", QT_TR_NOOP("Marriage Banns")},
    {"MarriageContract", QT_TR_NOOP("Marriage Contract")},
    {"MarriageLicense", QT_TR_NOOP("Marriage License")},
    {"MarriageNotice", QT_TR_NOOP("Marriage Notice")},
    {"Separation", QT_TR_NOOP("Separation")},

    {"AdoptiveParent", QT_TR_NOOP("Adoptive Parent")},
    {"BiologicalParent", QT_TR_NOOP("Biological Parent")},
    {"FosterParent", QT_TR_NOOP("Foster Parent")},
    {"GuardianParent", QT_TR_NOOP("Guardian Parent")},
    {"StepParent", QT_TR_NOOP("Step Parent")},
    {"SociologicalParent", QT_TR_NOOP("Socialogical Parent")},
    {"SurrogateParent", QT_TR_NOOP("Surrogate Parent")}
};

QMap<QString, QString> Relations::relations()
{
    return m_relations;
}
