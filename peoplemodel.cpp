#include <QDebug>
#include <QDate>
#include <QSqlRecord>
#include <QSqlField>

#include "peoplemodel.h"

PeopleModel::PeopleModel(QObject *parent) :
    QSqlQueryModel(parent)
{
    exec();
}

PeopleModel::~PeopleModel()
{
}

void PeopleModel::exec()
{
    setQuery("SELECT People.id, People.sex, People.first_name, People.surname, People.birth_date, Places.name AS place_name "
             "FROM People, Places "
             "WHERE People.birth_place_id=Places.id");
}

int PeopleModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant PeopleModel::data(const QModelIndex &item, int role) const
{
    if (role == Qt::DisplayRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 0)
            return QString("%1 %2").arg(rec.field("first_name").value().toString(), rec.field("surname").value().toString());
        else if (column == 1) {
            const QString val = rec.field("sex").value().toString();
            if (val == "M")
                return tr("Male");
            else if (val == "F")
                return tr("Female");
            else
                return tr("Unknown", "Gender");
        }
        else if (column == 2)
            return rec.field("birth_date").value().toDate();
        else if (column == 3)
            return rec.field("place_name").value();
    }

    return QSqlQueryModel::data(item, role);
}

QVariant PeopleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Name");
        else if (section == 1)
            return tr("Gender");
        else if (section == 2)
            return tr("Birth Date");
        else if (section == 3)
            return tr("Birth Place");
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}
