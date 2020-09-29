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

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class OsmGeoCoder: public QObject
{
    Q_OBJECT
public:
    explicit OsmGeoCoder(QObject *parent = nullptr);
    ~OsmGeoCoder() = default;

public slots:
    void geocode(const QString & query);

signals:
    void geocodeReply(const QString & originalQuery, const QString & lat, const QString & lon, const QString & displayName,
                      const QString & osmId);
    void geocodeFinished(const QString & originalQuery);

private slots:
    void slotReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager{nullptr};
};
