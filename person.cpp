#include <QtXml>

#include "person.h"

PersonModel::PersonModel(const QString &fileName, QObject *parent):
    QAbstractListModel(parent),
    m_fileName(fileName)
{
    if (!m_fileName.isEmpty())
        parse();
    else {
        qWarning() << Q_FUNC_INFO << "No filename specified";
    }
}

PersonModel::~PersonModel()
{
}

QString PersonModel::fileName() const
{
    return m_fileName;
}

void PersonModel::setFileName(const QString &fileName)
{
    m_fileName = fileName;
    if (!m_fileName.isEmpty()) {
        parse();
    }
}

int PersonModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_persons.count();
}

int PersonModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QHash<int, QByteArray> PersonModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "ID";
    roles[FullNameRole] = "FullName";
    roles[BirthDateRole] = "BirthDate";
    roles[DeathDateRole] = "DeathDate";
    return roles;
}

QVariant PersonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Full name");
        } else if (section == 1) {
            return tr("Date of birth");
        } else if (section == 2) {
            return tr("Date of death");
        }
    }

    return QAbstractListModel::headerData(section, orientation, role);
}

QVariant PersonModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const int column = index.column();

    qDebug() << "Data for row" << row << column;

    if (row >=0 && row < m_persons.count()) {
        const QDomElement personElem = m_persons.item(row).toElement();

        if (personElem.isNull())
            return QVariant();

        switch (role) {
        case FullNameRole: {
            qDebug() << "Wanting full name";
            const QDomElement namesElem = personElem.firstChildElement("names").toElement();
            if (!namesElem.isNull()) {
                return QString("%1 %2 %3 %4").arg(namesElem.attribute("first"), namesElem.attribute("middle"), namesElem.attribute("surname"),
                                                  namesElem.attribute("suffix")).simplified();
            }
            return QVariant();
        }
        case BirthDateRole: {
            qDebug() << "Wanting birth date";
            const QDomElement birthElem = personElem.firstChildElement("birth").toElement();
            if (!birthElem.isNull() && birthElem.hasAttribute("date")) {
                return QDate::fromString(birthElem.attribute("date"), Qt::ISODate);
            }
            return QVariant();
        }
        case DeathDateRole: {
            qDebug() << "Wanting death date";
            const QDomElement deathElem = personElem.firstChildElement("death").toElement();
            if (!deathElem.isNull() && deathElem.hasAttribute("date")) {
                qDebug() << "Death date elem null";
                return QDate::fromString(deathElem.attribute("date"), Qt::ISODate);
            }
            return QVariant();
        }
        case Qt::DisplayRole: {
            if (column == 0)
                return data(index, FullNameRole);
            else if (column == 1)
                return data(index, BirthDateRole);
            else if (column == 2)
                return data(index, DeathDateRole);
        }
        }
    }

    return QVariant();
}

bool PersonModel::parse()
{
    QDomDocument doc("pedikree");
    QFile file(m_fileName);
    qDebug() << "opening" << m_fileName;
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QString msg;
    int line;
    int column;
    if (!doc.setContent(&file, &msg, &line, &column)) {
        qWarning() << msg << "(" << line << "," << column << ")";
        file.close();
        return false;
    }
    file.close();

    const QDomElement peopleElem = doc.documentElement().firstChildElement("people");
    m_persons = peopleElem.elementsByTagName("person");
    qDebug() << "Pedigree contains" << m_persons.count() << "people";

    return true;
}
