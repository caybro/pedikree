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

#include <QLocale>
#include <QDebug>
#include <QPushButton>
#include <QCalendar>

#include "pddatedialog.h"
#include "ui_pddatedialog.h"

PdDateDialog::PdDateDialog(const QString &date, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PdDateDialog),
    m_initialDate(date)
{
    ui->setupUi(this);

    ui->leYear->setValidator(new QIntValidator(this));

    connect(ui->cbMonth, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PdDateDialog::slotCurrentMonthChanged);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, [this](QAbstractButton *button) {
        if (button == ui->buttonBox->button(QDialogButtonBox::Reset)) {
            slotClear();
        }
    });

    init();
    parse();
}

PdDateDialog::~PdDateDialog()
{
    delete ui;
}

void PdDateDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void PdDateDialog::slotClear()
{
    ui->cbDay->setCurrentIndex(0);
    ui->cbMonth->setCurrentIndex(0);
    ui->leYear->clear();
}

void PdDateDialog::slotCurrentMonthChanged(int month)
{
    const int day = ui->cbDay->currentIndex();

    ui->cbDay->clear();
    ui->cbDay->addItem(QString()); // empty day

    QCalendar cal;
    bool ok = false;
    int year = ui->leYear->text().toInt(&ok);
    if (!ok) year = QCalendar::Unspecified;

    for (int i = 1; i <= cal.daysInMonth(month, year); i++)
        ui->cbDay->addItem(QString::number(i));

    // try to restore the same day, if possible when changing month
    if (day < ui->cbDay->count()) {
        ui->cbDay->setCurrentIndex(day);
    } else { // set to the last day of the month
        ui->cbDay->setCurrentIndex(ui->cbDay->count() - 1);
    }
}

void PdDateDialog::init()
{
    ui->cbQualifier->addItem(tr("Exactly"), QString());
    ui->cbQualifier->addItem(tr("Around"), QStringLiteral("AROUND"));
    ui->cbQualifier->addItem(tr("Before"), QStringLiteral("BEFORE"));
    ui->cbQualifier->addItem(tr("After"), QStringLiteral("AFTER"));
    ui->cbQualifier->addItem(tr("Calculated"), QStringLiteral("CALC"));

    ui->cbMonth->addItem(QString()); // empty month
    for (int i = 1; i < 13; i++)
        ui->cbMonth->addItem(QLocale::system().standaloneMonthName(i));
}

void PdDateDialog::parse()
{
    // FORMAT: [AROUND/BEFORE/AFTER/CALC] YYYY[-MM[-DD]] or empty string

    QString datePart;

    if (m_initialDate.contains(' ')) {
        if (m_initialDate.startsWith("AROUND")) {
            ui->cbQualifier->setCurrentIndex(1);
        } else if (m_initialDate.startsWith("BEFORE")) {
            ui->cbQualifier->setCurrentIndex(2);
        } else if (m_initialDate.startsWith("AFTER")) {
            ui->cbQualifier->setCurrentIndex(3);
        } else if (m_initialDate.startsWith("CALC")) {
            ui->cbQualifier->setCurrentIndex(4);
        }

        datePart = m_initialDate.section(' ', 1, 1);
    } else {
        datePart = m_initialDate;
    }

    ui->leYear->setText(datePart.section('-', 0, 0));
    ui->cbMonth->setCurrentIndex(datePart.section('-', 1, 1).toInt());
    ui->cbDay->setCurrentIndex(datePart.section('-', 2, 2).toInt());
}

QString PdDateDialog::date() const
{
    if (ui->leYear->text().isEmpty())
        return QString();

    QString result = ui->cbQualifier->currentData().toString();
    if (!result.isEmpty())
        result += " ";

    result += ui->leYear->text();

    const QString month = ui->cbMonth->currentText();
    if (!month.isEmpty())
        result += QStringLiteral("-%1").arg(QString::number(ui->cbMonth->currentIndex()), 2, QChar('0'));

    const QString day = ui->cbDay->currentText();
    if (!day.isEmpty())
        result += QStringLiteral("-%1").arg(day, 2, QChar('0'));

    return result;
}
