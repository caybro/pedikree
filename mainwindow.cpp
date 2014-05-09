#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_viewGroup(new QActionGroup(this)),
    m_personModel(0)
{
    ui->setupUi(this);
    setupActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotOpen()
{
    //const QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath(), "", 0, QFileDialog::DontUseNativeDialog); // FIXME kfiledialog crashing
    const QString filename = "/home/ltinkl/git/pedikree/tinkl.xml";
    if (!filename.isEmpty()) {
        m_filename = filename;
        setWindowFilePath(m_filename);

        if (m_personModel) {
            m_personModel->deleteLater();
        }
        m_personModel = new PersonModel(m_filename, this);
        ui->treeView->setModel(m_personModel); // TODO switch view
        ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    }
}

void MainWindow::slotAbout()
{
    QMessageBox::about(this, tr("About Pedikree"),
                       tr("A simple genealogy program written using Qt") + "\n\n" +
                       QString::fromUtf8("(c) Lukáš Tinkl <lukas@kde.org>") + "\n\n" +
                       tr("Version: %1").arg(qApp->applicationVersion()));
}

void MainWindow::setupActions()
{
    m_viewGroup->setExclusive(true);
    m_viewGroup->addAction(ui->actionViewPeople);
    m_viewGroup->addAction(ui->actionViewEvents);
    m_viewGroup->addAction(ui->actionViewPlaces);

    ui->actionNew->setShortcut(QKeySequence(QKeySequence::New));
    ui->actionOpen->setShortcut(QKeySequence(QKeySequence::Open));
    ui->actionSave->setShortcut(QKeySequence(QKeySequence::Save));
    ui->actionSaveAs->setShortcut(QKeySequence(QKeySequence::SaveAs));
    ui->actionQuit->setShortcut(QKeySequence(QKeySequence::Quit));

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::slotOpen);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::slotAbout);
    connect(ui->actionQuit, &QAction::triggered, qApp, &QApplication::quit);
}
