// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gltestwidget.h"
#include <QException>

GLTestWidget::GLTestWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_currPaintCnt(0),
      m_lastPaintCnt(0)
{
    QTimer *auto_quit_timer = new QTimer(this);
    auto_quit_timer->setInterval(1000);

    QObject::connect(auto_quit_timer, &QTimer::timeout, [=] {
        qDebug() << "m_lastPaintCnt=" << m_lastPaintCnt;
        qDebug() << "m_currPaintCnt=" << m_currPaintCnt;
        if (m_currPaintCnt == m_lastPaintCnt) {
            qDebug() << "detect GLTest paint error";
            exit(-1);
        } else {
            m_lastPaintCnt = m_currPaintCnt;
        }
    });

    auto_quit_timer->start();

    m_xRotated = m_yRotated = m_zRotated = 0;

    QTimer *t = new QTimer(this);
    t->setInterval(1000 / 60);
    t->start();

    connect(t, &QTimer::timeout, this, [=] {
        m_xRotated += 1.0f;
        m_yRotated += 1.0f;
        m_zRotated += 1.0f;
        update();
    });
}

void GLTestWidget::initializeGL()
{
    try {
        qDebug() << "@#@#@Call initializeGL";
        glClearColor(0.3f, 0.3f, 0.6f, 1);
        glClearDepth(1.0);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_NORMALIZE);
        // glEnable(GL_POLYGON_OFFSET_FILL);
        // glPolygonMode(GL_FRONT, GL_LINE);
        glShadeModel(GL_SMOOTH);
    } catch (QException &e) {
        qDebug() << "initializeGL failed:" << e.what();
        exit(-1);
    }

}

void GLTestWidget::resizeGL(int w, int h)
{
    try {
        qDebug() << "@#@#@Call resizeGL";
        glViewport(0, 0, w, h);
    } catch (QException &e) {
        qDebug() << "resizeGL failed:" << e.what();
        exit(-1);
    }

}

void GLTestWidget::paintGL()
{
    try {
        glMatrixMode(GL_MODELVIEW);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        GLfloat ambientColor[] = {0.6f, 0.3f, 0.7f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

        GLfloat lightColor0[] = {1.0f, 0.0f, 0.0f, 1.0f};
        GLfloat lightPos0[] = {10.0f, 10.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

        GLfloat lightColor1[] = {0.4f, 0.7f, 0.2f, 1.0f};
        GLfloat lightPos1[] = {0.0f, 0.0f, -5.0f, 0.0f};
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

        // glColor3f(0.3, 0.4, 0.7);
        glRotatef(m_xRotated, 1.0, 0.0, 0.0);
        glRotatef(m_yRotated, 0.0, 1.0, 0.0);
        glRotatef(m_zRotated, 0.0, 0.0, 1.0);
        glScalef(.4f, .4f, .4f);
        glColor3f(0.3f, 0.4f, 0.4f);
        glutSolidTeapot(1.0);
        m_currPaintCnt++;
    } catch (QException &e) {
        qDebug() << "paintGL failed:" << e.what();
        exit(-1);
    }

}
