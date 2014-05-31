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

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QBasicTimer>
#include <QWidget>

class SlippyMap;

class MapWidget: public QWidget
{
    Q_OBJECT
public:
    MapWidget(QWidget *parent = 0);
    void setCenter(qreal lat, qreal lng);

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent * event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent * event);

private slots:
    void updateMap(const QRect &r);

private:
    void zoomIn(int steps = 1);
    void zoomOut(int steps = 1);

    SlippyMap *m_normalMap;
    bool pressed;
    bool snapped;
    QPoint pressPos;
    QPoint dragPos;
    QBasicTimer tapTimer;
    bool zoomed;
    QPixmap zoomPixmap;
    QPixmap maskPixmap;
    bool invert;
};

#endif