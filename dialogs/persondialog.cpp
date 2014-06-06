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
#include <QSqlError>
#include <QSqlRecord>
#include <QDate>
#include <QHeaderView>
#include <QBuffer>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

#include "persondialog.h"
#include "ui_persondialog.h"
#include "pddatedialog.h"
#include "placedialog.h"
#include "../relationsmodel.h"
#include "person.h"

PersonDialog::PersonDialog(QWidget *parent, int personID):
    QDialog(parent),
    ui(new Ui::PersonDialog),
    m_personID(personID),
    m_familyInitted(false),
    m_childrenModel(0),
    m_siblingsModel(0)
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
    connect(ui->photo, &QPushButton::clicked, this, &PersonDialog::slotSelectImage);

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

    // family tab
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &PersonDialog::tabChanged);
    connect(ui->btnPrevCouple, &QToolButton::clicked, this, &PersonDialog::prevPartner);
    connect(ui->btnNextCouple, &QToolButton::clicked, this, &PersonDialog::nextPartner);
    connect(ui->btnAddChild, &QPushButton::clicked, this, &PersonDialog::slotAddChild);
    connect(ui->btnRemoveChild, &QPushButton::clicked, this, &PersonDialog::slotRemoveChild);
    connect(ui->btnAddSibling, &QPushButton::clicked, this, &PersonDialog::slotAddSibling);
    connect(ui->btnRemoveSibling, &QPushButton::clicked, this, &PersonDialog::slotRemoveSibling);

    // button box
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
                      "color, eye_color, hair_color, occupation, alive, photo) "
                      "VALUES (:first_name, :surname, :maiden_name, :prefix, :suffix, :sex, "
                      ":birth_date, :birth_place, "
                      ":death_date, :death_place, :death_reason, :burial_place, "
                      ":contact_email, :contact_phone, :contact_web, "
                      ":contact_address, :contact_city, :contact_state, :contact_zip, :contact_country, "
                      ":nationality, :religion, :height, :weight, "
                      ":color, :eye_color, :hair_color, :occupation, :alive, :photo)");
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
                      "alive=:alive, photo=:photo "
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
    if (!m_photoFilename.isEmpty())
        query.bindValue(":photo", savePhoto());

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

void PersonDialog::slotSelectImage()
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Select Image"), QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(),
                                                          "*.png *.jpg *.jpeg"); // FIXME better use a MIME type
    if (!fileName.isEmpty()) {
        m_photoFilename = fileName;
        ui->photo->setIcon(QIcon(m_photoFilename));
    }
}

