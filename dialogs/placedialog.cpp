#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

#include "placedialog.h"
#include "ui_placedialog.h"

PlaceDialog::PlaceDialog(QWidget *parent, int placeID) :
    QDialog(parent),
    ui(new Ui::PlaceDialog),
    m_placeID(placeID)
{
    ui->setupUi(this);

    if (m_placeID != -1) { // editting, fill the controls
        populateControls();
    }

    qDebug() << "Editting place with ID:" << m_placeID;
}

PlaceDialog::~PlaceDialog()
{
    delete ui;
}

void PlaceDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void PlaceDialog::populateControls()
{
    QSqlQuery query;
    query.prepare(QString("SELECT id, name, lat, lon, comment "
                  "FROM Places "
                  "WHERE id=%1").arg(m_placeID));

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        return;
    }
    else
        query.first();

    ui->leName->setText(query.value("name").toString());
    ui->sbLat->setValue(query.value("lat").toDouble());
    ui->sbLon->setValue(query.value("lon").toDouble());
    ui->teComment->setPlainText(query.value("comment").toString());
}
