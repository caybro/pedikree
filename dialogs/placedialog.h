#ifndef PLACEDIALOG_H
#define PLACEDIALOG_H

#include <QDialog>

namespace Ui {
class PlaceDialog;
}

class PlaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlaceDialog(QWidget *parent = 0, int placeID = -1);
    ~PlaceDialog();

protected:
    void changeEvent(QEvent *e);

private:
    void populateControls();
    Ui::PlaceDialog *ui;
    int m_placeID;
};

#endif // PLACEDIALOG_H