void PersonDialog::tabChanged(int index)
{
    if (index == 1 && !m_familyInitted) {
        populateFamilyTab();
        connect(ui->children->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(slotCurrentChildChanged()));
        connect(m_childrenModel, SIGNAL(modelReset()), SLOT(slotCurrentChildChanged()));
        connect(ui->siblings->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(slotCurrentSiblingChanged()));
        connect(m_siblingsModel, SIGNAL(modelReset()), SLOT(slotCurrentSiblingChanged()));
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
                          "color, eye_color, hair_color, occupation, alive, photo "
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

    const QByteArray photo = query.value("photo").toByteArray();
    if (!photo.isEmpty()) {
        ui->photo->setIcon(QPixmap::fromImage(QImage::fromData(photo)));
    }
}

void PersonDialog::populateFamilyTab()
{
    // this person
    m_thisPersonQuery = QSqlQuery(QString("SELECT surname, printf(\"%s %s\", first_name, surname) as name FROM People WHERE id=%1").arg(m_personID));
    m_thisPersonQuery.exec();
    m_thisPersonQuery.first();

    // partners and children
    m_partnerQuery = QSqlQuery(QString("SELECT r.id as relation_id, r.type, r.date, "
                                       "p.id as person_id, printf(\"%s %s\", p.first_name, p.surname) as name "
                                       "FROM Relations r, People p "
                                       "WHERE (r.person1_id=%1 OR r.person2_id=%1) "
                                       "      AND (r.person1_id=p.id OR r.person2_id=p.id) "
                                       "      AND r.type IN ('Annulment', 'CommonLawMarriage', 'CivilUnion', 'DomesticPartnership', 'Divorce', 'DivorceFiling', "
                                       "                    'Engagement', 'Marriage', 'MarriageBanns', 'MarriageContract', 'MarriageLicense', 'MarriageNotice', 'Separation') "
                                       "      AND p.id!=%1").arg(m_personID));

    if (m_partnerQuery.exec() && m_partnerQuery.first()) {
        updatePartnersLabel();
        ui->btnNextCouple->setEnabled(true); // BUG in Qt, even after first() is successfully called, at() returns -1!!!
        //qDebug() << "First partner at:" << m_partnerQuery.at();

        const QString childrenQuery = QString("SELECT DISTINCT p.id as person_id, printf(\"%s %s\", p.first_name, p.surname) as name, p.birth_date, p.birth_place "
                                              "FROM People p, Relations r "
                                              "WHERE ((r.person1_id=:person1 AND r.person2_id=:person2) OR (r.person1_id=:person2 AND r.person2_id=:person1)) AND person_id=r.child_id "
                                              "AND r.type IN ('AdoptiveParent', 'BiologicalParent', 'FosterParent', 'GuardianParent', 'StepParent', 'SociologicalParent', 'SurrogateParent')");
        m_childrenQuery.prepare(childrenQuery);
        m_childrenQuery.bindValue(":person1", m_personID);
        m_childrenQuery.bindValue(":person2", m_partnerQuery.value("person_id"));
        if (m_childrenQuery.exec() && m_childrenQuery.first()) {
            //qDebug() << "Executed children query for" << m_personID << "and" << m_partnerQuery.value("person_id").toInt();
        } else {
            qWarning() << Q_FUNC_INFO << "Children query failed with" << m_childrenQuery.lastError().text();
        }
        m_childrenModel = new QSqlQueryModel(this);
        m_childrenModel->setQuery(m_childrenQuery);
        m_childrenModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        m_childrenModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        m_childrenModel->setHeaderData(2, Qt::Horizontal, tr("Birth Date"));
        m_childrenModel->setHeaderData(3, Qt::Horizontal, tr("Birth Place"));
        ui->children->setModel(m_childrenModel);
        ui->children->hideColumn(0);
        ui->children->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    } else {
        qWarning() << Q_FUNC_INFO << "Spouse query failed with" << m_partnerQuery.lastError().text();
        ui->couples->setText(tr("No spouse found for this person."));
        ui->btnAddChild->setEnabled(false);
        ui->btnRemoveChild->setEnabled(false);
    }

    // parents and siblings
    m_parentsQuery = QSqlQuery(QString("SELECT r.person1_id AS parent1_id, r.person2_id AS parent2_id, "
                                       "(SELECT printf(\"%s %s %s\", first_name, surname, suffix) FROM People WHERE id=r.person1_id) AS parent1_name, "
                                       "(SELECT printf(\"%s %s %s\", first_name, surname, suffix) FROM People WHERE id=r.person2_id) AS parent2_name "
                                       "FROM Relations r "
                                       "WHERE r.child_id=%1 "
                                       "AND r.type IN ('AdoptiveParent', 'BiologicalParent', 'FosterParent', 'GuardianParent', 'StepParent', 'SociologicalParent', 'SurrogateParent')")
                               .arg(m_personID));
    if (m_parentsQuery.exec() && m_parentsQuery.first()) {
        const QString siblingsQuery = QString("SELECT DISTINCT p.id as person_id, printf(\"%s %s\", p.first_name, p.surname) as name, p.birth_date, p.birth_place "
                                              "FROM People p, Relations r "
                                              "WHERE p.id=r.child_id AND r.child_id!=%3 AND ((r.person1_id=%1 AND r.person2_id=%2) OR (r.person1_id=%2 AND r.person2_id=%1))")
                                      .arg(m_parentsQuery.value("parent1_id").toInt()).arg(m_parentsQuery.value("parent2_id").toInt()).arg(m_personID);
        m_siblingsQuery.prepare(siblingsQuery);
        ui->parents->setText(QString("%1 + %2").arg(m_parentsQuery.value("parent1_name").toString(), m_parentsQuery.value("parent2_name").toString()));

        if (m_siblingsQuery.exec() && m_siblingsQuery.first()) {
            qDebug() << "Executed siblings query for" << m_personID;
        } else {
            qWarning() << Q_FUNC_INFO << "Siblings query failed with" << m_siblingsQuery.lastError().text();
        }
        m_siblingsModel = new QSqlQueryModel(this);
        m_siblingsModel->setQuery(m_siblingsQuery);
        m_siblingsModel->setHeaderData(0, Qt::Horizontal, tr("ID"));
        m_siblingsModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        m_siblingsModel->setHeaderData(2, Qt::Horizontal, tr("Birth Date"));
        m_siblingsModel->setHeaderData(3, Qt::Horizontal, tr("Birth Place"));
        ui->siblings->setModel(m_siblingsModel);
        ui->siblings->hideColumn(0);
        ui->siblings->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    } else {
        qWarning() << Q_FUNC_INFO << "Parents query failed with" << m_parentsQuery.lastError().text();
        ui->parents->setText(tr("No parents found for this person."));
        ui->btnAddSibling->setEnabled(false);
        ui->btnRemoveSibling->setEnabled(false);
    }

    m_familyInitted = true;
}

void PersonDialog::updatePartnersLabel()
{
    const QString type = qApp->translate("Relations", Relations::relations().value(m_partnerQuery.value("type").toString()).toUtf8());
    const QString format = tr(", since %1");
    QString dateStr = m_partnerQuery.value("date").toString();
    const QDate date = QDate::fromString(dateStr, Qt::ISODate);
    if (date.isValid()) {
        dateStr = date.toString(Qt::DefaultLocaleShortDate);
    }
    if (!dateStr.isEmpty()) {
        dateStr = format.arg(dateStr);
    }
    ui->couples->setText(QString("%1 + %2 (%3%4)").arg(m_thisPersonQuery.value("name").toString(),
                                                       m_partnerQuery.value("name").toString(),
                                                       type, dateStr));
}

void PersonDialog::updatePartnersButtons()
{
    ui->btnNextCouple->setDisabled(m_partnerQuery.at() == QSql::AfterLastRow);
    ui->btnPrevCouple->setDisabled(m_partnerQuery.at() == QSql::BeforeFirstRow || m_partnerQuery.at() == 0);
}

void PersonDialog::fetchChildren()
{
    //qDebug() << "Fetching children of" << m_personID << "and" << m_partnerQuery.value("person_id").toInt();

    m_childrenQuery.bindValue(":person2", m_partnerQuery.value("person_id"));
    if (m_childrenQuery.exec()) {
        ui->children->reset();
        m_childrenModel->setQuery(m_childrenQuery);
        m_childrenQuery.first();
        ui->children->update();
    } else {
        qWarning() << Q_FUNC_INFO << "Fetch children query failed with" << m_childrenQuery.lastError().text();
    }
}

void PersonDialog::fetchSiblings()
{
    if (m_siblingsQuery.exec()) {
        ui->siblings->reset();
        m_siblingsModel->setQuery(m_siblingsQuery);
        m_siblingsQuery.first();
        ui->siblings->update();
    } else {
        qWarning() << Q_FUNC_INFO << "Fetch siblings query failed with" << m_siblingsQuery.lastError().text();
    }
}

QByteArray PersonDialog::savePhoto() const
{
    QImage image(m_photoFilename);
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return byteArray;
}

void PersonDialog::nextPartner()
{
    if (m_partnerQuery.seek(1, true)) {
        ui->btnAddChild->setEnabled(true);
        updatePartnersLabel();
        fetchChildren();
    } else {
        if (!m_partnerQuery.last())
            ui->btnAddChild->setEnabled(false);
    }

    updatePartnersButtons();
}

void PersonDialog::prevPartner()
{
    if (m_partnerQuery.seek(-1, true)) {
        ui->btnAddChild->setEnabled(true);
        updatePartnersLabel();
        fetchChildren();
    } else {
        if (!m_partnerQuery.first())
            ui->btnAddChild->setEnabled(false);
    }

    updatePartnersButtons();
}

void PersonDialog::slotAddChild()
{
    qDebug() << "Adding child of" << m_personID << "and" << m_partnerQuery.value("person_id").toInt();

    PersonDialog * dlg = new PersonDialog(this);
    dlg->setSurname(m_thisPersonQuery.value("surname").toString());
    dlg->setWindowTitle(tr("Add Child of %1 and %2").arg(m_thisPersonQuery.value("name").toString(),
                                                         m_partnerQuery.value("name").toString()));
    if (dlg->exec() == QDialog::Accepted) {
        QSqlQuery query;
        query.exec(QString("SELECT birth_place, birth_date FROM People WHERE id=%1").arg(dlg->personID()));

        QSqlQuery query2;
        query2.prepare("INSERT INTO Relations (type, person1_id, person2_id, child_id, place, date) "
                       "VALUES ('BiologicalParent', :person1_id, :person2_id, :child_id, :place, :date)");
        query2.bindValue(":person1_id", m_personID);
        query2.bindValue(":person2_id", m_partnerQuery.value("person_id").toInt());
        query2.bindValue(":child_id", dlg->personID());
        if (query.first()) {
            query2.bindValue(":place", query.value("birth_place"));
            query2.bindValue(":date", query.value("birth_date"));
        }

        qDebug() << "Inserting child" << query2.executedQuery();

        if (!query2.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query2.lastError().text();
            return;
        }
    }

    fetchChildren();
}

void PersonDialog::slotRemoveChild()
{
    const int childID = m_childrenModel->record(ui->children->currentIndex().row()).value("person_id").toInt();
    qDebug() << "Removing child" << childID;

    if (QMessageBox::question(this, tr("Delete Child"), tr("Do you really want to delete the child '%1'?").arg(Person::personFullName(childID)),
                              (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery query(QString("DELETE FROM People WHERE id=%1").arg(childID));
        if (query.exec()) {
            fetchChildren();
        } else {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        }
    }
}

void PersonDialog::slotAddSibling()
{
    qDebug() << "Adding sibling of parents" << m_parentsQuery.value("parent1_id").toInt() << "and" << m_parentsQuery.value("parent2_id").toInt();

    PersonDialog * dlg = new PersonDialog(this);
    dlg->setSurname(m_thisPersonQuery.value("surname").toString());
    dlg->setWindowTitle(tr("Add Sibling of %1").arg(m_thisPersonQuery.value("name").toString()));
    if (dlg->exec() == QDialog::Accepted) {
        QSqlQuery query;
        query.exec(QString("SELECT birth_place, birth_date FROM People WHERE id=%1").arg(dlg->personID()));

        QSqlQuery query2;
        query2.prepare("INSERT INTO Relations (type, person1_id, person2_id, child_id, place, date) "
                       "VALUES ('BiologicalParent', :person1_id, :person2_id, :child_id, :place, :date)");
        query2.bindValue(":person1_id", m_parentsQuery.value("parent1_id").toInt());
        query2.bindValue(":person2_id", m_parentsQuery.value("parent2_id").toInt());
        query2.bindValue(":child_id", dlg->personID());
        if (query.first()) {
            query2.bindValue(":place", query.value("birth_place"));
            query2.bindValue(":date", query.value("birth_date"));
        }

        qDebug() << "Inserting sibling" << query2.executedQuery();

        if (!query2.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query2.lastError().text();
            return;
        }
    }

    fetchSiblings();
}

void PersonDialog::slotRemoveSibling()
{
    const int siblingID = m_siblingsModel->record(ui->siblings->currentIndex().row()).value("person_id").toInt();
    qDebug() << "Removing sibling" << siblingID;

    if (QMessageBox::question(this, tr("Delete Sibling"), tr("Do you really want to delete the sibling '%1'?").arg(Person::personFullName(siblingID)),
                              (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery query(QString("DELETE FROM People WHERE id=%1").arg(siblingID));
        if (query.exec()) {
            fetchSiblings();
        } else {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        }
    }
}

void PersonDialog::slotCurrentChildChanged()
{
    ui->btnRemoveChild->setEnabled(ui->children->currentIndex().isValid());
}

void PersonDialog::slotCurrentSiblingChanged()
{
    ui->btnRemoveSibling->setEnabled(ui->siblings->currentIndex().isValid());
}
