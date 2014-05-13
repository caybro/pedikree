#include "persondialog.h"
#include "ui_persondialog.h"

PersonDialog::PersonDialog(QWidget *parent, bool create) :
    QDialog(parent),
    ui(new Ui::PersonDialog),
    m_create(create)
{
    ui->setupUi(this);

    connect(ui->buttonGroupGender, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(genderClicked(QAbstractButton*)));
    connect(ui->buttonGroupAlive, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(deadAliveClicked(QAbstractButton*)));
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
