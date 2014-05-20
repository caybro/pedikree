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

#include "placedialog.h"
#include "ui_placedialog.h"

PlaceDialog::PlaceDialog(QWidget *parent, int placeID) :
    QDialog(parent),
    ui(new Ui::PlaceDialog),
    m_placeID(placeID)
{
    ui->setupUi(this);

    if (m_placeID != -1) { // editting, fill the controls
        populateControls();
    }

    connect(this, &PlaceDialog::accepted, this, &PlaceDialog::save);

    qDebug() << "Editting place with ID:" << m_placeID;
}

PlaceDialog::~PlaceDialog()
{
    delete ui;
}

void PlaceDialog::changeEvent(QEvent *e)
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

void PlaceDialog::save()
{
    QSqlQuery query;

    if (m_placeID == -1) { // add place
        query.prepare("INSERT INTO Places (name, lat, lon, comment) "
                      "VALUES (:name, :lat, :lon, :comment)");
        //qDebug() << "Adding place with query" << query.lastQuery();
    } else { // update place
        query.prepare("UPDATE Places SET name=:name, lat=:lat, lon=:lon, comment=:comment "
                      "WHERE id=:id");
        query.bindValue(":id", m_placeID);
    }

    query.bindValue(":name", ui->leName->text());
    query.bindValue(":lat", ui->sbLat->value());
    query.bindValue(":lon", ui->sbLon->value());
    query.bindValue(":comment", ui->teComment->toPlainText());

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
    }
}

void PlaceDialog::populateControls()
{
    QSqlQuery query;
    query.prepare(QString("SELECT id, name, lat, lon, comment "
                  "FROM Places "
                  "WHERE id=%1").arg(m_placeID));

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        return;
    }
    else
        query.first();

    ui->leName->setText(query.value("name").toString());
    ui->sbLat->setValue(query.value("lat").toDouble());
    ui->sbLon->setValue(query.value("lon").toDouble());
    ui->teComment->setPlainText(query.value("comment").toString());
}
