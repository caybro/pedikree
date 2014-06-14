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

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

#include "person.h"

Person::Person(int personID):
    m_personID(personID)
{
}

int Person::fatherID() const
{
    return getParentId("M");
}

int Person::motherID() const
{
    return getParentId("F");
}

QList<int> Person::spouseIDs() const
{
    QSqlQuery query(QStringLiteral("SELECT p.id "
                                   "FROM Relations r, People p "
                                   "WHERE ((r.person1_id=%1 AND r.person2_id=p.id) OR (r.person2_id=%1 AND r.person1_id=p.id)) "
                                   "AND r.type IN ('Annulment', 'CommonLawMarriage', 'CivilUnion', 'DomesticPartnership', 'Divorce', 'DivorceFiling', "
                                   "           'Engagement', 'Marriage', 'MarriageBanns', 'MarriageContract', 'MarriageLicense', 'MarriageNotice', 'Separation')")
                    .arg(m_personID));

    QList<int> result;
    if (query.exec()) {
        while (query.next()) {
            result.append(query.value(0).toInt());
        }
        return result;
    } else {
        qDebug() << Q_FUNC_INFO << "No spouse found for" << m_personID << query.lastError().text();
    }

    return QList<int>();
}

QList<int> Person::siblingIDs() const
{
    QSqlQuery query = QSqlQuery(QStringLiteral("SELECT DISTINCT p.id "
                                               "FROM People p, Relations r "
                                               "WHERE p.id=r.child_id AND r.child_id!=%3 AND ((r.person1_id=%1 AND r.person2_id=%2) OR (r.person1_id=%2 AND r.person2_id=%1))")
                                .arg(fatherID()).arg(motherID()).arg(m_personID));

    QList<int> result;
    if (query.exec()) {
        while (query.next()) {
            result.append(query.value(0).toInt());
        }
        return result;
    } else {
        qDebug() << Q_FUNC_INFO << "No siblings found for" << m_personID << query.lastError().text();
    }

    return QList<int>();
}

QList<int> Person::childrenIDs() const
{
    QSqlQuery query = QSqlQuery(QStringLiteral("SELECT DISTINCT p.id "
                                               "FROM People p, Relations r "
                                               "WHERE (r.person1_id=%1 OR r.person2_id=%1) AND p.id=child_id "
                                               "AND r.type IN ('AdoptiveParent', 'BiologicalParent', 'FosterParent', 'GuardianParent', 'StepParent', 'SociologicalParent', 'SurrogateParent')")
                                .arg(m_personID));

    QList<int> result;
    if (query.exec()) {
        while (query.next()) {
            result.append(query.value(0).toInt());
        }
        return result;
    } else {
        qDebug() << Q_FUNC_INFO << "No children found for" << m_personID << query.lastError().text();
    }

    return QList<int>();
}

QString Person::fullName() const
{
    return Person::personFullName(m_personID);
}

QString Person::personFullName(int personID)
{
    QSqlQuery query(QStringLiteral("SELECT printf(\"%s %s %s\", first_name, surname, suffix) as name FROM People WHERE id=%1").arg(personID));
    if (query.exec() && query.first()) {
        return query.value("name").toString();
    } else {
        qWarning() << Q_FUNC_INFO << "Person full name query failed with" << query.lastError().text();
    }

    return QString();
}

int Person::getParentId(const QString &sex) const
{
    QString queryString = QStringLiteral("SELECT %1 "
                                         "FROM Relations r, People p "
                                         "WHERE r.child_id=%2 AND %1=p.id AND p.sex='%3' "
                                         "AND r.type IN ('AdoptiveParent', 'BiologicalParent', 'FosterParent', 'GuardianParent', 'StepParent', 'SociologicalParent', 'SurrogateParent')");
    QSqlQuery query(queryString.arg("r.person1_id").arg(m_personID).arg(sex));
    if (query.exec() && query.first()) {
        return query.value(0).toInt();
    } else {
        query.prepare(queryString.arg("r.person2_id").arg(m_personID).arg(sex));
        if (query.exec() && query.first()) {
            return query.value(0).toInt();
        } else {
            //qDebug() << Q_FUNC_INFO << "No parent" << sex << "found for" << m_personID;
        }
    }

    return -1;
}
