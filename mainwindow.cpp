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

#include <QtSql>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QStandardPaths>
#include <QDebug>
#include <QSignalMapper>
#include <QDesktopServices>
#include <QSqlRecord>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "person.h"

#include "dialogs/persondialog.h"
#include "dialogs/placedialog.h"
#include "dialogs/relationdialog.h"

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_viewGroup(new QActionGroup(this)),
    m_proxyModel(new QSortFilterProxyModel(this)),
    m_peopleModel(0),
    m_placesModel(0),
    m_relationsModel(0)
{
    ui->setupUi(this);
    setupActions();

    // setup storage location
    m_storageLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    //qDebug() << "Storage location:" << m_storageLocation;
    QDir storageDir(m_storageLocation);
    if (!storageDir.exists()) {
        storageDir.mkpath(m_storageLocation);
    }

    m_proxyModel->setSortLocaleAware(true);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1); // all columns

    //connections
    connect(ui->tableView, &QTableView::clicked, this, &MainWindow::tableViewClicked);
    connect(ui->tableView, &QTableView::doubleClicked, this, &MainWindow::tableViewDoubleClicked);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &MainWindow::tableViewContextMenuRequested);
    connect(ui->leSearch, &QLineEdit::textChanged, this, &MainWindow::slotSearchStringChanged);

    // settings
    loadSettings();
}

MainWindow::~MainWindow()
{
    closeDatabase();
    saveSettings();
    delete ui;
}

void MainWindow::saveSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("state", saveState());
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void MainWindow::loadSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    restoreState(settings.value("state").toByteArray());
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();
}

void MainWindow::slotNew()
{
    const QString dbName = QInputDialog::getText(this, tr("New Database"), tr("Database name:"));
    if (!dbName.isEmpty()) {
        closeDatabase();
        qDebug() << "new DB name:" << dbName;
        openDatabase(m_storageLocation + "/" + dbName + ".pdb", true);
    }
}

void MainWindow::slotOpen()
{
    const QString filename = QFileDialog::getOpenFileName(this, tr("Open Database"), m_storageLocation, tr("Pedikree databases (*.pdb)"),
                                                          0
                                                      #ifdef Q_OS_LINUX
                                                          , QFileDialog::DontUseNativeDialog // FIXME native kfiledialog crashing
                                                      #endif
                                                          );
    if (!filename.isEmpty()) {
        openDatabase(filename);
    }
}

void MainWindow::slotAbout()
{
    QMessageBox::about(this, tr("About"),
                       "<p>" + tr("A simple genealogy application written using Qt") + "</p>" +
                       "<p>" + QString::fromUtf8("(c) Lukáš Tinkl <a href=\"mailto:lukas@kde.org\">lukas@kde.org</a>") + "</p>" +
                       "<p>" + tr("Uses free icons from %1").arg("<a href=\"http://icons8.com/license\">http://icons8.com/license</a></p>") +
                       tr("Version: %1").arg(qApp->applicationVersion()));
}

void MainWindow::slotSwitchView(QAction *action)
{
    if (!QSqlDatabase::database().isOpen())
        return;

    ui->leSearch->clear();

    if (action == ui->actionViewPeople) {
        if (!m_peopleModel) {
            m_peopleModel = new PeopleModel(this);
        }
        m_peopleModel->exec();
        m_proxyModel->setSourceModel(m_peopleModel);
        ui->tableView->setModel(m_proxyModel);
        ui->tableView->sortByColumn(1, Qt::AscendingOrder);
    } else if (action == ui->actionViewPlaces) {
        if (!m_placesModel) {
            m_placesModel = new PlacesModel(this);
        }
        m_placesModel->exec();
        m_proxyModel->setSourceModel(m_placesModel);
        ui->tableView->setModel(m_proxyModel);
        ui->tableView->sortByColumn(0, Qt::AscendingOrder);
    } else if (action == ui->actionViewRelations) {
        if (!m_relationsModel) {
            m_relationsModel = new RelationsModel(this);
        }
        m_relationsModel->exec();
        m_proxyModel->setSourceModel(m_relationsModel);
        ui->tableView->setModel(m_proxyModel);
        ui->tableView->sortByColumn(0, Qt::AscendingOrder);
    }
    ui->tableView->resizeColumnsToContents();
}

