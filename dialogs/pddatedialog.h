/*
 * Copyright (C) 2014 Lukáš Tinkl <lukas@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#pragma once

#include <QDialog>

namespace Ui {
class PdDateDialog;
}

class PdDateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PdDateDialog(const QString & date, QWidget *parent = nullptr);
    ~PdDateDialog();

    QString date() const;

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void slotClear();
    void slotCurrentMonthChanged(int month);

private:
    void init();
    void parse();
    Ui::PdDateDialog *ui;
    QString m_initialDate;
};
