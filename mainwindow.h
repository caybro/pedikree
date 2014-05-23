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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>

#include "peoplemodel.h"
#include "placesmodel.h"
#include "relationsmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    // settings
    void saveSettings();
    void loadSettings();

    // menu actions
    void slotNew();
    void slotOpen();
    void slotAbout();

    // views
    void slotSwitchView(QAction * action);

    // table view handling
    void tableViewDoubleClicked(const QModelIndex &index);
    void tableViewContextMenuRequested(const QPoint & pos);
    void updateEditMenuActions();

    // general action handling
    void slotAddItemActionTriggered();
    void slotEditItemActionTriggered();
    void slotDeleteItemActionTriggered();

    // people
    void slotAddPerson();
    void slotEditPerson(int personID);
    void slotDeletePerson(int personID);

    // places
    void slotAddPlace();
    void slotEditPlace(int placeID);
    void slotDeletePlace(int placeID);

    // relations
    void slotAddRelation();
    void slotEditRelation(int relationID);
    void slotDeleteRelation(int relationID);

    // person contextual actions
    void slotAddFather();
    void slotAddMother();
    void slotAddSon();
    void slotAddDaugther();
    void slotAddBrother();
    void slotAddSister();

private:
    void setupActions();
    void openDatabase(const QString & dbFilePath, bool create = false);
    void initDatabase();
    Ui::MainWindow *ui;
    QActionGroup * m_viewGroup;
    QString m_filename;
    QString m_storageLocation;

    // models
    PeopleModel * m_peopleModel;
    PlacesModel * m_placesModel;
    RelationsModel * m_relationsModel;
};

#endif // MAINWINDOW_H
