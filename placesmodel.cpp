#include "placesmodel.h"

PlacesModel::PlacesModel(QObject *parent):
    QSqlQueryModel(parent)
{
    exec();
}

PlacesModel::~PlacesModel()
{
}

void PlacesModel::exec()
{
    setQuery("SELECT id, name, lat, lon FROM Places ORDER BY name");
    removeColumn(0); // hide the ID column
}

QVariant PlacesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Name");
        else if (section == 1)
            return tr("Latitude");
        else if (section == 2)
            return ("Longitude");
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}
