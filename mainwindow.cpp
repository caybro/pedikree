#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "placesmodel.h"

#include "dialogs/persondialog.h"

#include <QtSql>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QStandardPaths>
#include <QDebug>

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
    connect(ui->tableView, &QTableView::doubleClicked, this, &MainWindow::treeViewDoubleClicked);
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

void MainWindow::treeViewDoubleClicked(const QModelIndex &index)
{
    if (!ui->tableView->model())
        return;

    const int row = index.row();

    if (m_viewGroup->checkedAction() == ui->actionViewPeople) {
        PeopleModel * model = qobject_cast<PeopleModel *>(ui->tableView->model());
        if (model) {
            qDebug() << "Row" << row << "has DB ID:" << model->idAtRow(row);
            PersonDialog * dlg = new PersonDialog(this);
            dlg->setWindowTitle(tr("Edit Person"));
            if (dlg->exec() == QDialog::Accepted) {
                // TODO edit the stuff
            }
        }
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
    }
    else {
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
