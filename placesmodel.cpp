#include <QSqlRecord>
#include <QSqlField>

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
}

int PlacesModel::idAtRow(int row) const
{
    return record(row).value("id").toInt();
}

int PlacesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 3;
}

QVariant PlacesModel::data(const QModelIndex &item, int role) const
{
    if (role == Qt::DisplayRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 0) {
            return rec.field("name").value();
        } else if (column == 1) {
            return rec.field("lat").value();
        } else if (column == 2) {
            return rec.field("lon").value();
        }
    }

    return QSqlQueryModel::data(item, role);
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
