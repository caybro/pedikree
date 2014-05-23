#include <QDebug>
#include <QNetworkRequest>
#include <QDomDocument>
#include <QDomElement>

#include "osmgeocoder.h"

OsmGeoCoder::OsmGeoCoder(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &OsmGeoCoder::slotReplyFinished);
}

OsmGeoCoder::~OsmGeoCoder()
{
}

void OsmGeoCoder::geocode(const QString &query)
{
    QNetworkRequest request(QUrl::fromUserInput(
                                QString("http://nominatim.openstreetmap.org/search?q=%1&format=xml&addressdetails=1&accept-language=%2,en")
                                .arg(query)
                                .arg(QLocale::system().bcp47Name()))
                            );
    m_manager->get(request);
}

void OsmGeoCoder::slotReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << Q_FUNC_INFO << "Geocode query failed with error:" << reply->error() << ", " << reply->errorString();
        return;
    }

    QDomDocument dom;
    dom.setContent(reply);
    const QString origQuery = dom.documentElement().attribute("querystring");
    QDomNodeList places = dom.elementsByTagName("place");
    qDebug() << "Reply contains" << places.count() << "unfiltered entries";

    for (int i = 0; i < places.count(); i++) {
        QDomElement place = places.item(i).toElement();
        if (!place.isNull()) {
            const QString classAttr = place.attribute("class");
            if (classAttr == "place" || classAttr == "boundary") {
                //qDebug() << "Class" << classAttr;
                const QString lat = place.attribute("lat");
                const QString lon = place.attribute("lon");
                const QString osmId = QString("%1:%2").arg(place.attribute("osm_type"), place.attribute("osm_id"));
                QStringList displayName;

                // try suburb and locality and then city, town, village, hamlet, in that order
                QDomElement suburbElem = place.firstChildElement("suburb");
                if (suburbElem.isNull()) {
                    suburbElem = place.firstChildElement("locality");
                }
                if (!suburbElem.isNull()) {
                    displayName.append(suburbElem.text());
                }

                QDomElement locationElem = place.firstChildElement("city");
                if (locationElem.isNull())
                    locationElem = place.firstChildElement("town");
                if (locationElem.isNull())
                    locationElem = place.firstChildElement("village");
                if (locationElem.isNull())
                    locationElem = place.firstChildElement("hamlet");
                if (locationElem.isNull()) { // still empty, bail out
                    qWarning() << "Place" << place.text() << "has no suitable location description";
                }
                if (!locationElem.isNull()) {
                    displayName.append(locationElem.text());
                }

                QDomElement countyElem = place.firstChildElement("county");
                if (!countyElem.isNull()) {
                    displayName.append(countyElem.text());
                }

                QDomElement countryElem = place.firstChildElement("country");
                if (!countryElem.isNull()) {
                    displayName.append(countryElem.text());
                }

                emit geocodeReply(origQuery, lat, lon, displayName.join(", "), osmId);
            }
        }
    }

    emit geocodeFinished(origQuery);
    reply->deleteLater();
}
