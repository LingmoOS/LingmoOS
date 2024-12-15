// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLTESTWIDGET_H
#define GLTESTWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QThread>
#include <QProcess>
#include <QDebug>
#include <GL/gl.h>
#include <GL/glut.h>

class GLTestWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit GLTestWidget(QWidget *parent = nullptr);

protected:
    void initializeGL();

    void resizeGL(int w, int h);

    void paintGL();

private:
    float m_xRotated, m_yRotated, m_zRotated;
    unsigned int m_currPaintCnt;
    unsigned int m_lastPaintCnt;
};

#endif
