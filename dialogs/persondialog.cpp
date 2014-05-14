#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>

#include "persondialog.h"
#include "ui_persondialog.h"

PersonDialog::PersonDialog(QWidget *parent, int personID):
    QDialog(parent),
    ui(new Ui::PersonDialog),
    m_personID(personID)
{
    ui->setupUi(this);

    connect(ui->buttonGroupGender, SIGNAL(buttonToggled(QAbstractButton*, bool)), this, SLOT(genderClicked(QAbstractButton*)));
    connect(ui->buttonGroupAlive, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(deadAliveClicked(QAbstractButton*)));

    if (m_personID == -1) { // new person, preset some values
        ui->cbAlive->setChecked(true);
    } else { // editting, fill the controls
        populateControls();
    }

    qDebug() << "Editting person with ID:" << m_personID;
}

PersonDialog::~PersonDialog()
{
    delete ui;
}

void PersonDialog::changeEvent(QEvent *e)
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

void PersonDialog::genderClicked(QAbstractButton *button)
{
    Q_UNUSED(button)
    const bool hide = ui->cbMale->isChecked();
    ui->leBirthSurname->setHidden(hide);
    ui->labelBirthSurname->setHidden(hide);
}

void PersonDialog::deadAliveClicked(QAbstractButton *button)
{
    Q_UNUSED(button)
    const bool hide = ui->cbAlive->isChecked();
    ui->leDeathDate->setHidden(hide);
    ui->leDeathPlace->setHidden(hide);
    ui->leDeathReason->setHidden(hide);
    ui->leBurialPlace->setHidden(hide);
    ui->labelDeathDate->setHidden(hide);
    ui->labelDeathPlace->setHidden(hide);
    ui->labelDeathReason->setHidden(hide);
    ui->labelBurial->setHidden(hide);
}

void PersonDialog::populateControls()
{
    QSqlQuery query;
    query.prepare(QString("SELECT first_name, middle_name, surname, prefix, suffix, sex, "
                          "birth_date, birth_place, "
                          "death_date, death_place "
                          "FROM People "
                          "WHERE id=%1").arg(m_personID));
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        return;
    }
    else
        query.first();

    // sex
    const QString sex = query.value("sex").toString();
    if (sex == "M")
        ui->cbMale->setChecked(true);
    else if (sex == "F")
        ui->cbFemale->setChecked(true);

    // names
    ui->leGivenNames->setText(QString("%1 %2").arg(query.value("first_name").toString(), query.value("middle_name").toString()).simplified());
    ui->leSurname->setText(query.value("surname").toString());
    ui->lePrefix->setText(query.value("prefix").toString());
    ui->leSuffix->setText(query.value("suffix").toString());

    // birth
    ui->leBirthDate->setText(query.value("birth_date").toDate().toString(Qt::DefaultLocaleLongDate));
    ui->leBirthPlace->setText(query.value("birth_place").toString());

    // death
    const QDate deathDate = query.value("death_date").toDate();
    if (deathDate.isValid()) {
        ui->cbDeceased->setChecked(true);
        ui->leDeathDate->setText(deathDate.toString(Qt::DefaultLocaleLongDate));
    } else {
        ui->cbAlive->setChecked(true);
    }
}
