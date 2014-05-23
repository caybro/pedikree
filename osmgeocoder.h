#ifndef OSMGEOCODER_H
#define OSMGEOCODER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class OsmGeoCoder: public QObject
{
    Q_OBJECT
public:
    explicit OsmGeoCoder(QObject *parent = 0);
    virtual ~OsmGeoCoder();

public slots:
    void geocode(const QString & query);

signals:
    void geocodeReply(const QString & originalQuery, const QString & lat, const QString & lon, const QString & displayName, const QString & osmId);
    void geocodeFinished(const QString & originalQuery);

private slots:
    void slotReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
};

#endif // OSMGEOCODER_H
