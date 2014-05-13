#ifndef PERSONDIALOG_H
#define PERSONDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class PersonDialog;
}

class PersonDialog: public QDialog
{
    Q_OBJECT

public:
    explicit PersonDialog(QWidget *parent = 0, int personID = -1);
    ~PersonDialog();

protected:
    void changeEvent(QEvent *e);

private slots:
    void genderClicked(QAbstractButton * button);
    void deadAliveClicked(QAbstractButton * button);

private:
    void populateControls();
    Ui::PersonDialog *ui;
    int m_personID;
};

#endif // PERSONDIALOG_H
