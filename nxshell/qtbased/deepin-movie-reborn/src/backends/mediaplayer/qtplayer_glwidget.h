// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _DMR_QTPLAYER_GLWIDGET_H
#define _DMR_QTPLAYER_GLWIDGET_H

#include <QtWidgets>
#undef Bool
#include "../../vendor/qthelper.hpp"
#include <DGuiApplicationHelper>
//DWIDGET_USE_NAMESPACE



namespace dmr {
class QtPlayerGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    friend class MpvProxy;

    explicit QtPlayerGLWidget(QWidget *parent);
    virtual ~QtPlayerGLWidget();

    /**
     * rounded clipping consumes a lot of resources, and performs bad on 4K video
     */
    void toggleRoundedClip(bool bFalse);

    void setPlaying(bool);

    void setVideoTex(QImage image);

#ifdef __x86_64__
    //更新全屏时影院播放进度
    void updateMovieProgress(qint64 duration, qint64 pos);
#endif
    void setRawFormatFlag(bool bRawFormat);

protected:
    /**
     * @brief opengl初始化 cppcheck误报
     */
    void initializeGL() override;
    void resizeGL(int nWidth, int nHeight) override;
    void paintGL() override;

    void setMiniMode(bool);

private:
    void initMember();
    void updateVbo();
    void updateVboCorners();
    void updateVboBlend();

    void updateMovieFbo();
    void updateCornerMasks();

    void setupBlendPipe();
    void setupIdlePipe();

    void prepareSplashImages();

private:

    bool m_bPlaying;                   //记录播放状态
    bool m_bInMiniMode;                //是否是最小化
    bool m_bDoRoundedClipping;         //

    QOpenGLVertexArrayObject m_vao;    //顶点数组对象
    QOpenGLBuffer m_vbo;               //顶点缓冲对象
    QOpenGLTexture *m_pDarkTex;        //深色主题背景纹理
    QOpenGLTexture *m_pLightTex;       //浅色主题背景纹理
    QOpenGLShaderProgram *m_pGlProg;

    QOpenGLVertexArrayObject m_vaoBlend;
    QOpenGLBuffer m_vboBlend;
    QOpenGLShaderProgram *m_pGlProgBlend;
    QOpenGLFramebufferObject *m_pFbo;
    QOpenGLShaderProgram *m_pGlProgBlendCorners;

    //textures for corner
    QOpenGLVertexArrayObject m_vaoCorner;
    QOpenGLTexture *m_pCornerMasks[4];
    QOpenGLBuffer m_vboCorners[4];
    QOpenGLShaderProgram *m_pGlProgCorner; //着色器程序

    QImage m_imgBgDark;                    //深色主题背景图
    QImage m_imgBgLight;                   //浅色主题背景图

    QOpenGLTexture* m_pVideoTex;
    int m_currWidth;
    int m_currHeight;
#ifdef __x86_64__
    qreal m_pert; // 影院播放进度
    QString m_strPlayTime; // 播放时间显示；
#endif
    bool m_bRawFormat;
};

}

#endif /* ifndef _DMR_MPV_GLWIDGET_H */

