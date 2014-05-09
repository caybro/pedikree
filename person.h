#ifndef PERSON_H
#define PERSON_H

#include <QAbstractListModel>
#include <QDomNodeList>

class PersonModel: public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
public:
    enum ItemRole {
        IdRole = Qt::UserRole + 1,
        FullNameRole,
        BirthDateRole,
        DeathDateRole
    };

    PersonModel(const QString & fileName, QObject * parent = 0);
    virtual ~PersonModel();

    QString fileName() const;
    void setFileName(const QString & fileName);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;

signals:
    void fileNameChanged();

private:
    bool parse();
    QString m_fileName;
    QDomNodeList m_persons;
};

#endif // PERSON_H
