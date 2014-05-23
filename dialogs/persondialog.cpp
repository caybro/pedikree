/*
 * Copyright (C) 2014 Lukáš Tinkl <lukas@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>

#include "persondialog.h"
#include "ui_persondialog.h"
#include "pddatedialog.h"
#include "placedialog.h"

PersonDialog::PersonDialog(QWidget *parent, int personID):
    QDialog(parent),
    ui(new Ui::PersonDialog),
    m_personID(personID)
{
    ui->setupUi(this);

    // places
    m_placesModel = new PlacesLookupModel(this);
    ui->leBirthPlace->setModel(m_placesModel);
    ui->leBirthPlace->setModelColumn(1);
    ui->leDeathPlace->setModel(m_placesModel);
    ui->leDeathPlace->setModelColumn(1);
    ui->leBurialPlace->setModel(m_placesModel);
    ui->leBurialPlace->setModelColumn(1);

    // nationality
    ui->nationality->addItem(QString());
    ui->country->addItem(QString());
    for (int i = 1; i <= QLocale::LastCountry; i++) {
        const QString countryName = QLocale::system().countryToString((QLocale::Country)i);
        ui->nationality->addItem(countryName, i);
        ui->country->addItem(countryName, i);
    }

    connect(ui->buttonGroupGender, SIGNAL(buttonToggled(QAbstractButton*, bool)), this, SLOT(genderClicked(QAbstractButton*)));
    connect(ui->buttonGroupAlive, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(deadAliveClicked(QAbstractButton*)));

    if (m_personID == -1) { // new person, preset some values
        ui->cbAlive->setChecked(true);
        ui->leBirthPlace->setEditText(QString());
        ui->leDeathPlace->setEditText(QString());
        ui->leBurialPlace->setEditText(QString());
        ui->leMaidenName->setVisible(false);
        ui->labelMaidenName->setVisible(false);
    } else { // editting, fill the controls
        populateControls();
    }

    connect(ui->btnAddBirthPlace, &QPushButton::clicked, this, &PersonDialog::slotAddPlace);
    connect(ui->btnAddDeathPlace, &QPushButton::clicked, this, &PersonDialog::slotAddPlace);
    connect(ui->btnAddBurialPlace, &QPushButton::clicked, this, &PersonDialog::slotAddPlace);

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

void PersonDialog::setMale()
{
    ui->cbMale->setChecked(true);
}

void PersonDialog::setFemale()
{
    ui->cbFemale->setChecked(true);
}

void PersonDialog::setSurname(const QString &surname)
{
    ui->leSurname->setText(surname);
}

int PersonDialog::personID() const
{
    return m_personID;
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
    const bool show = ui->cbFemale->isChecked();
    ui->leMaidenName->setVisible(show);
    ui->labelMaidenName->setVisible(show);
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
    ui->btnAddDeathPlace->setHidden(hide);
    ui->btnAddBurialPlace->setHidden(hide);
}

void PersonDialog::save()
{
    QSqlQuery query;

    if (m_personID == -1) { // add person
        query.prepare("INSERT INTO People (first_name, surname, maiden_name, prefix, suffix, sex, "
                      "birth_date, birth_place, "
                      "death_date, death_place, death_reason, burial_place, "
                      "contact_email, contact_phone, contact_web, "
                      "contact_address, contact_city, contact_state, contact_zip, contact_country, "
                      "nationality, religion, height, weight, "
                      "color, eye_color, hair_color, occupation, alive) "
                      "VALUES (:first_name, :surname, :maiden_name, :prefix, :suffix, :sex, "
                      ":birth_date, :birth_place, "
                      ":death_date, :death_place, :death_reason, :burial_place, "
                      ":contact_email, :contact_phone, :contact_web, "
                      ":nationality, :religion, :height, :weight, "
                      ":color, :eye_color, :hair_color, :occupation, :alive)");
    } else { // update person
        query.prepare("UPDATE People SET "
                      "first_name=:first_name, surname=:surname, maiden_name=:maiden_name, prefix=:prefix, suffix=:suffix, sex=:sex, "
                      "birth_date=:birth_date, birth_place=:birth_place, "
                      "death_date=:death_date, death_place=:death_place, death_reason=:death_reason, burial_place=:burial_place, "
                      "contact_email=:contact_email, contact_phone=:contact_phone, contact_web=:contact_web, "
                      "contact_address=:contact_address, contact_city=:contact_city, contact_state=:contact_state, "
                      "contact_zip=:contact_zip, contact_country=:contact_country, "
                      "nationality=:nationality, religion=:religion, height=:height, weight=:weight, "
                      "color=:color, eye_color=:eye_color, hair_color=:hair_color, occupation=:occupation, "
                      "alive=:alive "
                      "WHERE id=:id");

        query.bindValue(":id", m_personID);
    }

    query.bindValue(":first_name", ui->leGivenNames->text());
    query.bindValue(":surname", ui->leSurname->text());
    query.bindValue(":maiden_name", ui->leMaidenName->text());
    query.bindValue(":prefix", ui->lePrefix->text());
    query.bindValue(":suffix", ui->leSuffix->text());
    query.bindValue(":sex", ui->cbMale->isChecked() ? "M" : (ui->cbFemale->isChecked() ? "F" : ""));
    query.bindValue(":birth_date", ui->leBirthDate->text());
    query.bindValue(":birth_place", ui->leBirthPlace->currentText());
    // TODO places id
    const bool alive = ui->cbAlive->isChecked();
    query.bindValue(":alive", alive ? 1 : 0);
    if (!alive) {
        query.bindValue(":death_date", ui->leDeathDate->text());
        query.bindValue(":death_place", ui->leDeathPlace->currentText());
        query.bindValue(":death_reason", ui->leDeathReason->text());
        query.bindValue(":burial_place", ui->leBurialPlace->currentText());
    }
    query.bindValue(":contact_email", ui->email->text());
    query.bindValue(":contact_phone", ui->phone->text());
    query.bindValue(":contact_web", ui->web->text());
    query.bindValue(":contact_address", ui->address->text());
    query.bindValue(":contact_city", ui->city->text());
    query.bindValue(":contact_state", ui->state->text());
    query.bindValue(":contact_zip", ui->zip->text());
    query.bindValue(":contact_country", ui->country->currentText());
    query.bindValue(":nationality", ui->nationality->currentText());
    query.bindValue(":religion", ui->religion->currentText());
    query.bindValue(":height", ui->height->value());
    query.bindValue(":weight", ui->weight->value());
    query.bindValue(":color", ui->color->currentText());
    query.bindValue(":eye_color", ui->eyeColor->currentText());
    query.bindValue(":hair_color", ui->hairColor->currentText());
    query.bindValue(":occupation", ui->occupation->text());

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        return;
    }

    if (query.lastInsertId().isValid())
        m_personID = query.lastInsertId().toInt();
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

void PersonDialog::slotAddPlace()
{
    PlaceDialog * dlg = new PlaceDialog(this);
    dlg->setWindowTitle(tr("Add Place"));
    if (dlg->exec() == QDialog::Accepted) {
        m_placesModel->reload();

        QPushButton * btn = qobject_cast<QPushButton *>(sender());
        if (btn == ui->btnAddBirthPlace) {
            ui->leBirthPlace->setCurrentIndex(ui->leBirthPlace->findData(dlg->placeId()));
        } else if (btn == ui->btnAddDeathPlace) {
            ui->leDeathPlace->setCurrentIndex(ui->leDeathPlace->findData(dlg->placeId()));
        } else if (btn == ui->btnAddBurialPlace) {
            ui->leBurialPlace->setCurrentIndex(ui->leBurialPlace->findData(dlg->placeId()));
        }
    }
}

void PersonDialog::populateControls()
{
    QSqlQuery query;
    query.prepare(QString("SELECT first_name, surname, maiden_name, prefix, suffix, sex, "
                          "birth_date, birth_place, "
                          "death_date, death_place, death_reason, burial_place, "
                          "contact_email, contact_phone, contact_web, "
                          "contact_address, contact_city, contact_state, contact_zip, contact_country, "
                          "nationality, religion, "
                          "height, weight, "
                          "color, eye_color, hair_color, occupation, alive "
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
    else
        ui->cbUnknown->setChecked(true);

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
    if (query.value("alive").toBool()) {
        ui->cbAlive->setChecked(true);
    } else {
        ui->cbDeceased->setChecked(true);
    }
    ui->leDeathDate->setText(query.value("death_date").toString());
    ui->leDeathPlace->setEditText(query.value("death_place").toString());
    ui->leDeathReason->setText(query.value("death_reason").toString());
    ui->leBurialPlace->setEditText(query.value("burial_place").toString());

    // contact
    ui->email->setText(query.value("contact_email").toString());
    ui->phone->setText(query.value("contact_phone").toString());
    ui->web->setText(query.value("contact_web").toString());

    ui->address->setText(query.value("contact_address").toString());
    ui->city->setText(query.value("contact_city").toString());
    ui->state->setText(query.value("contact_state").toString());
    ui->zip->setText(query.value("contact_zip").toString());
    ui->country->setEditText(query.value("contact_country").toString());

    // personal
    ui->nationality->setEditText(query.value("nationality").toString());
    ui->religion->setEditText(query.value("religion").toString());

    ui->height->setValue(query.value("height").toInt());
    ui->weight->setValue(query.value("weight").toInt());

    ui->color->setEditText(query.value("color").toString());
    ui->eyeColor->setEditText(query.value("eye_color").toString());
    ui->hairColor->setEditText(query.value("hair_color").toString());

    ui->occupation->setText(query.value("occupation").toString());
}
