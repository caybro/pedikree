/****************************************************************************
 **
 ** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the demonstration applications of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Digia.  For licensing terms and
 ** conditions see http://qt.digia.com/licensing.  For further information
 ** use the contact form at http://qt.digia.com/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Digia gives you certain additional
 ** rights.  These rights are described in the Digia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 3.0 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU General Public License version 3.0 requirements will be
 ** met: http://www.gnu.org/copyleft/gpl.html.
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include <math.h>

#include "mapwidget.h"
#include "slippymap.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// how long (milliseconds) the user need to hold (after a tap on the screen)
// before triggering the magnifying glass feature
// 701, a prime number, is the sum of 229, 233, 239
// (all three are also prime numbers, consecutive!)
#define HOLD_TIME 701

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent), pressed(false), snapped(false), zoomed(false),
      invert(false)
{
    m_normalMap = new SlippyMap(this);
    connect(m_normalMap, SIGNAL(updated(QRect)), SLOT(updateMap(QRect)));
}

void MapWidget::setCenter(qreal lat, qreal lng)
{
    m_normalMap->latitude = lat;
    m_normalMap->longitude = lng;
    m_normalMap->invalidate();
}

void MapWidget::updateMap(const QRect &r)
{
    update(r);
}

void MapWidget::resizeEvent(QResizeEvent *)
{
    m_normalMap->width = width();
    m_normalMap->height = height();
    m_normalMap->invalidate();
}

void MapWidget::paintEvent(QPaintEvent *event)
{
    QPainter p;
    p.begin(this);
    m_normalMap->render(&p, event->rect());
    p.setPen(Qt::black);

    QFont font = p.font();
    font.setPointSize(8);
    p.setFont(font);

    p.drawText(rect(), Qt::AlignBottom | Qt::AlignRight | Qt::TextWordWrap, QStringLiteral("CC BY-SA © OpenStreetMap contributors"));
    p.end();
}

void MapWidget::timerEvent(QTimerEvent *)
{
    update();
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() != Qt::LeftButton)
        return;
    pressed = snapped = true;
    pressPos = dragPos = event->pos();
    tapTimer.stop();
    tapTimer.start(HOLD_TIME, this);
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!event->buttons())
        return;
    if (!zoomed) {
        if (!pressed || !snapped) {
            QPoint delta = event->pos() - pressPos;
            pressPos = event->pos();
            m_normalMap->pan(delta);
            return;
        } else {
            const int threshold = 10;
            QPoint delta = event->pos() - pressPos;
            if (snapped) {
                snapped &= delta.x() < threshold;
                snapped &= delta.y() < threshold;
                snapped &= delta.x() > -threshold;
                snapped &= delta.y() > -threshold;
            }
            if (!snapped)
                tapTimer.stop();
        }
    } else {
        dragPos = event->pos();
        update();
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent *)
{
    zoomed = false;
    update();
}

void MapWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!event->buttons())
        return;
    if (event->button() == Qt::LeftButton)
        zoomIn();
    else if (event->button() == Qt::RightButton)
        zoomOut();
}

void MapWidget::keyPressEvent(QKeyEvent *event)
{
    if (!zoomed) {
        if (event->key() == Qt::Key_Left)
            m_normalMap->pan(QPoint(20, 0));
        if (event->key() == Qt::Key_Right)
            m_normalMap->pan(QPoint(-20, 0));
        if (event->key() == Qt::Key_Up)
            m_normalMap->pan(QPoint(0, 20));
        if (event->key() == Qt::Key_Down)
            m_normalMap->pan(QPoint(0, -20));
    } else {
        QPoint delta(0, 0);
        if (event->key() == Qt::Key_Left)
            delta = QPoint(-15, 0);
        if (event->key() == Qt::Key_Right)
            delta = QPoint(15, 0);
        if (event->key() == Qt::Key_Up)
            delta = QPoint(0, -15);
        if (event->key() == Qt::Key_Down)
            delta = QPoint(0, 15);
        if (delta != QPoint(0, 0)) {
            dragPos += delta;
            update();
        }
    }
}

void MapWidget::wheelEvent(QWheelEvent *event)
{
    const int scale = event->angleDelta().y()/120;

    if (scale > 0) {
        zoomIn(scale);
    } else if (scale < 0) {
        zoomOut(-scale);
    }

    event->accept();
}

void MapWidget::zoomIn(int steps)
{
    m_normalMap->zoom = qBound(0, m_normalMap->zoom+steps, 19);
    m_normalMap->invalidate();
}

void MapWidget::zoomOut(int steps)
{
    m_normalMap->zoom = qBound(0, m_normalMap->zoom-steps, 19);
    m_normalMap->invalidate();
}
