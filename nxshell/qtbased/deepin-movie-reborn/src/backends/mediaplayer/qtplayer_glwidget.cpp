// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include "mpv_proxy.h"
#include "qtplayer_glwidget.h"

#include <dthememanager.h>
#include <DApplication>
#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(__SCITECH_SNAP__)
/* Win32 but not WinCE */
#   define KHRONOS_APIENTRY __stdcall
#else
#   define KHRONOS_APIENTRY
#endif
DWIDGET_USE_NAMESPACE
#ifndef EGLAPIENTRY
#define EGLAPIENTRY  KHRONOS_APIENTRY
#endif

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

static const char *vs_blend = R"(
attribute vec2 position;
attribute vec2 vTexCoord;

varying vec2 texCoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    texCoord = vTexCoord;
}
)";

static const char* fs_blend = R"(
varying vec2 texCoord;

uniform sampler2D movie;

void main() {
     gl_FragColor = texture2D(movie, texCoord);
}
)";

static const char* fs_blend_wayland = R"(
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 texCoord;

uniform sampler2D movie;

void main() {
     gl_FragColor = texture2D(movie, texCoord);
}
)";

static const char* vs_blend_corner = R"(
attribute vec2 position;
attribute vec2 maskTexCoord;
attribute vec2 vTexCoord;

varying vec2 maskCoord;
varying vec2 texCoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    texCoord = vTexCoord;
    maskCoord = maskTexCoord;
}
)";

static const char* fs_blend_corner = R"(
varying vec2 maskCoord;
varying vec2 texCoord;

uniform sampler2D movie;
uniform sampler2D mask;

void main() {
     gl_FragColor = texture2D(movie, texCoord) * texture2D(mask, maskCoord).a;
}
)";

static const char* fs_blend_corner_wayland = R"(
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 maskCoord;
varying vec2 texCoord;

uniform sampler2D movie;
uniform sampler2D mask;

void main() {
     gl_FragColor = texture2D(movie, texCoord) * texture2D(mask, maskCoord).a;
}
)";

static const char* vs_code = R"(
attribute vec2 position;
attribute vec2 vTexCoord;

varying vec2 texCoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    texCoord = vTexCoord;
}
)";

static const char* fs_code = R"(
varying vec2 texCoord;

uniform sampler2D sampler;
uniform vec4 bg;

void main() {
    vec4 s = texture2D(sampler, texCoord);
    gl_FragColor = vec4(s.rgb * s.a + bg.rgb * (1.0 - s.a), 1.0);
}
)";

static const char* fs_code_wayland = R"(
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 texCoord;

uniform sampler2D sampler;
uniform vec4 bg;

void main() {
    vec4 s = texture2D(sampler, texCoord);
    gl_FragColor = vec4(s.rgb * s.a + bg.rgb * (1.0 - s.a), 1.0);
}
)";

static const char* fs_corner_code = R"(
varying vec2 texCoord;

uniform sampler2D corner;
uniform vec4 bg;

void main() {
    vec4 s = texture2D(corner, texCoord);
    gl_FragColor = s.a * bg;
}
)";

static const char* fs_corner_code_wayland = R"(
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 texCoord;

uniform sampler2D corner;
uniform vec4 bg;

void main() {
    vec4 s = texture2D(corner, texCoord);
    gl_FragColor = s.a * bg;
}
)";

namespace dmr {

    QtPlayerGLWidget::QtPlayerGLWidget(QWidget *parent)
        :QOpenGLWidget(parent) {

        initMember();

        setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    }

    QtPlayerGLWidget::~QtPlayerGLWidget()
    {
        makeCurrent();
        if (m_pDarkTex) {
            m_pDarkTex->destroy();
            delete m_pDarkTex;
        }
        if (m_pLightTex) {
            m_pLightTex->destroy();
            delete m_pLightTex;
        }

        for (auto mask: m_pCornerMasks) {
            if (mask) mask->destroy();
        }

        m_vbo.destroy();

        for (int i = 0; i < 4; i++) {
            m_vboCorners[i].destroy();

            ///指针数组m_pCornerMasks在mpv_glwidget.cpp 476行申请内存后未释放///
            delete m_pCornerMasks[i];
            m_pCornerMasks[i] = nullptr;
        }

        m_vao.destroy();
        m_vaoBlend.destroy();
        m_vaoCorner.destroy();

        delete m_pGlProgBlend;
        m_pGlProgBlend = nullptr;

        delete m_pGlProgBlendCorners;
        m_pGlProgBlendCorners = nullptr;;

        delete m_pGlProg;
        m_pGlProg = nullptr;

        delete m_pGlProgCorner;
        m_pGlProgCorner = nullptr;

        if (m_pFbo) delete m_pFbo;
        doneCurrent();
    }

