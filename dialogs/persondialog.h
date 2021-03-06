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

#pragma once

#include <QDialog>
#include <QAbstractButton>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QAction>

#include "../placesmodel.h"

namespace Ui {
class PersonDialog;
}

class PersonDialog: public QDialog
{
    Q_OBJECT

public:
    explicit PersonDialog(QWidget *parent = nullptr, int personID = -1);
    ~PersonDialog();

    void setMale();
    void setFemale();
    void setSurname(const QString & surname);

    int personID() const;

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void genderClicked(QAbstractButton * button);
    void deadAliveClicked(QAbstractButton * button);
    void save();

    void popupBirthDateCalendar();
    void popupDeathDateCalendar();
    void slotAddPlace();
    void slotSelectImage();

    void tabChanged(int index);
    void nextPartner();
    void prevPartner();

    void slotAddChild();
    void slotRemoveChild();
    void slotAddSibling();
    void slotRemoveSibling();
    void slotAddSpouse();

    void slotCurrentChildChanged();
    void slotCurrentSiblingChanged();

private:
    void populateControls();
    void populateFamilyTab();
    void updatePartnersLabel();
    void updatePartnersButtons();
    void fetchChildren();
    void fetchSiblings();
    QByteArray savePhoto() const;
    Ui::PersonDialog *ui;
    int m_personID;
    PlacesLookupModel * m_placesModel;
    QString m_photoFilename;

    // family tab
    bool m_familyInitted{false};
    QSqlQuery m_partnerQuery;
    QSqlQuery m_thisPersonQuery;
    QSqlQuery m_childrenQuery;
    QSqlQueryModel * m_childrenModel{nullptr};
    QSqlQuery m_parentsQuery;
    QSqlQuery m_siblingsQuery;
    QSqlQueryModel * m_siblingsModel{nullptr};
};
