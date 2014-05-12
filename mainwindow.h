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
    void slotOpen();
    void slotAbout();

    void slotSwitchView(QAction * action);

private:
    void setupActions();
    Ui::MainWindow *ui;
    QActionGroup * m_viewGroup;
    QString m_filename;

    // models
    PeopleModel * m_peopleModel;
    PlacesModel * m_placesModel;
};

#endif // MAINWINDOW_H
