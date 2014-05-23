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
    m_placeID(placeID),
    m_gc(0)
{
    ui->setupUi(this);

    if (m_placeID != -1) { // editting, fill the controls
        populateControls();
    }

    m_gc = new OsmGeoCoder(this);
    m_placeMenu = new QMenu(this);

    connect(ui->btnGeoCode, &QPushButton::clicked, this, &PlaceDialog::geocode);
    connect(this, &PlaceDialog::accepted, this, &PlaceDialog::save);

    qDebug() << "Editting place with ID:" << m_placeID;
}

PlaceDialog::~PlaceDialog()
{
    delete ui;
}

int PlaceDialog::placeId() const
{
    return m_placeID;
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
        return;
    }

    if (query.lastInsertId().isValid())
        m_placeID = query.lastInsertId().toInt();
}

void PlaceDialog::geocode()
{
    const QString query = ui->leName->text();
    qDebug() << "Geocoding" << query;

    m_gc->geocode(query);

    ui->btnGeoCode->setCursor(Qt::BusyCursor);

    connect(m_gc, SIGNAL(geocodeReply(QString,QString,QString,QString,QString)),
            this, SLOT(geocodeReply(QString,QString,QString,QString,QString)), Qt::UniqueConnection);
    connect(m_gc, SIGNAL(geocodeFinished(QString)), this, SLOT(geocodeFinished(QString)), Qt::UniqueConnection);
}

void PlaceDialog::geocodeReply(const QString &originalQuery, const QString &lat, const QString &lon, const QString &displayName, const QString &osmId)
{
    qDebug() << "GEOCODE RESULT" << originalQuery << lat << lon << displayName << osmId;
    if (originalQuery != ui->leName->text())
        return;

    QAction * action = new QAction(m_placeMenu);
    action->setText(displayName);
    action->setData(QString("%1:%2:%3").arg(lat).arg(lon).arg(displayName));
    m_placeMenu->addAction(action);
    connect(m_placeMenu, SIGNAL(triggered(QAction*)), this, SLOT(placeTriggered(QAction*)), Qt::UniqueConnection);
}

void PlaceDialog::geocodeFinished(const QString &originalQuery)
{
    qDebug() << "GEOCODE FINISHED" << originalQuery;
    if (!m_placeMenu->actions().isEmpty()) {
        ui->btnGeoCode->setText(tr("Suggestions available"));
        ui->btnGeoCode->setMenu(m_placeMenu);
    }
    ui->btnGeoCode->unsetCursor();
}

void PlaceDialog::placeTriggered(QAction *action)
{
    qDebug() << "Place triggered" << action->text().replace('&', "");

    const QString userData = action->data().toString();

    // fill the controls with the action's data
    ui->leName->setText(userData.section(':', 2, 2));
    ui->sbLat->setValue(userData.section(':', 0, 0).toDouble());
    ui->sbLon->setValue(userData.section(':', 1, 1).toDouble());

    ui->btnGeoCode->setText(QString());
    m_placeMenu->clear();
    ui->btnGeoCode->setMenu(0);
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
