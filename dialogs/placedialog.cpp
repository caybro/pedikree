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
#include <QGeoCodingManager>
#include <QGeoAddress>

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

    m_mapWidget = new MapWidget(this);
    m_mapWidget->setMinimumSize(QSize(300, 200));
    ui->verticalLayout->insertWidget(1, m_mapWidget, 1);
    if (m_placeID != -1) {
        updateMapCenter();
    }

    m_placeMenu = new QMenu(this);

    qDebug() << "!!! GEOCODE service providers:" << QGeoServiceProvider::availableServiceProviders();
    m_geoProvider = new QGeoServiceProvider(QStringLiteral("osm"));
    if (m_geoProvider->error() == QGeoServiceProvider::NoError) {
        m_gc = m_geoProvider->geocodingManager();
        if (!m_gc) {
            qWarning() << Q_FUNC_INFO << "Failed to acquire QGeoCodingManager:" << m_geoProvider->geocodingErrorString();
        } else {
            ui->btnGeoCode->setEnabled(true);
            connect(m_gc, &QGeoCodingManager::finished, this, &PlaceDialog::geocodeFinished);
            connect(m_placeMenu, &QMenu::triggered, this, &PlaceDialog::placeTriggered);
        }
    } else {
        qWarning() << Q_FUNC_INFO << "Failed to acquire QGeoServiceProvider:" << m_geoProvider->errorString();
    }

    connect(ui->sbLat, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PlaceDialog::updateMapCenter);
    connect(ui->sbLon, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PlaceDialog::updateMapCenter);
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
        query.prepare(QStringLiteral("INSERT INTO Places (name, lat, lon, comment) "
                                     "VALUES (:name, :lat, :lon, :comment)"));
        //qDebug() << "Adding place with query" << query.lastQuery();
    } else { // update place
        query.prepare(QStringLiteral("UPDATE Places SET name=:name, lat=:lat, lon=:lon, comment=:comment "
                                     "WHERE id=:id"));
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

    if (m_gc) {
        m_gc->geocode(query);

        ui->btnGeoCode->setCursor(Qt::BusyCursor);
    }
}

void PlaceDialog::geocodeFinished(QGeoCodeReply *reply)
{
    reply->deleteLater();
    ui->btnGeoCode->unsetCursor();

    if (reply->error() == QGeoCodeReply::NoError && !reply->locations().isEmpty()) {
        for (const auto &loc: reply->locations()) {
            QAction * action = new QAction(m_placeMenu);
            action->setText(loc.address().text());
            const auto coord = loc.coordinate();
            action->setData(QStringLiteral("%1:%2:%3").arg(coord.latitude()).arg(coord.longitude()).arg(action->text()));
            m_placeMenu->addAction(action);
        }

        if (!m_placeMenu->actions().isEmpty()) {
            ui->btnGeoCode->setText(tr("Suggestions available"));
            ui->btnGeoCode->setMenu(m_placeMenu);
        }
    } else {
        qWarning() << Q_FUNC_INFO << "Geocoding finished with error:" << reply->errorString();
    }
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
    ui->btnGeoCode->setMenu(nullptr);
}

void PlaceDialog::updateMapCenter()
{
    m_mapWidget->setCenter(ui->sbLat->value(), ui->sbLon->value());
}

void PlaceDialog::populateControls()
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT id, name, lat, lon, comment "
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
