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
    setQuery("SELECT id, sex, first_name, surname, birth_date, birth_place, occupation "
             "FROM People "
             "ORDER BY surname");
}

int PeopleModel::idAtRow(int row) const
{
    return record(row).value("id").toInt();
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
        if (column == 0) {
            const QString val = rec.field("sex").value().toString();
            QString symbol;
            if (val == "M")
                symbol = "♂";
            else if (val == "F")
                symbol = "♀";
            return QString("%1 %2 %3").arg(symbol, rec.field("first_name").value().toString(), rec.field("surname").value().toString());
        } else if (column == 1) {
            return rec.field("birth_date").value().toDate();
        } else if (column == 2) {
            return rec.field("birth_place").value();
        } else if (column == 3) {
            return rec.field("occupation").value();
        }
    } else if (role == Qt::ToolTipRole) {
        const QSqlRecord rec = record(item.row());
        const int column = item.column();
        if (column == 1) {
            return rec.field("birth_date").value().toDate().toString(Qt::DefaultLocaleLongDate);
        }
    }

    return QSqlQueryModel::data(item, role);
}

QVariant PeopleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Name");
        else if (section == 1)
            return tr("Birth Date");
        else if (section == 2)
            return tr("Birth Place");
        else if (section == 3)
            return tr("Occupation");
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}
