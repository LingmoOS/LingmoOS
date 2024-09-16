// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _DMR_MPV_GLWIDGET_H
#define _DMR_MPV_GLWIDGET_H

#include <QtWidgets>
#include <mpv/render.h>
#include <mpv/render_gl.h>
#include <mpv_proxy.h>
#undef Bool
#include "../../vendor/qthelper.hpp"
#include <DGuiApplicationHelper>
//DWIDGET_USE_NAMESPACE

//add by heyi
typedef void (*mpv_render_contextSet_update_callback)(mpv_render_context *ctx,
                                                      mpv_render_update_fn callback,
                                                      void *callback_ctx);
typedef void (*mpv_render_contextReport_swap)(mpv_render_context *ctx);
typedef void (*mpv_renderContext_free)(mpv_render_context *ctx);
typedef int (*mpv_renderContext_create)(mpv_render_context **res, mpv_handle *mpv,
                                        mpv_render_param *params);
typedef int (*mpv_renderContext_render)(mpv_render_context *ctx, mpv_render_param *params);
typedef uint64_t (*mpv_renderContext_update)(mpv_render_context *ctx);


namespace dmr {
class MpvGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    friend class MpvProxy;

    MpvGLWidget(QWidget *parent, MpvHandle h);
    virtual ~MpvGLWidget();

    /**
     * rounded clipping consumes a lot of resources, and performs bad on 4K video
     */
    void toggleRoundedClip(bool bFalse);
    //add by heyi
    /**
     * @brief setHandle 设置句柄
     * @param h 传入的句柄
     */
    void setHandle(MpvHandle h);

protected:
    /**
     * @brief opengl初始化 cppcheck误报
     */
    void initializeGL() override;
    void resizeGL(int nWidth, int nHeight) override;
    void paintGL() override;

    void setPlaying(bool);
    void setMiniMode(bool);
    //add by heyi
    /**
     * @brief initMpvFuns 第一次播放需要初库始化函数指针
     */
    void initMpvFuns();
#if 0
    //更新全屏时影院播放进度
    void updateMovieProgress(qint64 duration, qint64 pos);
#endif
    void setRawFormatFlag(bool bRawFormat);

protected slots:
    void onNewFrame();
    void onFrameSwapped();

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
    MpvHandle m_handle;                //mpv句柄
    mpv_render_context *m_pRenderCtx;  //mpv渲染上下文

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

    //add by heyi
    mpv_render_contextSet_update_callback m_callback;
    mpv_render_contextReport_swap m_context_report;
    mpv_renderContext_free m_renderContex;
    mpv_renderContext_create m_renderCreat;
    mpv_renderContext_render m_renderContexRender;
    mpv_renderContext_update m_renderContextUpdate;
#ifdef __x86_64__
    qreal m_pert; //影院播放进度
    QString m_strPlayTime; //播放时间显示；
#endif
    bool m_bRawFormat;     // 播放内容为原始格式文件标志
};

}

#endif /* ifndef _DMR_MPV_GLWIDGET_H */

