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

class Relation: public QObject {
    Q_OBJECT
    Q_ENUMS(RelationType)
public:
    enum RelationType {
        // couples
        Annulment = 0, // The fact of an annulment of a marriage
        CommonLawMarriage, // The fact of a marriage by common law
        CivilUnion, // The fact of a civil union of a couple
        DomesticPartnership, // The fact of a domestic partnership of a couple
        Divorce, // The fact of a divorce of a couple
        DivorceFiling, // The fact of a filing for divorce
        Engagement, // The fact of an engagement to be married
        Marriage, // The fact of a marriage
        MarriageBanns, // The fact of a marriage banns
        MarriageContract, // The fact of a marriage contract
        MarriageLicense, // The fact of a marriage license
        MarriageNotice, // The fact of a marriage notice
        Separation, // A fact of a couple's separation

        // parent-child
        AdoptiveParent, // A fact about an adoptive relationship between a parent and a child
        BiologicalParent, // A fact the biological relationship between a parent and a child
        FosterParent, // A fact about a foster relationship between a foster parent and a child
        GuardianParent, // A fact about a legal guardianship between a parent and a child
        StepParent, // A fact about the step relationship between a parent and a child
        SociologicalParent, // A fact about a sociological relationship between a parent and a child, but not definable in typical legal or biological terms
        SurrogateParent, // A fact about a pregnancy surrogate relationship between a parent and a child
        LastRelationType = SurrogateParent
    };

    Relation(QObject *parent = 0);

    QString relationTypeAsString(int type) const;
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
