#ifndef PDDATEDIALOG_H
#define PDDATEDIALOG_H

#include <QDialog>

namespace Ui {
class PdDateDialog;
}

class PdDateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PdDateDialog(const QString & date, QWidget *parent = 0);
    ~PdDateDialog();

    QString date() const;

protected:
    void changeEvent(QEvent *e);

private:
    void init();
    void parse();
    Ui::PdDateDialog *ui;
    QString m_initialDate;
};

#endif // PDDATEDIALOG_H
