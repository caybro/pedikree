#ifndef PERSONDIALOG_H
#define PERSONDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class PersonDialog;
}

class PersonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PersonDialog(QWidget *parent = 0, bool create = false);
    ~PersonDialog();

protected:
    void changeEvent(QEvent *e);

private slots:
    void genderClicked(QAbstractButton * button);
    void deadAliveClicked(QAbstractButton * button);

private:
    Ui::PersonDialog *ui;
    bool m_create;
};

#endif // PERSONDIALOG_H
