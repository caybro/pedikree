#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>

#include "person.h"

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

private:
    void setupActions();
    Ui::MainWindow *ui;
    QActionGroup * m_viewGroup;
    QString m_filename;

    //models
    PersonModel * m_personModel;
};

#endif // MAINWINDOW_H