    void QtPlayerGLWidget::setupBlendPipe()
    {
        updateMovieFbo();

        m_vaoBlend.create();
        m_vaoBlend.bind();
        updateVboBlend();

        m_pGlProgBlend = new QOpenGLShaderProgram();
        m_pGlProgBlend->addShaderFromSourceCode(QOpenGLShader::Vertex, vs_blend);
        if(utils::check_wayland_env()){
            m_pGlProgBlend->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_blend_wayland);
        }else {
            m_pGlProgBlend->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_blend);
        }

        if (!m_pGlProgBlend->link()) {
            qInfo() << "link failed";
        }
        m_pGlProgBlend->bind();
        m_vboBlend.bind();

        int vLocBlend = m_pGlProgBlend->attributeLocation("position");
        int coordLocBlend = m_pGlProgBlend->attributeLocation("vTexCoord");
        m_pGlProgBlend->enableAttributeArray(vLocBlend);
        m_pGlProgBlend->setAttributeBuffer(vLocBlend, GL_FLOAT, 0, 2, 6*sizeof(GLfloat));
        m_pGlProgBlend->enableAttributeArray(coordLocBlend);
        m_pGlProgBlend->setAttributeBuffer(coordLocBlend, GL_FLOAT, 2*sizeof(GLfloat), 2, 6*sizeof(GLfloat));
        m_pGlProgBlend->setUniformValue("movie", 0);
        m_pGlProgBlend->release();
        m_vaoBlend.release();

        m_pGlProgBlendCorners = new QOpenGLShaderProgram();
        m_pGlProgBlendCorners->addShaderFromSourceCode(QOpenGLShader::Vertex, vs_blend_corner);
        if(utils::check_wayland_env()){
            m_pGlProgBlendCorners->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_blend_corner_wayland);
        }else{
            m_pGlProgBlendCorners->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_blend_corner);
        }

        if (!m_pGlProgBlendCorners->link()) {
            qInfo() << "link failed";
        }
    }

    void QtPlayerGLWidget::setupIdlePipe()
    {
        m_vao.create();
        m_vao.bind();

        m_pDarkTex = new QOpenGLTexture(m_imgBgDark, QOpenGLTexture::DontGenerateMipMaps);
        m_pDarkTex->setMinificationFilter(QOpenGLTexture::Linear);
        m_pLightTex = new QOpenGLTexture(m_imgBgLight, QOpenGLTexture::DontGenerateMipMaps);
        m_pLightTex->setMinificationFilter(QOpenGLTexture::Linear);

        updateVbo();
        m_vbo.bind();

        m_pGlProg = new QOpenGLShaderProgram();
        m_pGlProg->addShaderFromSourceCode(QOpenGLShader::Vertex, vs_code);
        if(utils::check_wayland_env()){
            m_pGlProg->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_code_wayland);
        }else{
            m_pGlProg->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_code);
        }

        if (!m_pGlProg->link()) {
            qInfo() << "link failed";
        }
        m_pGlProg->bind();

        int vertexLoc = m_pGlProg->attributeLocation("position");
        int coordLoc = m_pGlProg->attributeLocation("vTexCoord");
        m_pGlProg->enableAttributeArray(vertexLoc);
        m_pGlProg->setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 2, 4*sizeof(GLfloat));
        m_pGlProg->enableAttributeArray(coordLoc);
        m_pGlProg->setAttributeBuffer(coordLoc, GL_FLOAT, 2*sizeof(GLfloat), 2, 4*sizeof(GLfloat));
        m_pGlProg->setUniformValue("sampler", 0);
        m_pGlProg->release();
        m_vao.release();

        {
            m_vaoCorner.create();
            m_vaoCorner.bind();

            // setting up corners
            updateVboCorners();
            updateCornerMasks();

            m_pGlProgCorner = new QOpenGLShaderProgram();
            m_pGlProgCorner->addShaderFromSourceCode(QOpenGLShader::Vertex, vs_code);
            if(utils::check_wayland_env()){
                m_pGlProgCorner->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_corner_code_wayland);
            }else{
                m_pGlProgCorner->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_corner_code);
            }

            if (!m_pGlProgCorner->link()) {
                qInfo() << "link failed";
            }
            m_vaoCorner.release();
        }
    }

    void QtPlayerGLWidget::prepareSplashImages()
    {
        QPixmap pixmap;
        QImage img=utils::LoadHiDPIImage(":/resources/icons/dark/init-splash-bac.svg");
        pixmap=pixmap.fromImage(img);

        QPixmap pixmap2;
        QImage img1=QIcon::fromTheme("deepin-movie").pixmap(130, 130).toImage();
        pixmap2=pixmap2.fromImage(img1);

        QPainter painter(&pixmap);
        painter.drawPixmap(98,127,pixmap2);
        m_imgBgDark=pixmap.toImage();
        m_imgBgDark.setDevicePixelRatio(qApp->devicePixelRatio());

        QPixmap pixmap3;
        QImage image(pixmap.size(),QImage::Format_Alpha8);
        image.fill(QColor(0, 0, 0, 0));
        image.setDevicePixelRatio(qApp->devicePixelRatio());
        pixmap3=pixmap3.fromImage(image);

        QPixmap pixmap4;
        QImage img2=QIcon::fromTheme("deepin-movie").pixmap(130, 130).toImage();
        pixmap4=pixmap4.fromImage(img2);

        QPainter painter1(&pixmap3);
        painter1.drawPixmap(98,127,pixmap4);
        m_imgBgLight = pixmap3.toImage();
        m_imgBgLight.setDevicePixelRatio(qApp->devicePixelRatio());
    }

    //cppcheck误报
    void QtPlayerGLWidget::initializeGL()
    {
        QOpenGLFunctions *pGLFunction = QOpenGLContext::currentContext()->functions();
        float a = static_cast<float>(16.0 / 255.0);

        if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()){
            a = static_cast<float>(252.0 / 255.0);
        }
        pGLFunction->glClearColor(a, a, a, 1.0);

        prepareSplashImages();
        setupIdlePipe();
        setupBlendPipe();

