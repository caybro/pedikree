#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>

#include "persondialog.h"
#include "ui_persondialog.h"
#include "pddatedialog.h"

PersonDialog::PersonDialog(QWidget *parent, int personID):
    QDialog(parent),
    ui(new Ui::PersonDialog),
    m_personID(personID)
{
    ui->setupUi(this);

    // places
    m_placesModel = new QSqlQueryModel(this);
    m_placesModel->setQuery("SELECT id, name FROM Places");
    ui->leBirthPlace->setModel(m_placesModel);
    ui->leBirthPlace->setModelColumn(1);
    ui->leDeathPlace->setModel(m_placesModel);
    ui->leDeathPlace->setModelColumn(1);
    ui->leBurialPlace->setModel(m_placesModel);
    ui->leBurialPlace->setModelColumn(1);

    connect(ui->buttonGroupGender, SIGNAL(buttonToggled(QAbstractButton*, bool)), this, SLOT(genderClicked(QAbstractButton*)));
    connect(ui->buttonGroupAlive, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(deadAliveClicked(QAbstractButton*)));

    if (m_personID == -1) { // new person, preset some values
        ui->cbAlive->setChecked(true);
        ui->leBirthPlace->setEditText(QString());
        ui->leDeathPlace->setEditText(QString());
        ui->leBurialPlace->setEditText(QString());
    } else { // editting, fill the controls
        populateControls();
    }

    QAction * tmp = ui->leBirthDate->addAction(QIcon::fromTheme("view-calendar-day"), QLineEdit::TrailingPosition);
    tmp->setToolTip(tr("Enter date"));
    connect(tmp, &QAction::triggered, this, &PersonDialog::popupBirthDateCalendar);

    QAction * tmp2 = ui->leDeathDate->addAction(QIcon::fromTheme("view-calendar-day"), QLineEdit::TrailingPosition);
    tmp2->setToolTip(tr("Enter date"));
    connect(tmp2, &QAction::triggered, this, &PersonDialog::popupDeathDateCalendar);

    connect(this, &PersonDialog::accepted, this, &PersonDialog::save);

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
    ui->leMaidenName->setHidden(hide);
    ui->labelMaidenName->setHidden(hide);
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

void PersonDialog::save()
{
    QSqlQuery query;

    if (m_personID == -1) { // add person
        query.prepare("INSERT INTO People (first_name, surname, maiden_name, prefix, suffix, sex, "
                      "birth_date, birth_place, "
                      "death_date, death_place, death_reason, burial_place) "
                      "VALUES (:first_name, :surname, :maiden_name, :prefix, :suffix, :sex, "
                      ":birth_date, :birth_place, "
                      ":death_date, :death_place, :death_reason, :burial_place)");
        query.bindValue(":first_name", ui->leGivenNames->text());
        query.bindValue(":surname", ui->leSurname->text());
        query.bindValue(":maiden_name", ui->leMaidenName->text());
        query.bindValue(":prefix", ui->lePrefix->text());
        query.bindValue(":suffix", ui->leSuffix->text());
        query.bindValue(":sex", ui->cbMale->isChecked() ? "M" : (ui->cbFemale->isChecked() ? "F" : ""));
        query.bindValue(":birth_date", ui->leBirthDate->text());
        query.bindValue(":birth_place", ui->leBirthPlace->currentText());
        // TODO places id
        if (ui->cbDeceased->isChecked()) {
            query.bindValue(":death_date", ui->leDeathDate->text());
            query.bindValue(":death_place", ui->leDeathPlace->currentText());
            query.bindValue(":death_reason", ui->leDeathReason->text());
            query.bindValue(":burial_place", ui->leBurialPlace->currentText());
        }

        qDebug() << "Adding person with query" << query.lastQuery();

        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        }
    } else { // update person
        query.prepare("UPDATE People SET "
                      "first_name=:first_name, surname=:surname, maiden_name=:maiden_name, prefix=:prefix, suffix=:suffix, sex=:sex, "
                      "birth_date=:birth_date, birth_place=:birth_place, "
                      "death_date=:death_date, death_place=:death_place, death_reason=:death_reason, burial_place=:burial_place "
                      "WHERE id=:id");

        query.bindValue(":first_name", ui->leGivenNames->text());
        query.bindValue(":surname", ui->leSurname->text());
        query.bindValue(":maiden_name", ui->leMaidenName->text());
        query.bindValue(":prefix", ui->lePrefix->text());
        query.bindValue(":suffix", ui->leSuffix->text());
        query.bindValue(":sex", ui->cbMale->isChecked() ? "M" : (ui->cbFemale->isChecked() ? "F" : ""));
        query.bindValue(":birth_date", ui->leBirthDate->text());
        query.bindValue(":birth_place", ui->leBirthPlace->currentText());
        // TODO places id
        if (ui->cbDeceased->isChecked()) {
            query.bindValue(":death_date", ui->leDeathDate->text());
            query.bindValue(":death_place", ui->leDeathPlace->currentText());
            query.bindValue(":death_reason", ui->leDeathReason->text());
            query.bindValue(":burial_place", ui->leBurialPlace->currentText());
        }
        query.bindValue(":id", m_personID);

        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        }
    }
}

void PersonDialog::popupBirthDateCalendar()
{
    PdDateDialog * dlg = new PdDateDialog(ui->leBirthDate->text(), this);
    if (dlg->exec() == QDialog::Accepted) {
        ui->leBirthDate->setText(dlg->date());
    }
    delete dlg;
}

void PersonDialog::popupDeathDateCalendar()
{
    PdDateDialog * dlg = new PdDateDialog(ui->leDeathDate->text(), this);
    if (dlg->exec() == QDialog::Accepted) {
        ui->leDeathDate->setText(dlg->date());
    }
    delete dlg;
}

void PersonDialog::populateControls()
{
    QSqlQuery query;
    query.prepare(QString("SELECT first_name, surname, maiden_name, prefix, suffix, sex, "
                          "birth_date, birth_place, "
                          "death_date, death_place, death_reason, burial_place "
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
    ui->leGivenNames->setText(query.value("first_name").toString());
    ui->leSurname->setText(query.value("surname").toString());
    ui->lePrefix->setText(query.value("prefix").toString());
    ui->leSuffix->setText(query.value("suffix").toString());
    ui->leMaidenName->setText(query.value("maiden_name").toString());

    // birth
    ui->leBirthDate->setText(query.value("birth_date").toString());
    ui->leBirthPlace->setEditText(query.value("birth_place").toString());

    // death
    const QString deathDate = query.value("death_date").toString();
    if (!deathDate.isEmpty()) {
        ui->cbDeceased->setChecked(true);
    } else {
        ui->cbAlive->setChecked(true);
    }
    ui->leDeathDate->setText(deathDate);
    ui->leDeathPlace->setEditText(query.value("death_place").toString());
    ui->leDeathReason->setText(query.value("death_reason").toString());
    ui->leBurialPlace->setEditText(query.value("burial_place").toString());
}
