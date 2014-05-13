#ifndef PEOPLEMODEL_H
#define PEOPLEMODEL_H

#include <QSqlQueryModel>

class PeopleModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    PeopleModel(QObject *parent = 0);
    virtual ~PeopleModel();

    void exec();

    int idAtRow(int row) const;

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
};


#endif // PEOPLEMODEL_H
