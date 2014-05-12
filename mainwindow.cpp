#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "placesmodel.h"

#include <QtSql>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_viewGroup(new QActionGroup(this)),
    m_peopleModel(0),
    m_placesModel(0)
{
    ui->setupUi(this);
    setupActions();
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // all table views are readonly
}

MainWindow::~MainWindow()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen())
        db.close();
    QSqlDatabase::removeDatabase(db.connectionName());

    delete ui;
}

void MainWindow::slotOpen()
{
    //const QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath(), "", 0, QFileDialog::DontUseNativeDialog); // FIXME kfiledialog crashing
    const QString filename = "/home/ltinkl/git/pedikree/tinkl.xml";
    if (!filename.isEmpty()) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("/home/ltinkl/git/pedikree/test.db"); // TODO
        if (db.open()) {
            m_filename = db.databaseName();
            setWindowFilePath(m_filename);
            ui->actionViewPeople->trigger();
        }
        else
            qWarning() << "Error opening the DB" << db.lastError().text();
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
        if (!m_peopleModel)
            m_peopleModel = new PeopleModel(this);
        m_peopleModel->exec();
        ui->treeView->setModel(m_peopleModel);
    } else if (action == ui->actionViewPlaces) {
        if (!m_placesModel)
            m_placesModel = new PlacesModel(this);
        m_placesModel->exec();
        ui->treeView->setModel(m_placesModel);
    }
}

void MainWindow::setupActions()
{
    m_viewGroup->setExclusive(true);
    m_viewGroup->addAction(ui->actionViewPeople);
    m_viewGroup->addAction(ui->actionViewEvents);
    m_viewGroup->addAction(ui->actionViewPlaces);
    connect(m_viewGroup, &QActionGroup::triggered, this, &MainWindow::slotSwitchView);

    ui->actionNew->setShortcut(QKeySequence(QKeySequence::New));
    ui->actionOpen->setShortcut(QKeySequence(QKeySequence::Open));
    ui->actionSaveAs->setShortcut(QKeySequence(QKeySequence::SaveAs));
    ui->actionQuit->setShortcut(QKeySequence(QKeySequence::Quit));

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::slotOpen);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::slotAbout);
    connect(ui->actionQuit, &QAction::triggered, qApp, &QApplication::quit);
}
