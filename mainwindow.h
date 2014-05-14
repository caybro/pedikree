#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>

#include "peoplemodel.h"
#include "placesmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    // menu actions
    void slotNew();
    void slotOpen();
    void slotAbout();

    void slotSwitchView(QAction * action);

    // table view handling
    void tableViewSelectionChanged(const QModelIndex &index);
    void tableViewDoubleClicked(const QModelIndex &index);
    void tableViewContextMenuRequested(const QPoint & pos);

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
};

#endif // MAINWINDOW_H
