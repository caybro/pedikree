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
#include <QMenu>
#include <QGeoServiceProvider>
#include <QGeoCodeReply>

#include "../mapwidget/mapwidget.h"

namespace Ui {
class PlaceDialog;
}

class PlaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlaceDialog(QWidget *parent = nullptr, int placeID = -1);
    ~PlaceDialog();

    int placeId() const;

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void save();
    void geocode();
    void geocodeFinished(QGeoCodeReply *reply);
    void placeTriggered(QAction * action);
    void updateMapCenter();

private:
    void populateControls();
    Ui::PlaceDialog *ui;
    int m_placeID;
    QGeoServiceProvider *m_geoProvider{nullptr};
    QGeoCodingManager * m_gc{nullptr};
    QMenu * m_placeMenu{nullptr};
    MapWidget * m_mapWidget{nullptr};
};
