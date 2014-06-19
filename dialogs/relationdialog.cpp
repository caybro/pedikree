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

    setChildMode(false);

    QMapIterator<QString, QString> i(Relations::relations());
    while (i.hasNext()) {
        i.next();
        ui->type->addItem(qApp->translate("Relations", i.value().toUtf8()), i.key());
    }

    m_peopleModel = new PeopleLookupModel(this);
    ui->person1->setModel(m_peopleModel);
    ui->person1->setModelColumn(1);
    ui->person2->setModel(m_peopleModel);
    ui->person2->setModelColumn(1);
    ui->child->setModel(m_peopleModel);
    ui->child->setModelColumn(1);

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

    connect(ui->type, SIGNAL(activated(int)), SLOT(slotTypeChanged(int)));
    connect(ui->person1, SIGNAL(currentIndexChanged(int)), SLOT(slotTypeChanged(int)));
    connect(ui->person2, SIGNAL(currentIndexChanged(int)), SLOT(slotTypeChanged(int)));
    connect(ui->child, SIGNAL(currentIndexChanged(int)), SLOT(slotTypeChanged(int)));

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
        query.prepare(QStringLiteral("INSERT INTO Relations (type, person1_id, person2_id, child_id, place, date, comment) "
                                     "VALUES (:type, :person1_id, :person2_id, :child_id, :place, :date, :comment)"));
    } else { // update relation
        query.prepare(QStringLiteral("UPDATE Relations "
                                     "SET type=:type, person1_id=:person1_id, person2_id=:person2_id, child_id=:child_id, place=:place, date=:date, comment=:comment "
                                     "WHERE id=:id"));
        query.bindValue(":id", m_relationID);
    }

    query.bindValue(":type", ui->type->currentData());
    query.bindValue(":person1_id", ui->person1->currentData());
    query.bindValue(":person2_id", ui->person2->currentData());
    if (ui->type->currentData().toString().contains("Parent")) {
        query.bindValue(":child_id", ui->child->currentData());
    }
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

    const QString type = ui->type->currentData().toString();
    if (type.contains("Parent") && ui->person1->currentData() != ui->person2->currentData()) {
        qDebug() << "Type changed to parent/child";

        setChildMode(true);

        QSqlQuery query(QStringLiteral("SELECT birth_date, birth_place FROM People WHERE id=%1").arg(ui->child->currentData().toInt()));
        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
            return;
        }
        else
            query.first();
        ui->date->setText(query.value("birth_date").toString());
        ui->place->setEditText(query.value("birth_place").toString()); // TODO place id
    } else {
        setChildMode(false);
        ui->place->setEditText(QString());
        ui->date->setText(QString());
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
    query.prepare(QStringLiteral("SELECT id, type, person1_id, person2_id, child_id, place, date, comment "
                                 "FROM Relations "
                                 "WHERE id=%1").arg(m_relationID));

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        return;
    }
    else
        query.first();

    ui->type->setCurrentIndex(ui->type->findData(query.value("type").toString()));
    ui->person1->setCurrentIndex(ui->person1->findData(query.value("person1_id").toInt()));
    ui->person2->setCurrentIndex(ui->person2->findData(query.value("person2_id").toInt()));
    ui->child->setCurrentIndex(ui->child->findData(query.value("child_id").toInt()));
    ui->place->setCurrentIndex(ui->place->findText(query.value("place").toString()));
    // TODO place_id
    ui->date->setText(query.value("date").toString());
    ui->comment->setPlainText(query.value("comment").toString());

    if (ui->type->currentData().toString().contains("Parent")) { //enable the child combo
        setChildMode(true);
    } else {
        setChildMode(false);
    }
}

void RelationDialog::setChildMode(bool on)
{
    ui->child->setVisible(on);
    ui->childLabel->setVisible(on);
    if (on) {
        ui->person1Label->setText(tr("Parent 1:"));
        ui->person2Label->setText(tr("Parent 2:"));
    } else {
        ui->person1Label->setText(tr("Person 1:"));
        ui->person2Label->setText(tr("Person 2:"));
    }
}
