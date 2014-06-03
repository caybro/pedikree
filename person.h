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

#ifndef PERSON_H
#define PERSON_H

#include <QString>

class Person
{
public:
    explicit Person(int personID);

    int fatherID() const;
    int motherID() const;
    QList<int> spouseIDs() const;
    QList<int> siblingIDs() const;
    QList<int> childrenIDs() const;
    QString fullName() const;

    static QString personFullName(int personID);

private:
    int getParentId(const QString & sex) const;
    int m_personID;
};

#endif // PERSON_H