#ifdef _LIBDMR_
        if(utils::check_wayland_env()){
            toggleRoundedClip(true);
        }else{
            toggleRoundedClip(false);
        }
#else
#ifndef USE_DXCB
        connect(window()->windowHandle(), &QWindow::windowStateChanged, [=]() {
            QWidget* pTopWid = this->topLevelWidget();
            bool rounded = !pTopWid->isFullScreen() && !pTopWid->isMaximized();
            if(utils::check_wayland_env()){
                toggleRoundedClip(true);
            } else {
                toggleRoundedClip(rounded);
            }
        });
#endif
#endif
    }

    void QtPlayerGLWidget::updateMovieFbo()
    {
        if (!m_bDoRoundedClipping) return;

        auto desiredSize = size() * qApp->devicePixelRatio();

        if (m_pFbo) {
            if (m_pFbo->size() == desiredSize) {
                return;
            }
            m_pFbo->release();
            delete m_pFbo;
        }
        m_pFbo = new QOpenGLFramebufferObject(desiredSize);
    }

    void QtPlayerGLWidget::updateCornerMasks()
    {
        if (!utils::check_wayland_env() && !m_bDoRoundedClipping) return;

        for (int i = 0; i < 4; i++) {
            QSize sz(RADIUS, RADIUS);
            QImage img(sz, QImage::Format_ARGB32);
            img.fill(Qt::transparent);

            QPainter painter;
            painter.begin(&img);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath pp;
            switch (i) {
                case 0:
                    pp.moveTo({0, static_cast<qreal>(sz.height())});
                    pp.arcTo(QRectF(0, 0, RADIUS*2, RADIUS*2), 180.0, -90.0);
                    pp.lineTo(RADIUS, RADIUS);
                    pp.closeSubpath();
                    break;

                case 1:
                    pp.moveTo({0, 0});
                    pp.arcTo(QRectF(-RADIUS, 0, RADIUS*2, RADIUS*2), 90.0, -90.0);
                    pp.lineTo(0, RADIUS);
                    pp.closeSubpath();
                    break;

                case 2:
                    pp.moveTo({static_cast<qreal>(sz.width()), 0});
                    pp.arcTo(QRectF(-RADIUS, -RADIUS, RADIUS*2, RADIUS*2), 0.0, -90.0);
                    pp.lineTo(0, 0);
                    pp.closeSubpath();
                    break;

                case 3:
                    pp.moveTo({static_cast<qreal>(sz.width()), static_cast<qreal>(sz.height())});
                    pp.arcTo(QRectF(0, -RADIUS, RADIUS*2, RADIUS*2), 270.0, -90.0);
                    pp.lineTo(RADIUS, 0);
                    pp.closeSubpath();
                    break;
                default: return;
            }

            painter.setPen(Qt::red);
            painter.setBrush(Qt::red);
            painter.drawPath(pp);
            painter.end();

            if (m_pCornerMasks[i] == nullptr) {
                m_pCornerMasks[i] = new QOpenGLTexture(img, QOpenGLTexture::DontGenerateMipMaps);
                m_pCornerMasks[i]->setMinificationFilter(QOpenGLTexture::Linear);
                m_pCornerMasks[i]->setWrapMode(QOpenGLTexture::ClampToEdge);
            }
        }
    }

    void QtPlayerGLWidget::updateVboBlend()
    {
        if (!m_vboBlend.isCreated()) {
            m_vboBlend.create();
        }

        QSize rectSize = rect().size();
        int nImgWidth = m_currWidth;
        int nImgHeigth = m_currHeight;

        GLfloat x1 = -1.0f;
        GLfloat x2 =  1.0f;
        GLfloat y1 =  1.0f;
        GLfloat y2 = -1.0f;

        GLfloat s1 = 0.0f;
        GLfloat t1 = 1.0f;
        GLfloat s2 = 1.0f;
        GLfloat t2 = 0.0f;

        float wRate = float(rectSize.width()) / nImgWidth;
        float hRate = float(rectSize.height()) / nImgHeigth;

        if(wRate < hRate)
        {
            x1 = -1.0f;
            x2 = 1.0f;
            y2 = float(rectSize.height() - nImgHeigth * wRate) / rectSize.height() - 1.0f;
            y1 = 1.0f - float(rectSize.height() - nImgHeigth * wRate) / rectSize.height();
        }else {
            x1 = float(rectSize.width() - nImgWidth * hRate) / rectSize.width() - 1.0f;
            x2 = 1.0f - float(rectSize.width() - nImgWidth * hRate) / rectSize.width();
            y2 = -1.0f;
            y1 = 1.0f ;
        }

        GLfloat vdata[] = {
            x1, y1, s1, t2, 0.0f, 1.0f,
            x2, y1, s2, t2, 1.0f, 1.0f,
            x2, y2, s2, t1, 1.0f, 0.0f,

            x1, y1, s1, t2, 0.0f, 1.0f,
            x2, y2, s2, t1, 1.0f, 0.0f,
            x1, y2, s1, t1, 0.0f, 0.0f
        };

        m_vboBlend.bind();
        m_vboBlend.allocate(vdata, sizeof(vdata));
        m_vboBlend.release();
    }

    void QtPlayerGLWidget::updateVboCorners()
    {
        auto vp = rect().size();
        auto tex_sz = QSize(RADIUS, RADIUS);
        auto r = QRect(0, 0, vp.width(), vp.height());

        QPoint pos[4] = {
            {0, r.height() - tex_sz.height()}, //top left
            {r.width() - tex_sz.width(), r.height() - tex_sz.height()}, //top right
            {r.width() - tex_sz.width(), 0}, //bottom right
            {0, 0}, //bottom left
        };

        for (int i = 0; i < 4; i++) {
            if (!m_vboCorners[i].isCreated()) {
                m_vboCorners[i].create();
            }

            auto r2 = QRect(pos[i], tex_sz);

            GLfloat x1 = static_cast<float>(r2.left()) / r.width();
            GLfloat x2 = static_cast<float>(r2.right()+1) / r.width();
            GLfloat y1 = static_cast<float>(r2.top()) / r.height();
            GLfloat y2 = static_cast<float>(r2.bottom()+1) / r.height();

            x1 = static_cast<GLfloat>(static_cast<double>(x1) * 2.0 - 1.0);
            x2 = static_cast<GLfloat>(static_cast<double>(x2) * 2.0 - 1.0);
            y1 = static_cast<GLfloat>(static_cast<double>(y1) * 2.0 - 1.0);
            y2 = static_cast<GLfloat>(static_cast<double>(y2) * 2.0 - 1.0);

            // for video tex coord
            GLfloat s1 = static_cast<GLfloat>(r2.left()) / r.width();
            GLfloat s2 = static_cast<GLfloat>(r2.right()+1) / r.width();
            GLfloat t2 = static_cast<GLfloat>(r2.top()) / r.height();
            GLfloat t1 = static_cast<GLfloat>(r2.bottom()+1) / r.height();
            
            // corner(and video) coord, corner-tex-coord, and video-as-tex-coord
            GLfloat vdata[] = {
                x1, y1,  0.0f, 1.0f,  s1, t2,
                x2, y1,  1.0f, 1.0f,  s2, t2,
                x2, y2,  1.0f, 0.0f,  s2, t1,
                                             
                x1, y1,  0.0f, 1.0f,  s1, t2,
                x2, y2,  1.0f, 0.0f,  s2, t1,
                x1, y2,  0.0f, 0.0f,  s1, t1,
            };
            m_vboCorners[i].bind();
            m_vboCorners[i].allocate(vdata, sizeof(vdata));
            m_vboCorners[i].release();
        }
    }

    void QtPlayerGLWidget::updateVbo()
    {
        if (!m_vbo.isCreated()) {
            m_vbo.create();
        }
        //HACK: we assume if any of width or height is 380, then we are in mini mode
        auto vp = rect().size();

        auto bg_size = QSizeF(m_imgBgDark.size()) / devicePixelRatioF();
        m_bInMiniMode = vp.width() <= 380 || vp.height() <= 380;
        auto tex_sz = m_bInMiniMode ? bg_size/2 : bg_size;

        auto r = QRectF(0, 0, vp.width(), vp.height());
        auto r2 = QRectF(r.center() - QPointF(tex_sz.width()/2, tex_sz.height()/2), tex_sz);

        GLfloat x1 = static_cast<GLfloat>(static_cast<double>(r2.left()) / r.width());
        GLfloat x2 = static_cast<GLfloat>(static_cast<double>((r2.right()+1) / r.width()));
        GLfloat y1 = static_cast<GLfloat>(static_cast<double>(r2.top() / r.height()));
        GLfloat y2 = static_cast<GLfloat>(static_cast<double>((r2.bottom()+1) / r.height()));

        x1 = static_cast<GLfloat>(static_cast<double>(x1) * 2.0 - 1.0);
        x2 = static_cast<GLfloat>(static_cast<double>(x2) * 2.0 - 1.0);
        y1 = static_cast<GLfloat>(static_cast<double>(y1) * 2.0 - 1.0);
        y2 = static_cast<GLfloat>(static_cast<double>(y2) * 2.0 - 1.0);

        GLfloat vdata[] = {
            x1, y1, 0.0f, 1.0f,
            x2, y1, 1.0f, 1.0f,
            x2, y2, 1.0f, 0.0f,

            x1, y1, 0.0f, 1.0f,
            x2, y2, 1.0f, 0.0f,
            x1, y2, 0.0f, 0.0f
        };
        m_vbo.bind();
        m_vbo.allocate(vdata, sizeof(vdata));
        m_vbo.release();
    }

    void QtPlayerGLWidget::resizeGL(int nWidth, int nHeight)
    {

        updateMovieFbo();
        updateVbo();
        updateVboBlend();
        if (m_bDoRoundedClipping){
            updateVboCorners();
        }
        qInfo() << "GL resize" << nWidth << nHeight;
        QOpenGLWidget::resizeGL(nWidth, nHeight);
    }

    void QtPlayerGLWidget::toggleRoundedClip(bool bFalse)
    {
        m_bDoRoundedClipping = bFalse;
        makeCurrent();
        updateMovieFbo();
        update();
    }

    void QtPlayerGLWidget::initMember()
    {
        m_bPlaying = false;
        m_bInMiniMode= false;

        m_bDoRoundedClipping=true;
        m_pDarkTex = nullptr;
        m_pLightTex = nullptr;
        m_pGlProg = nullptr;
        m_pGlProgBlend  = nullptr;
        m_pFbo = nullptr;
        m_pGlProgBlendCorners = nullptr;
        m_pGlProgCorner = nullptr;
        m_pCornerMasks[0] = nullptr;
        m_pCornerMasks[1] = nullptr;
        m_pCornerMasks[2] = nullptr;
        m_pCornerMasks[3] = nullptr;
        m_pVideoTex = nullptr;
        m_bRawFormat = false;

        m_currWidth = rect().width();
        m_currHeight = rect().height();
    }

    /*not used yet*/

    void QtPlayerGLWidget::paintGL()
    {
        QOpenGLFunctions *pGLFunction = QOpenGLContext::currentContext()->functions();
        if (m_bPlaying && m_pVideoTex) {
            {
                pGLFunction->glEnable(GL_BLEND);
                pGLFunction->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                pGLFunction->glClearColor(0.0f, 0.0f, 0.0f, 1.0);
                pGLFunction->glClear(GL_COLOR_BUFFER_BIT);

                QOpenGLVertexArrayObject::Binder vaoBind(&m_vaoBlend);
                m_vaoBlend.bind();
                m_pGlProgBlend->bind();
                QOpenGLTexture *pGLTexture = m_pVideoTex;
                pGLTexture->bind();
                pGLFunction->glActiveTexture(GL_TEXTURE0);
                pGLFunction->glDrawArrays(GL_TRIANGLES, 0, 6);
                pGLTexture->release();
                m_pGlProgBlend->release();

                pGLFunction->glDisable(GL_BLEND);
            }

#ifdef __x86_64__
            QWidget *topWidget = topLevelWidget();
            if(topWidget && (topWidget->isFullScreen())) {                // 全屏状态播放时更新显示进度
                QString time_text = QTime::currentTime().toString("hh:mm");
                QRect rectTime = QRect(rect().width() - 90, 0, 90, 40);
                QPainter painter;
                painter.begin(this);
                QPen pen;
                pen.setColor(QColor(255, 255, 255, 255 * .4));
                painter.setPen(pen);
                QFontMetrics fm(font());
                auto fr = fm.boundingRect(time_text);
                fr.moveCenter(rectTime.center());
                //显示系统时间
                painter.drawText(fr,time_text);
                QPoint pos((rectTime.topLeft().x() + 20), rectTime.topLeft().y() + rectTime.height() - 5);
                int pert = qMin(m_pert * 10, 10.0);
                for (int i = 0; i < 10; i++) {                           // 显示影院视频播放进度
                    if (i >= pert) {
                        painter.fillRect(QRect(pos, QSize(3, 3)), QColor(255, 255, 255, 255 * .25));
                    } else {
                        painter.fillRect(QRect(pos, QSize(3, 3)), QColor(255, 255, 255, 255 * .5));
                    }
                    pos.rx() += 5;
                }
                QRect rectMovieTime = QRect(rect().width() - 175, 46, 175, 20);
                if(m_strPlayTime.isNull() || m_strPlayTime.isEmpty()) return;
                QPalette Palette;
                pen.setColor(Palette.color(QPalette::Text));
                if (m_bRawFormat) {
                    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
                        pen.setColor(QColor(0, 0, 0, 40));
                    } else {
                        pen.setColor(QColor(255, 255, 255, 40));
                    }
                }
                painter.setPen(pen);
                fr = fm.boundingRect(m_strPlayTime);
                fr.moveCenter(rectMovieTime.center());
                //显示影院视频播放时间与总时间
                painter.drawText(fr,m_strPlayTime);
                painter.end();
            }
#endif
        } else {
            pGLFunction->glEnable(GL_BLEND);
            pGLFunction->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            QColor color = QColor(37, 37, 37, 255);
            float fRation = 37.0f / 255.0f;
            if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()){
                color = QColor(252, 252, 252, 255);
                fRation = 252.0f / 255.0f;
            }
            pGLFunction->glClearColor(fRation, fRation, fRation, 1.0);
            pGLFunction->glClear(GL_COLOR_BUFFER_BIT);

            for(int i = 0;i < 2 ;i ++){
                {
                    QOpenGLVertexArrayObject::Binder vaoBind(&m_vao);
                    m_vbo.bind();
                    m_pGlProg->bind();
                    m_pGlProg->setUniformValue("bg", color);
                    prepareSplashImages();
                    QOpenGLTexture *pGLTexture;
                    m_pLightTex->setData(m_imgBgLight);
                    pGLTexture = m_pLightTex;
                    //和产品、ui商议深色主题下去除深色背景效果
//                    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
//                    if (themeType == DGuiApplicationHelper::DarkType) {
//                        pGLTexture = m_pDarkTex;
//                    }
                    pGLTexture->bind();
                    pGLFunction->glActiveTexture(GL_TEXTURE0);
                    pGLFunction->glDrawArrays(GL_TRIANGLES, 0, 6);
                    pGLTexture->release();
                    m_pGlProg->release();
                    m_vbo.release();
                }
            }

            if (m_bDoRoundedClipping) {
                pGLFunction->glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
                // blend corners
                QOpenGLVertexArrayObject::Binder vaoBind(&m_vaoCorner);

                for (int i = 0; i < 4; i++) {
                    m_pGlProgCorner->bind();
                    m_vboCorners[i].bind();

                    int vertexLoc = m_pGlProgCorner->attributeLocation("position");
                    int coordLoc = m_pGlProgCorner->attributeLocation("vTexCoord");
                    m_pGlProgCorner->enableAttributeArray(vertexLoc);
                    m_pGlProgCorner->setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 2, 6*sizeof(GLfloat));
                    m_pGlProgCorner->enableAttributeArray(coordLoc);
                    m_pGlProgCorner->setAttributeBuffer(coordLoc, GL_FLOAT, 2*sizeof(GLfloat), 2, 6*sizeof(GLfloat));
                    m_pGlProgCorner->setUniformValue("bg", color);
                    
                    pGLFunction->glActiveTexture(GL_TEXTURE0);
                    m_pCornerMasks[i]->bind();

                    pGLFunction->glDrawArrays(GL_TRIANGLES, 0, 6);

                    m_pCornerMasks[i]->release();
                    m_pGlProgCorner->release();
                    m_vboCorners[i].release();
                }
            }

            pGLFunction->glDisable(GL_BLEND);

        }
    }

    void QtPlayerGLWidget::setPlaying(bool bFalse)
    {
        if (m_bPlaying != bFalse) {
            m_bPlaying = bFalse;
            delete m_pVideoTex;
            m_pVideoTex = nullptr;
        }
        updateVbo();
        updateVboCorners();
        updateMovieFbo();
        update();
    }

    void QtPlayerGLWidget::setVideoTex(QImage image)
    {
        if(!m_pVideoTex){
            QFileInfo fi("/dev/mwv206_0");
            QFileInfo jmfi("/dev/jmgpu");
            if (fi.exists() || jmfi.exists()) {
                m_pVideoTex = new QOpenGLTexture(image, QOpenGLTexture::DontGenerateMipMaps);
            } else {
                m_pVideoTex = new QOpenGLTexture(image, QOpenGLTexture::GenerateMipMaps);
            }
        } else {
            m_pVideoTex->setData(image);
        }

        if(m_currWidth != image.width() || m_currHeight != image.height())
        {
            m_currWidth = image.width();
            m_currHeight = image.height();
            updateVboBlend();
        }
    }

#ifdef __x86_64__
    void QtPlayerGLWidget::updateMovieProgress(qint64 duration, qint64 pos)
    {
        if (pos > duration)
            pos = duration;
        m_pert = (qreal)pos / duration;//更新影院播放进度
        QString sCurtime = QString("%1 %2").arg(utils::Time2str(pos)).arg("/ ");
        QString stime = QString("%1").arg(utils::Time2str(duration));
        m_strPlayTime = sCurtime + stime;//更新影院当前播放时长
    }
#endif
    void QtPlayerGLWidget::setRawFormatFlag(bool bRawFormat)
    {
        m_bRawFormat = bRawFormat;
    }
}