void MainWindow::slotSearchStringChanged(const QString &pattern)
{
    m_proxyModel->setFilterFixedString(pattern);
}

void MainWindow::tableViewClicked(const QModelIndex &index)
{
    const int row = m_proxyModel->mapToSource(index).row();

    if (m_viewGroup->checkedAction() == ui->actionViewPlaces) {
        PlacesModel * model = qobject_cast<PlacesModel *>(m_proxyModel->sourceModel());
        if (model) {
            qDebug() << "Clicked place at row " << row << "with DB ID:" << model->idAtRow(row);
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewPeople) {
        PeopleModel * model = qobject_cast<PeopleModel *>(m_proxyModel->sourceModel());
        if (model) {
            Person person(model->idAtRow(row));
            qDebug() << "Person" << model->idAtRow(row)<< "has father:" << person.fatherID() << ", mother:" << person.motherID();
            qDebug() << "\thas spouses:" << person.spouseIDs();
        }
    }
}

void MainWindow::tableViewDoubleClicked(const QModelIndex &index)
{
    if (!m_proxyModel->sourceModel())
        return;

    const int row = m_proxyModel->mapToSource(index).row();

    if (m_viewGroup->checkedAction() == ui->actionViewPeople) {
        PeopleModel * model = qobject_cast<PeopleModel *>(m_proxyModel->sourceModel());
        if (model) {
            qDebug() << "Double clicked person at row " << row << "with DB ID:" << model->idAtRow(row);
            slotEditPerson(model->idAtRow(row));
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewPlaces) {
        PlacesModel * model = qobject_cast<PlacesModel *>(m_proxyModel->sourceModel());
        if (model) {
            qDebug() << "Double clicked place at row " << row << "with DB ID:" << model->idAtRow(row);
            slotEditPlace(model->idAtRow(row));
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewRelations) {
        RelationsModel * model = qobject_cast<RelationsModel *>(m_proxyModel->sourceModel());
        if (model) {
            qDebug() << "Double clicked relation at row " << row << "with DB ID:" << model->idAtRow(row);
            slotEditRelation(model->idAtRow(row));
        }
    }
}

void MainWindow::tableViewContextMenuRequested(const QPoint &pos)
{
    if (m_filename.isEmpty())
        return;

    const bool validIndex = ui->tableView->indexAt(pos).isValid();

    ui->actionAddItem->setEnabled(!m_filename.isEmpty());
    ui->actionEditItem->setEnabled(validIndex);
    ui->actionDeleteItem->setEnabled(validIndex);

    QMenu menu;
    menu.addAction(ui->actionAddItem);
    if (validIndex) {
        menu.addAction(ui->actionEditItem);
        menu.addAction(ui->actionDeleteItem);
    }

    if (validIndex && m_viewGroup->checkedAction() == ui->actionViewPeople) { // add Person specific entries
        const int row = m_proxyModel->mapToSource(ui->tableView->indexAt(pos)).row();

        PeopleModel * model = qobject_cast<PeopleModel *>(m_proxyModel->sourceModel());
        if (model) {
            menu.addSeparator();

            Person person(model->idAtRow(row));

            QMenu * menuPerson = new QMenu(tr("Relatives"), this);

            // parents
            const int fatherID = person.fatherID();
            const int motherID = person.motherID();

            if (fatherID != -1 || motherID != -1) {
                QMenu * menuParents = menuPerson->addMenu(tr("Parents"));
                QAction * fatherAct = new QAction(tr("Father: %1").arg(Person::personFullName(fatherID)), this);
                fatherAct->setData(fatherID);
                menuParents->addAction(fatherAct);
                QAction * motherAct = new QAction(tr("Mother: %1").arg(Person::personFullName(motherID)), this);
                motherAct->setData(motherID);
                menuParents->addAction(motherAct);
            }

            // spouses
            const QList<int> spouseIDs = person.spouseIDs();
            if (!spouseIDs.isEmpty()) {
                QMenu * menuSpouses = menuPerson->addMenu(tr("Spouses (%1)").arg(spouseIDs.count()));
                foreach (int spouseID, spouseIDs) {
                    QAction * spouseAct = new QAction(tr("Spouse: %1").arg(Person::personFullName(spouseID)), this);
                    spouseAct->setData(spouseID);
                    menuSpouses->addAction(spouseAct);
                }
            }

            // siblings
            const QList<int> siblingIDs = person.siblingIDs();
            if (!siblingIDs.isEmpty()) {
                QMenu * menuSiblings = menuPerson->addMenu(tr("Siblings (%1)").arg(siblingIDs.count()));
                foreach (int siblingID, siblingIDs) {
                    QAction * siblingAct = new QAction(tr("Sibling: %1").arg(Person::personFullName(siblingID)), this);
                    siblingAct->setData(siblingID);
                    menuSiblings->addAction(siblingAct);
                }
            }

            // children
            const QList<int> childrenIDs = person.childrenIDs();
            if (!childrenIDs.isEmpty()) {
                QMenu * menuChildren = menuPerson->addMenu(tr("Children (%1)").arg(childrenIDs.count()));
                foreach (int childID, childrenIDs) {
                    QAction * childAct = new QAction(tr("Child: %1").arg(Person::personFullName(childID)), this);
                    childAct->setData(childID);
                    menuChildren->addAction(childAct);
                }
            }

            if (!menuPerson->isEmpty()) {
                menu.addMenu(menuPerson);
            }
            connect(menuPerson, SIGNAL(triggered(QAction*)), this, SLOT(slotEditPerson(QAction*)));
        }
    } else if (validIndex && m_viewGroup->checkedAction() == ui->actionViewPlaces) { // add Person specific entries
        const int row = m_proxyModel->mapToSource(ui->tableView->indexAt(pos)).row();

        PlacesModel * model = qobject_cast<PlacesModel *>(m_proxyModel->sourceModel());
        if (model) {
            menu.addSeparator();
            QAction * tmp = menu.addAction(tr("View on OpenStreetMaps..."), this, SLOT(slotViewPlace()));
            tmp->setData(model->idAtRow(row));
        }
    }

#if 0 // FIXME
    if (validIndex && m_viewGroup->checkedAction() == ui->actionViewPeople) { // add Person specific entries
        menu.addSeparator();
        QMenu * menuPerson = menu.addMenu(tr("Add new"));
        menuPerson->addSeparator()->setText(tr("Parents"));
        menuPerson->addAction(tr("Add &father..."), this, SLOT(slotAddFather()));
        menuPerson->addAction(tr("Add &mother..."), this, SLOT(slotAddMother()));
        menuPerson->addSeparator()->setText(tr("Children"));
        menuPerson->addAction(tr("Add &son..."), this, SLOT(slotAddSon()));
        menuPerson->addAction(tr("Add &daughter..."), this, SLOT(slotAddDaugther()));
        menuPerson->addSeparator()->setText(tr("Siblings"));
        menuPerson->addAction(tr("Add &brother..."), this, SLOT(slotAddBrother()));
        menuPerson->addAction(tr("Add s&ister..."), this, SLOT(slotAddSister()));
    }
#endif

    menu.exec(QCursor::pos());
}

void MainWindow::updateEditMenuActions()
{
    const bool validIndex = ui->tableView->currentIndex().isValid();

    ui->actionAddItem->setEnabled(!m_filename.isEmpty());
    ui->actionEditItem->setEnabled(validIndex);
    ui->actionDeleteItem->setEnabled(validIndex);
}

void MainWindow::slotAddItemActionTriggered()
{
    if (m_viewGroup->checkedAction() == ui->actionViewPeople) {
        slotAddPerson();
    } else if (m_viewGroup->checkedAction() == ui->actionViewPlaces) {
        slotAddPlace();
    } else if (m_viewGroup->checkedAction() == ui->actionViewRelations) {
        slotAddRelation();
    }
}

void MainWindow::slotEditItemActionTriggered()
{
    const QModelIndex currentIndex = m_proxyModel->mapToSource(ui->tableView->currentIndex());
    if (!currentIndex.isValid())
        return;

    const int row = currentIndex.row();

    if (m_viewGroup->checkedAction() == ui->actionViewPeople) {
        PeopleModel * model = qobject_cast<PeopleModel *>(m_proxyModel->sourceModel());
        if (model) {
            slotEditPerson(model->idAtRow(row));
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewPlaces) {
        PlacesModel * model = qobject_cast<PlacesModel *>(m_proxyModel->sourceModel());
        if (model) {
            slotEditPlace(model->idAtRow(row));
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewRelations) {
        RelationsModel * model = qobject_cast<RelationsModel *>(m_proxyModel->sourceModel());
        if (model) {
            slotEditRelation(model->idAtRow(row));
        }
    }
}

void MainWindow::slotDeleteItemActionTriggered()
{
    const QModelIndex currentIndex = m_proxyModel->mapToSource(ui->tableView->currentIndex());
    if (!currentIndex.isValid())
        return;

    const int row = currentIndex.row();

    if (m_viewGroup->checkedAction() == ui->actionViewPeople) {
        PeopleModel * model = qobject_cast<PeopleModel *>(m_proxyModel->sourceModel());
        if (model) {
            slotDeletePerson(model->idAtRow(row));
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewPlaces) {
        PlacesModel * model = qobject_cast<PlacesModel *>(m_proxyModel->sourceModel());
        if (model) {
            slotDeletePlace(model->idAtRow(row));
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewRelations) {
        RelationsModel * model = qobject_cast<RelationsModel *>(m_proxyModel->sourceModel());
        if (model) {
            slotDeleteRelation(model->idAtRow(row));
        }
    }
}

void MainWindow::slotAddPerson()
{
    PersonDialog * dlg = new PersonDialog(this);
    dlg->setWindowTitle(tr("Add Person"));
    if (dlg->exec() == QDialog::Accepted) {
        m_peopleModel->exec();
    }
}

void MainWindow::slotEditPerson(int personID)
{
    PersonDialog * dlg = new PersonDialog(this, personID);
    dlg->setWindowTitle(tr("Edit Person"));
    dlg->exec();
    m_peopleModel->exec();
}

void MainWindow::slotEditPerson(QAction *action)
{
    slotEditPerson(action->data().toInt());
}

void MainWindow::slotDeletePerson(int personID)
{
    qDebug() << Q_FUNC_INFO << "Deleting person" << personID;
    if (QMessageBox::question(this, tr("Delete Person"), tr("Do you really want to delete the person '%1'?").arg(Person::personFullName(personID)),
                              (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery query(QStringLiteral("DELETE FROM People WHERE id=%1").arg(personID));
        if (query.exec()) {
            m_peopleModel->exec();
        } else {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        }
    }
}

void MainWindow::slotAddPlace()
{
    PlaceDialog * dlg = new PlaceDialog(this);
    dlg->setWindowTitle(tr("Add Place"));
    if (dlg->exec() == QDialog::Accepted) {
        m_placesModel->exec();
    }
}

void MainWindow::slotEditPlace(int placeID)
{
    PlaceDialog * dlg = new PlaceDialog(this, placeID);
    dlg->setWindowTitle(tr("Edit Place"));
    if (dlg->exec() == QDialog::Accepted) {
        m_placesModel->exec();
    }
}

void MainWindow::slotDeletePlace(int placeID)
{
    qDebug() << Q_FUNC_INFO << "Deleting place" << placeID;
    if (QMessageBox::question(this, tr("Delete Place"), tr("Do you really want to delete the place with ID %1?").arg(placeID),
                              (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery query(QStringLiteral("DELETE FROM Places WHERE id=%1").arg(placeID));
        if (query.exec()) {
            m_placesModel->exec();
        } else {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        }
    }
}

void MainWindow::slotViewPlace()
{
    const int placeID = qobject_cast<QAction *>(sender())->data().toInt();
    qDebug() << "View place" << placeID;

    QSqlQuery query(QStringLiteral("SELECT lat, lon FROM Places WHERE id=%1").arg(placeID));
    if (query.exec() && query.first()) {
        QDesktopServices::openUrl(QUrl::fromUserInput(QStringLiteral("http://www.openstreetmap.org/#map=14/%1/%2")
                                                      .arg(query.record().value("lat").toDouble())
                                                      .arg(query.record().value("lon").toDouble())));
    }
}

void MainWindow::slotAddRelation()
{
    RelationDialog * dlg = new RelationDialog(this);
    dlg->setWindowTitle(tr("Add Relation"));
    if (dlg->exec() == QDialog::Accepted) {
        m_relationsModel->exec();
    }
}

void MainWindow::slotEditRelation(int relationID)
{
    RelationDialog * dlg = new RelationDialog(this, relationID);
    dlg->setWindowTitle(tr("Edit Relation"));
    if (dlg->exec() == QDialog::Accepted) {
        m_relationsModel->exec();
    }
}

void MainWindow::slotDeleteRelation(int relationID)
{
    qDebug() << Q_FUNC_INFO << "Deleting relation" << relationID;
    if (QMessageBox::question(this, tr("Delete Relation"), tr("Do you really want to delete the relation with ID %1?").arg(relationID),
                              (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery query(QStringLiteral("DELETE FROM Relations WHERE id=%1").arg(relationID));
        if (query.exec()) {
            m_relationsModel->exec();
        } else {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        }
    }
}

void MainWindow::slotAddFather()
{
    const QModelIndex currentIndex = m_proxyModel->mapToSource(ui->tableView->currentIndex());
    const int personID = m_peopleModel->idAtRow(currentIndex.row());

    qDebug() << "Adding father of " << personID;

    QSqlQuery query(QStringLiteral("SELECT sex, first_name, maiden_name, surname, birth_place, birth_date FROM People WHERE id=%1").arg(personID));

    PersonDialog * dlg = new PersonDialog(this);
    dlg->setMale();
    if (query.exec() && query.first()) {
        dlg->setWindowTitle(tr("Add Father of %1 %2").arg(query.value("first_name").toString(), query.value("surname").toString()));
        if (query.value("sex").toString() == "F")
            dlg->setSurname(query.value("maiden_name").toString());
        else
            dlg->setSurname(query.value("surname").toString());
    } else {
        dlg->setWindowTitle(tr("Add Father"));
    }

    if (dlg->exec() == QDialog::Accepted) {
        QSqlQuery query2;
        query2.prepare("INSERT INTO Relations (type, person1_id, person2_id, place, date) "
                       "VALUES ('BiologicalParent', :person1_id, :person2_id, :place, :date)");
        query2.bindValue(":person1_id", dlg->personID());
        query2.bindValue(":person2_id", personID);
        query2.bindValue(":place", query.value("birth_place"));
        query2.bindValue(":date", query.value("birth_date"));

        qDebug() << "Inserting father" << query2.executedQuery();

        if (!query2.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query2.lastError().text();
            return;
        }
    }

    m_peopleModel->exec();
}

void MainWindow::slotAddMother()
{
    const QModelIndex currentIndex = m_proxyModel->mapToSource(ui->tableView->currentIndex());
    const int personID = m_peopleModel->idAtRow(currentIndex.row());

    qDebug() << "Adding mother of " << personID;

    QSqlQuery query(QStringLiteral("SELECT sex, first_name, maiden_name, surname, birth_place, birth_date FROM People WHERE id=%1").arg(personID));

    PersonDialog * dlg = new PersonDialog(this);
    dlg->setFemale();
    if (query.exec() && query.first()) {
        dlg->setWindowTitle(tr("Add Mother of %1 %2").arg(query.value("first_name").toString(), query.value("surname").toString()));
        if (query.value("sex").toString() == "F")
            dlg->setSurname(query.value("maiden_name").toString());
        else
            dlg->setSurname(query.value("surname").toString());
    } else {
        dlg->setWindowTitle(tr("Add Mother"));
    }

    if (dlg->exec() == QDialog::Accepted) {
        QSqlQuery query2;
        query2.prepare("INSERT INTO Relations (type, person1_id, person2_id, place, date) "
                       "VALUES ('BiologicalParent', :person1_id, :person2_id, :place, :date)");
        query2.bindValue(":person1_id", dlg->personID());
        query2.bindValue(":person2_id", personID);
        query2.bindValue(":place", query.value("birth_place"));
        query2.bindValue(":date", query.value("birth_date"));

        qDebug() << "Inserting mother" << query2.executedQuery();

        if (!query2.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query2.lastError().text();
            return;
        }
    }

    m_peopleModel->exec();
}

void MainWindow::slotAddSon()
{
    const QModelIndex currentIndex = m_proxyModel->mapToSource(ui->tableView->currentIndex());
    const int personID = m_peopleModel->idAtRow(currentIndex.row());

    qDebug() << "Adding son of " << personID;

    QSqlQuery query(QStringLiteral("SELECT first_name, surname FROM People WHERE id=%1").arg(personID));

    PersonDialog * dlg = new PersonDialog(this);
    dlg->setMale();
    if (query.exec() && query.first()) {
        dlg->setWindowTitle(tr("Add Son of %1 %2").arg(query.value("first_name").toString(), query.value("surname").toString()));
        dlg->setSurname(query.value("surname").toString());
    } else {
        dlg->setWindowTitle(tr("Add Son"));
    }

    if (dlg->exec() == QDialog::Accepted) {
        query.finish();
        query.exec(QStringLiteral("SELECT birth_place, birth_date FROM People WHERE id=%1").arg(dlg->personID()));

        QSqlQuery query2;
        query2.prepare("INSERT INTO Relations (type, person1_id, person2_id, place, date) "
                       "VALUES ('BiologicalParent', :person1_id, :person2_id, :place, :date)");
        query2.bindValue(":person1_id", personID);
        query2.bindValue(":person2_id", dlg->personID());
        if (query.exec() && query.first()) {
            query2.bindValue(":place", query.value("birth_place"));
            query2.bindValue(":date", query.value("birth_date"));
        }

        qDebug() << "Inserting son" << query2.executedQuery();

        if (!query2.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query2.lastError().text();
            return;
        }
    }

    m_peopleModel->exec();
}

void MainWindow::slotAddDaugther()
{
    const QModelIndex currentIndex = m_proxyModel->mapToSource(ui->tableView->currentIndex());
    const int personID = m_peopleModel->idAtRow(currentIndex.row());

    qDebug() << "Adding daughter of " << personID;

    QSqlQuery query(QStringLiteral("SELECT first_name, surname FROM People WHERE id=%1").arg(personID));

    PersonDialog * dlg = new PersonDialog(this);
    dlg->setFemale();
    if (query.exec() && query.first()) {
        dlg->setWindowTitle(tr("Add Daughter of %1 %2").arg(query.value("first_name").toString(), query.value("surname").toString()));
        dlg->setSurname(query.value("surname").toString());
    } else {
        dlg->setWindowTitle(tr("Add Daughter"));
    }

    if (dlg->exec() == QDialog::Accepted) {
        query.finish();
        query.exec(QStringLiteral("SELECT birth_place, birth_date FROM People WHERE id=%1").arg(dlg->personID()));

        QSqlQuery query2;
        query2.prepare("INSERT INTO Relations (type, person1_id, person2_id, place, date) "
                       "VALUES ('BiologicalParent', :person1_id, :person2_id, :place, :date)");
        query2.bindValue(":person1_id", personID);
        query2.bindValue(":person2_id", dlg->personID());
        if (query.exec() && query.first()) {
            query2.bindValue(":place", query.value("birth_place"));
            query2.bindValue(":date", query.value("birth_date"));
        }

        qDebug() << "Inserting daughter" << query2.executedQuery();

        if (!query2.exec()) {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query2.lastError().text();
            return;
        }
    }

    m_peopleModel->exec();
}

void MainWindow::slotAddBrother()
{
    // TODO
}

void MainWindow::slotAddSister()
{
    // TODO
}

void MainWindow::setupActions()
{
    // views
    m_viewGroup->setExclusive(true);
    m_viewGroup->addAction(ui->actionViewPeople);
    m_viewGroup->addAction(ui->actionViewRelations);
    m_viewGroup->addAction(ui->actionViewPlaces);
    connect(m_viewGroup, &QActionGroup::triggered, this, &MainWindow::slotSwitchView);

    // table view context menu
    ui->tableView->addAction(ui->actionAddItem);
    ui->tableView->addAction(ui->actionEditItem);
    ui->tableView->addAction(ui->actionDeleteItem);

    connect(ui->menuEdit, &QMenu::aboutToShow, this, &MainWindow::updateEditMenuActions);

    connect(ui->actionAddItem, &QAction::triggered, this, &MainWindow::slotAddItemActionTriggered);
    connect(ui->actionEditItem, &QAction::triggered, this, &MainWindow::slotEditItemActionTriggered);
    connect(ui->actionDeleteItem, &QAction::triggered, this, &MainWindow::slotDeleteItemActionTriggered);

    // shortcuts
    ui->actionNew->setShortcut(QKeySequence(QKeySequence::New));
    ui->actionOpen->setShortcut(QKeySequence(QKeySequence::Open));
    ui->actionSaveAs->setShortcut(QKeySequence(QKeySequence::SaveAs));
    ui->actionQuit->setShortcut(QKeySequence(QKeySequence::Quit));

    // connections
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::slotNew);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::slotOpen);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::slotAbout);
    connect(ui->actionQuit, &QAction::triggered, qApp, &QApplication::quit);
}

void MainWindow::openDatabase(const QString &dbFilePath, bool create)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbFilePath);

    if (!db.driver()->hasFeature(QSqlDriver::QuerySize)) {
        qWarning() << "DB driver" << db.driverName() << "doesn't support query size!";
    }

    if (db.open()) {
        if (create) {
            initDatabase();
        }
        m_filename = db.databaseName();
        setWindowFilePath(m_filename);
        ui->actionViewPeople->trigger(); // default view
    } else {
        qWarning() << "Error opening the DB" << db.lastError().text();
    }
}

void MainWindow::closeDatabase()
{
    ui->tableView->setModel(0);

    delete m_peopleModel;
    m_peopleModel = 0;
    delete m_relationsModel;
    m_relationsModel = 0;
    delete m_placesModel;
    m_placesModel = 0;

    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        db.close();
    }
    QSqlDatabase::removeDatabase(db.connectionName());
}

void MainWindow::initDatabase()
{
    const QStringList tables = QStringList() << "Places" << "People" << "Events" << "Relations";
    QSqlQuery query;

    foreach (const QString & table, tables) {
        QFile file(QString::fromLatin1(":/schema/%1.sql").arg(table));
        if (!file.open(QFile::ReadOnly))
            return;

        query.prepare(file.readAll());
        //qDebug() << query.executedQuery();
        if (!query.exec()) {
            qWarning() << "Initting DB table" << table << "failed with:" << query.lastError().text();
        }
        query.finish();
        file.close();
    }
}
