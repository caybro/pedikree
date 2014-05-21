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
#include <QSqlQuery>
#include <QSqlError>

#include "../relationsmodel.h"

#include "relationdialog.h"
#include "pddatedialog.h"
#include "placedialog.h"
#include "ui_relationdialog.h"

RelationDialog::RelationDialog(QWidget *parent, int relationID) :
    QDialog(parent),
    ui(new Ui::RelationDialog),
    m_relationID(relationID)
{
    ui->setupUi(this);

    Relation * rel = new Relation(this);
    for (int i = 0; i < Relation::LastRelationType; i++) {
        ui->type->addItem(rel->relationTypeAsString(i), i);
    }
    delete rel;

    m_peopleModel = new PeopleLookupModel(this);
    ui->person1->setModel(m_peopleModel);
    ui->person1->setModelColumn(1);
    ui->person2->setModel(m_peopleModel);
    ui->person2->setModelColumn(1);

    m_placesModel = new PlacesLookupModel(this);
    ui->place->setModel(m_placesModel);
    ui->place->setModelColumn(1);

    QAction * tmp = ui->date->addAction(QIcon::fromTheme("view-calendar-day"), QLineEdit::TrailingPosition);
    tmp->setToolTip(tr("Enter date"));
    connect(tmp, &QAction::triggered, this, &RelationDialog::popupCalendar);

    if (m_relationID != -1) { // editting, fill the controls
        populateControls();
    } else {
        ui->place->setEditText(QString());
    }

    connect(ui->type, SIGNAL(currentIndexChanged(int)), SLOT(slotTypeChanged(int)));
    connect(ui->person1, SIGNAL(currentIndexChanged(int)), SLOT(slotTypeChanged(int)));
    connect(ui->person2, SIGNAL(currentIndexChanged(int)), SLOT(slotTypeChanged(int)));

    connect(ui->btnAddPlace, &QPushButton::clicked, this, &RelationDialog::slotAddPlace);

    connect(this, &RelationDialog::accepted, this, &RelationDialog::save);

    qDebug() << "Editting relation with ID:" << m_relationID;
}

RelationDialog::~RelationDialog()
{
    delete ui;
}

void RelationDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void RelationDialog::save()
{
    QSqlQuery query;

    if (m_relationID == -1) { // add relation
        query.prepare("INSERT INTO Relations (type, person1_id, person2_id, place, date, comment) "
                      "VALUES (:type, :person1_id, :person2_id, :place, :date, :comment)");
    } else { // update relation
        query.prepare("UPDATE Relations "
                      "SET type=:type, person1_id=:person1_id, person2_id=:person2_id, place=:place, date=:date, comment=:comment "
                      "WHERE id=:id");
        query.bindValue(":id", m_relationID);
    }

    query.bindValue(":type", ui->type->currentText());
    query.bindValue(":person1_id", ui->person1->currentData());
    query.bindValue(":person2_id", ui->person2->currentData());
    query.bindValue(":place", ui->place->currentText());
    // TODO place id
    query.bindValue(":date", ui->date->text());
    query.bindValue(":comment", ui->comment->toPlainText());

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
    }
}

void RelationDialog::popupCalendar()
{
    PdDateDialog * dlg = new PdDateDialog(ui->date->text(), this);
    if (dlg->exec() == QDialog::Accepted) {
        ui->date->setText(dlg->date());
    }
    delete dlg;
}

void RelationDialog::slotTypeChanged(int index)
{
    Q_UNUSED(index)

    const int currentType = ui->type->currentData().toInt();
    if (currentType >= Relation::AdoptiveParent && currentType <= Relation::SurrogateParent
            && ui->person1->currentData() != ui->person2->currentData()) {
        qDebug() << "Type changed to parent/child";

        QSqlQuery query(QString("SELECT birth_date, birth_place FROM People WHERE id=%1").arg(ui->person2->currentData().toInt()));
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
            return;
        }
        else
            query.first();
        ui->date->setText(query.value("birth_date").toString());
        ui->place->setEditText(query.value("birth_place").toString());
    }
}

void RelationDialog::slotAddPlace()
{
    PlaceDialog * dlg = new PlaceDialog(this);
    dlg->setWindowTitle(tr("Add Place"));
    if (dlg->exec() == QDialog::Accepted) {
        m_placesModel->reload();

        QPushButton * btn = qobject_cast<QPushButton *>(sender());
        if (btn == ui->btnAddPlace) {
            ui->place->setCurrentIndex(ui->place->findData(dlg->placeId()));
        }
    }
}

void RelationDialog::populateControls()
{
    QSqlQuery query;
    query.prepare(QString("SELECT id, type, person1_id, person2_id, place, date, comment "
                  "FROM Relations "
                  "WHERE id=%1").arg(m_relationID));

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        return;
    }
    else
        query.first();

    ui->type->setCurrentIndex(ui->type->findText(query.value("type").toString()));
    ui->person1->setCurrentIndex(ui->person1->findData(query.value("person1_id").toInt()));
    ui->person2->setCurrentIndex(ui->person2->findData(query.value("person2_id").toInt()));
    ui->place->setCurrentIndex(ui->place->findText(query.value("place").toString()));
    // TODO place_id
    ui->date->setText(query.value("date").toString());
    ui->comment->setPlainText(query.value("comment").toString());
}
