// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewopenglwidget.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <malloc.h>

PreviewOpenglWidget::PreviewOpenglWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_textureY = nullptr;
    m_textureU = nullptr;
    m_textureV = nullptr;
    m_yuvPtr = nullptr;
    m_program = nullptr;
    m_videoWidth = 0;
    m_videoHeight = 0;
}

int PreviewOpenglWidget::getFrameHeight()
{
    return static_cast<int>(m_videoHeight);
}

int PreviewOpenglWidget::getFrameWidth()
{
    return static_cast<int>(m_videoWidth);
}

#ifndef __mips__
void PreviewOpenglWidget::slotShowYuv(uchar *ptr, uint width, uint height)
{
    m_Rendermutex.lock();
    m_videoWidth = width;
    m_videoHeight = height;
    m_yuvPtr = ptr;//数据拷贝挪到major类

    if (m_yuvPtr)
        update();

    m_Rendermutex.unlock();
}
#endif

void PreviewOpenglWidget::initializeGL()
{

    makeCurrent();
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    static const GLfloat vertices[] {
        //顶点坐标
        -1.0f, -1.0f,
            -1.0f, +1.0f,
            +1.0f, +1.0f,
            +1.0f, -1.0f,
            //纹理坐标
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
        };

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices, sizeof(vertices));

    const char *vsrc =
        "attribute vec4 vertexIn; \
        attribute vec2 textureIn; \
        varying vec2 textureOut;  \
        void main(void)           \
        {                         \
            gl_Position = vertexIn; \
            textureOut = textureIn; \
        }";

    const char *fsrc;

    if (get_wayland_status() == 0) {
        fsrc = "varying vec2 textureOut; \
        uniform sampler2D tex_y; \
        uniform sampler2D tex_u; \
        uniform sampler2D tex_v; \
        void main(void) \
        { \
            vec3 yuv; \
            vec3 rgb; \
            yuv.x = texture2D(tex_y, textureOut).r; \
            yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
            yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
            rgb = mat3( 1,       1,         1, \
                        0,       -0.39465,  2.03211, \
                        1.13983, -0.58060,  0) * yuv; \
            gl_FragColor = vec4(rgb, 1); \
        }";
    } else {
        fsrc = "#ifdef GL_ES \
                precision mediump float; \
                #endif \
        varying vec2 textureOut; \
        uniform sampler2D tex_y; \
        uniform sampler2D tex_u; \
        uniform sampler2D tex_v; \
        void main(void) \
        { \
            vec3 yuv; \
            vec3 rgb; \
            yuv.x = texture2D(tex_y, textureOut).r; \
            yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
            yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
            rgb = mat3( 1,       1,         1, \
                        0,       -0.39465,  2.03211, \
                        1.13983, -0.58060,  0) * yuv; \
            gl_FragColor = vec4(rgb, 1); \
        }";
    }

    m_program = new QOpenGLShaderProgram(this);
    m_textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_textureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_textureV = new QOpenGLTexture(QOpenGLTexture::Target2D);

    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
    m_program->link();
    m_program->bind();
    m_program->enableAttributeArray(VERTEXIN);
    m_program->enableAttributeArray(TEXTUREIN);
    m_program->setAttributeBuffer(VERTEXIN, GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    m_program->setAttributeBuffer(TEXTUREIN, GL_FLOAT, 8 * sizeof(GLfloat), 2, 2 * sizeof(GLfloat));

    m_textureUniformY = static_cast<uint>(m_program->uniformLocation("tex_y"));
    m_textureUniformU = static_cast<uint>(m_program->uniformLocation("tex_u"));
    m_textureUniformV = static_cast<uint>(m_program->uniformLocation("tex_v"));

    m_textureY->create();
    m_textureU->create();
    m_textureV->create();
    m_idY = m_textureY->textureId();
    m_idU = m_textureU->textureId();
    m_idV = m_textureV->textureId();
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void PreviewOpenglWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    update();
}

void PreviewOpenglWidget::paintGL()
{
    if (m_yuvPtr == nullptr)
        return;
  
    glActiveTexture(GL_TEXTURE0);  //激活纹理单元GL_TEXTURE0,系统里面的
    glBindTexture(GL_TEXTURE_2D, m_idY); //绑定y分量纹理对象id到激活的纹理单元

    //使用内存中的数据创建真正的y分量纹理数据,https://blog.csdn.net/xipiaoyouzi/article/details/53584798 纹理参数解析
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(m_videoWidth), static_cast<int>(m_videoHeight), 0, GL_RED, GL_UNSIGNED_BYTE, m_yuvPtr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE1); //激活纹理单元GL_TEXTURE1
    glBindTexture(GL_TEXTURE_2D, m_idU);

    //使用内存中的数据创建真正的u分量纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth >> 1, m_videoHeight >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, m_yuvPtr + m_videoWidth * m_videoHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE2); //激活纹理单元GL_TEXTURE2
    glBindTexture(GL_TEXTURE_2D, m_idV);

    //使用内存中的数据创建真正的v分量纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_videoWidth >> 1, m_videoHeight >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, m_yuvPtr + m_videoWidth * m_videoHeight * 5 / 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //指定y纹理要使用新值
    glUniform1i(static_cast<int>(m_textureUniformY), 0);

    //指定u纹理要使用新值
    glUniform1i(static_cast<int>(m_textureUniformU), 1);

    //指定v纹理要使用新值
    glUniform1i(static_cast<int>(m_textureUniformV), 2);

    //使用顶点数组方式绘制图形
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


PreviewOpenglWidget::~PreviewOpenglWidget()
{
    m_vbo.destroy();

    if (m_textureY) {
        m_textureY->destroy();
        delete m_textureY;
        m_textureY = nullptr;
    }

    if (m_textureU) {
        m_textureU->destroy();
        delete m_textureU;
        m_textureU = nullptr;
    }

    if (m_textureV) {
        m_textureV->destroy();
        delete m_textureV;
        m_textureV = nullptr;
    }

    doneCurrent();
}
