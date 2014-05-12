#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include <QSqlQueryModel>

class PlacesModel: public QSqlQueryModel
{
    Q_OBJECT
public:
    PlacesModel(QObject *parent = 0);
    virtual ~PlacesModel();

    void exec();

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
};

#endif // PLACESMODEL_H