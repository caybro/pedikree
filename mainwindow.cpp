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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "placesmodel.h"

#include "dialogs/persondialog.h"
#include "dialogs/placedialog.h"

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_viewGroup(new QActionGroup(this)),
    m_peopleModel(0),
    m_placesModel(0)
{
    ui->setupUi(this);
    setupActions();

    // setup storage location
    m_storageLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    qDebug() << "Storage location:" << m_storageLocation;
    QDir storageDir(m_storageLocation);
    if (!storageDir.exists()) {
        storageDir.mkpath(m_storageLocation);
    }

    //connections
    connect(ui->tableView, &QTableView::doubleClicked, this, &MainWindow::tableViewDoubleClicked);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, &MainWindow::tableViewContextMenuRequested);
}

MainWindow::~MainWindow()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        db.close();
    }
    QSqlDatabase::removeDatabase(db.connectionName());

    delete ui;
}

void MainWindow::slotNew()
{
    const QString dbName = QInputDialog::getText(this, tr("New Pedikree Database"), tr("Database name:"));
    if (!dbName.isEmpty()) {
        qDebug() << "new DB name:" << dbName;
        openDatabase(m_storageLocation + "/" + dbName + ".pdb", true);
    }
}

void MainWindow::slotOpen()
{
    const QString filename = QFileDialog::getOpenFileName(this, tr("Open Pedikree Database"), m_storageLocation, tr("Pedikree databases (*.pdb)"),
                                                          0, QFileDialog::DontUseNativeDialog); // FIXME native kfiledialog crashing
    if (!filename.isEmpty()) {
        openDatabase(filename);
    }
}

void MainWindow::slotAbout()
{
    QMessageBox::about(this, tr("About Pedikree"),
                       tr("A simple genealogy application written using Qt") + "\n\n" +
                       QString::fromUtf8("(c) Lukáš Tinkl <lukas@kde.org>") + "\n\n" +
                       tr("Version: %1").arg(qApp->applicationVersion()));
}

void MainWindow::slotSwitchView(QAction *action)
{
    if (action == ui->actionViewPeople) {
        if (!m_peopleModel) {
            m_peopleModel = new PeopleModel(this);
        } else if (m_peopleModel) {
            m_peopleModel->exec();
        }
        ui->tableView->setModel(m_peopleModel);
    } else if (action == ui->actionViewPlaces) {
        if (!m_placesModel) {
            m_placesModel = new PlacesModel(this);
        } else if (m_placesModel) {
            m_placesModel->exec();
        }
        ui->tableView->setModel(m_placesModel);
    }
    ui->tableView->resizeColumnsToContents();
}

void MainWindow::tableViewDoubleClicked(const QModelIndex &index)
{
    if (!ui->tableView->model())
        return;

    const int row = index.row();

    if (m_viewGroup->checkedAction() == ui->actionViewPeople) {
        PeopleModel * model = qobject_cast<PeopleModel *>(ui->tableView->model());
        if (model) {
            qDebug() << "Double clicked person at row " << row << "with DB ID:" << model->idAtRow(row);
            slotEditPerson(model->idAtRow(row));
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewPlaces) {
        PlacesModel * model = qobject_cast<PlacesModel *>(ui->tableView->model());
        if (model) {
            qDebug() << "Double clicked place at row " << row << "with DB ID:" << model->idAtRow(row);
            slotEditPlace(model->idAtRow(row));
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
    }
}

void MainWindow::slotEditItemActionTriggered()
{
    const QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid())
        return;

    const int row = currentIndex.row();

    if (m_viewGroup->checkedAction() == ui->actionViewPeople) {
        PeopleModel * model = qobject_cast<PeopleModel *>(ui->tableView->model());
        if (model) {
            slotEditPerson(model->idAtRow(row));
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewPlaces) {
        PlacesModel * model = qobject_cast<PlacesModel *>(ui->tableView->model());
        if (model) {
            slotEditPlace(model->idAtRow(row));
        }
    }
}

void MainWindow::slotDeleteItemActionTriggered()
{
    const QModelIndex currentIndex = ui->tableView->currentIndex();
    if (!currentIndex.isValid())
        return;

    const int row = currentIndex.row();

    if (m_viewGroup->checkedAction() == ui->actionViewPeople) {
        PeopleModel * model = qobject_cast<PeopleModel *>(ui->tableView->model());
        if (model) {
            slotDeletePerson(model->idAtRow(row));
        }
    } else if (m_viewGroup->checkedAction() == ui->actionViewPlaces) {
        PlacesModel * model = qobject_cast<PlacesModel *>(ui->tableView->model());
        if (model) {
            slotDeletePlace(model->idAtRow(row));
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
    if (dlg->exec() == QDialog::Accepted) {
        m_peopleModel->exec();
    }
}

void MainWindow::slotDeletePerson(int personID)
{
    qDebug() << Q_FUNC_INFO << "Deleting person" << personID;
    if (QMessageBox::question(this, tr("Delete Place"), tr("Do you really want to delete the person with ID %1?").arg(personID)) == QMessageBox::Yes) {
        QSqlQuery query(QString("DELETE FROM People WHERE id=%1").arg(personID));
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
    if (QMessageBox::question(this, tr("Delete Place"), tr("Do you really want to delete the place with ID %1?").arg(placeID)) == QMessageBox::Yes) {
        QSqlQuery query(QString("DELETE FROM Places WHERE id=%1").arg(placeID));
        if (query.exec()) {
            m_placesModel->exec();
        } else {
            qWarning() << Q_FUNC_INFO << "Query failed with" << query.lastError().text();
        }
    }
}

void MainWindow::setupActions()
{
    // views
    m_viewGroup->setExclusive(true);
    m_viewGroup->addAction(ui->actionViewPeople);
    m_viewGroup->addAction(ui->actionViewEvents);
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
    if (db.open()) {
        if (create) {
            initDatabase();
        }
        m_filename = db.databaseName();
        setWindowFilePath(m_filename);
        ui->actionViewPeople->trigger();
    } else {
        qWarning() << "Error opening the DB" << db.lastError().text();
    }
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
        //qDebug() << query.lastQuery();
        if (!query.exec()) {
            qWarning() << "Initting DB table" << table << "failed with:" << query.lastError().text();
        }
        query.finish();
        file.close();
    }
}
