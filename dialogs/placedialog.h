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

#ifndef PLACEDIALOG_H
#define PLACEDIALOG_H

#include <QDialog>
#include <QMenu>

#include "osmgeocoder.h"
#include "../mapwidget/mapwidget.h"

namespace Ui {
class PlaceDialog;
}

class PlaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlaceDialog(QWidget *parent = 0, int placeID = -1);
    ~PlaceDialog();

    int placeId() const;

protected:
    void changeEvent(QEvent *e) Q_DECL_OVERRIDE;

private slots:
    void save();
    void geocode();
    void geocodeReply(const QString & originalQuery, const QString & lat, const QString & lon, const QString & displayName, const QString & osmId);
    void geocodeFinished(const QString & originalQuery);
    void placeTriggered(QAction * action);
    void updateMapCenter();

private:
    void populateControls();
    Ui::PlaceDialog *ui;
    int m_placeID;
    OsmGeoCoder * m_gc;
    QMenu * m_placeMenu;
    MapWidget * m_mapWidget;
};

#endif // PLACEDIALOG_H
