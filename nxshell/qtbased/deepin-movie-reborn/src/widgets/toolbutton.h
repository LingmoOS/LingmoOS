// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_TOOLBUTTON_H
#define _DMR_TOOLBUTTON_H

//#include <QtWidgets>
#include <QWidget>
#include <QWheelEvent>
#include <dimagebutton.h>
#include <DIconButton>
#include <QGraphicsDropShadowEffect>
#include <DFontSizeManager>
#include <DPalette>
#include <DApplicationHelper>
#include <QGuiApplication>
#include <QPainterPath>
#include <QThread>
#include <DArrowRectangle>
#include <DButtonBox>
#include <QDBusInterface>
#include <QDBusReply>
#include <QPushButton>
#include <DToolButton>

DWIDGET_USE_NAMESPACE

namespace dmr {

enum ThemeTYpe {
    lightTheme,
    darkTheme,
    defaultTheme
};

class ButtonBoxButton: public DButtonBoxButton
{
    Q_OBJECT
public:
    explicit ButtonBoxButton(const QString &text, QWidget *parent = nullptr)
        : DButtonBoxButton(text, parent)
    {
        installEventFilter(this);
    };

signals:
    void entered();
    void leaved();
    void sigUnsupported();
protected:
    void enterEvent(QEvent *ev) override
    {
        emit entered();
    };
    void leaveEvent(QEvent *ev) override
    {
        emit leaved();
    };
    bool eventFilter(QObject *obj, QEvent *e)
    {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(e);
        if(!isEnabled() && pMouseEvent)                  // 音量按钮不能使用时需要给出提示
        {
            if(pMouseEvent->type() == QEvent::MouseButtonPress) {
               emit sigUnsupported();
            }
            return false;
        }

        return QObject::eventFilter(obj, e);
    };
};

class ButtonToolTip : public DArrowRectangle
{
    Q_OBJECT
public:
    explicit ButtonToolTip(QWidget *parent = nullptr)
        : DArrowRectangle(DArrowRectangle::ArrowBottom, DArrowRectangle::FloatWidget, parent)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setAttribute(Qt::WA_TranslucentBackground);
        resetSize();
        connect(qApp, &QGuiApplication::fontChanged, this, [ = ] {
            resetSize();
        });
        QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect(this);
        bodyShadow->setBlurRadius(10.0);
        bodyShadow->setColor(QColor(0, 0, 0, 0.1 * 255));
        bodyShadow->setOffset(0, 2.0);

        setArrowWidth(1);
        setArrowHeight(1);
        hide();
    }
    virtual ~ButtonToolTip() {};
    void setText(const QString &strText)
    {
        m_strText = strText;
    }
    void changeTheme(ThemeTYpe themeType = defaultTheme)
    {
        m_themeType = themeType;
        update();
    }
    void show()
    {
        if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
            changeTheme(lightTheme);
        } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
            changeTheme(darkTheme);
        } else {
            changeTheme(lightTheme);
        }
        resetSize();
        QWidget::show();
    }
protected:
    virtual void resizeEvent(QResizeEvent *ev)
    {
        resetSize();
        update();
        return QWidget::resizeEvent(ev);
    }
    virtual void paintEvent(QPaintEvent *ev)
    {
        QPainter pt(this);
        pt.setRenderHint(QPainter::Antialiasing);

        if (lightTheme == m_themeType) {
            pt.setPen(QColor(0, 0, 0, 10));
            pt.setBrush(QBrush(QColor(247, 247, 247, 220)));
        } else if (darkTheme == m_themeType) {
            pt.setPen(QColor(255, 255, 255, 10));
            pt.setBrush(QBrush(QColor(42, 42, 42, 220)));
        } else {
            pt.setPen(QColor(0, 0, 0, 10));
            pt.setBrush(QBrush(QColor(247, 247, 247, 220)));
        }

        QRect rect = this->rect();
        rect.setWidth(rect.width() - 1);
        rect.setHeight(rect.height() - 1);
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 8, 8);
        pt.drawPath(painterPath);

        DPalette pal_text = DApplicationHelper::instance()->palette(this);
        pal_text.setBrush(DPalette::Text, pal_text.color(DPalette::ToolTipText));
        this->setPalette(pal_text);
        pt.setPen(pal_text.color(DPalette::ToolTipText));


        DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
        QFontMetrics fm(font);
        auto w = fm.boundingRect(m_strText).width();
        auto h = fm.height();
        pt.drawText((rect.width() - w) / 2, (rect.height() + h / 2) / 2, m_strText);
    }

    void resetSize()
    {
        DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
        QFontMetrics fm(font);
        auto w = fm.boundingRect(m_strText).width();
        auto h = fm.height();
        resize(w + 14, h + 8);
    }

private:
    QString m_strText = nullptr;
    ThemeTYpe m_themeType;
};

class ToolTip: public QFrame
{
    Q_OBJECT
public:
    explicit ToolTip(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowFlags(windowFlags() | Qt::ToolTip);
        setAttribute(Qt::WA_TranslucentBackground);
        resetSize();
        connect(qApp, &QGuiApplication::fontChanged, this, [ = ] {
            resetSize();
        });

        auto *bodyShadow = new QGraphicsDropShadowEffect(this);
        bodyShadow->setBlurRadius(10.0);
        bodyShadow->setColor(QColor(0, 0, 0, int(0.1 * 255)));
        bodyShadow->setOffset(0, 2.0);
//        this->setGraphicsEffect(bodyShadow);

        m_pWMDBus = new QDBusInterface("com.deepin.WMSwitcher","/com/deepin/WMSwitcher","com.deepin.WMSwitcher",QDBusConnection::sessionBus());
        QDBusReply<QString> reply = m_pWMDBus->call("CurrentWM");
        m_bIsWM = reply.value().contains("deepin wm");
        connect(m_pWMDBus, SIGNAL(WMChanged(QString)), this, SLOT(slotWMChanged(QString)));
    }
    virtual ~ToolTip() {}

    void setText(const QString &strText)
    {
        m_strText = strText;
        resetSize();
    }

    void changeTheme(ThemeTYpe themeType = defaultTheme)
    {
        m_themeType = themeType;
        update();
    }

public slots:
    void slotWMChanged(QString msg)
    {
        if (msg.contains("deepin metacity")) {
            m_bIsWM = false;
        } else {
            m_bIsWM = true;
        }
    }


protected:
    virtual void paintEvent(QPaintEvent *)
    {
        QPainter pt(this);
        pt.setRenderHint(QPainter::Antialiasing);

        int transparency = 220;
        if (!m_bIsWM) {
            transparency = 255;
        }
        if (lightTheme == m_themeType) {
            pt.setPen(QColor(0, 0, 0, 10));
            pt.setBrush(QBrush(QColor(247, 247, 247, transparency)));
        } else if (darkTheme == m_themeType) {
            pt.setPen(QColor(255, 255, 255, 10));
            pt.setBrush(QBrush(QColor(42, 42, 42, transparency)));
        } else {
            pt.setPen(QColor(0, 0, 0, 10));
            pt.setBrush(QBrush(QColor(247, 247, 247, transparency)));
        }

        QRect rect = this->rect();
        QPainterPath painterPath;
        if (m_bIsWM) {
            rect.setWidth(rect.width() - 1);
            rect.setHeight(rect.height() - 1);
            painterPath.addRoundedRect(rect, 8, 8);
        } else {
            painterPath.addRoundedRect(rect, 0, 0);
        }
        pt.drawPath(painterPath);

        DPalette pal_text = DApplicationHelper::instance()->palette(this);
        pal_text.setBrush(DPalette::Text, pal_text.color(DPalette::ToolTipText));
        this->setPalette(pal_text);
        pt.setPen(pal_text.color(DPalette::ToolTipText));

        DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
        QFontMetrics fm(font);
        auto w = fm.boundingRect(m_strText).width();
        auto h = fm.height();
        pt.drawText((rect.width() - w) / 2, (rect.height() + h / 2) / 2, m_strText);
    }

    virtual void resizeEvent(QResizeEvent *ev)
    {
        resetSize();
        update();
        return QWidget::resizeEvent(ev);
    }

private:
    void resetSize()
    {
        DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
        QFontMetrics fm(font);
        auto w = fm.boundingRect(m_strText).width();
        auto h = fm.height();
        resize(w + 14, h + 8);
    }

private:
    bool m_bTheme;
    ThemeTYpe m_themeType;
    QString m_strText = nullptr;
    QDBusInterface* m_pWMDBus {nullptr};
    bool m_bIsWM {false};
};

class ToolButton: public DToolButton
{
    Q_OBJECT
public:
    explicit ToolButton(QWidget *parent = nullptr): DToolButton(parent) {}
    virtual ~ToolButton() {}

    void initToolTip()
    {
        if (nullptr == m_pToolTip) {
            m_pToolTip = new ToolTip;
        }
    }

    void showToolTip()
    {
        QPoint pos = this->parentWidget()->mapToGlobal(this->pos());
        pos.rx() = pos.x() + (this->width() - m_pToolTip->width()) / 2;
        pos.ry() = pos.y() - 40;

        if (nullptr != m_pToolTip) {
            m_pToolTip->move(pos);
            QThread::msleep(10);
            m_pToolTip->show();
        }
    }

    void hideToolTip()
    {
        if (nullptr != m_pToolTip) {
            QThread::msleep(10);
            m_pToolTip->hide();
        }
    }

    void changeTheme(ThemeTYpe themeType = defaultTheme)
    {
        m_pToolTip->changeTheme(themeType);
    }

    void setTooTipText(const QString &strTip)
    {
        m_pToolTip->setText(strTip);
    }

signals:
    void entered();
    void leaved();

protected:
    void enterEvent(QEvent *) override
    {
        emit entered();
    }
    void leaveEvent(QEvent *) override
    {
        emit leaved();
    }

private:
    ToolTip *m_pToolTip {nullptr};
};

class VolumeButton: public QPushButton
{
    Q_OBJECT
public:
    explicit VolumeButton(QWidget *parent = 0);
    void changeStyle();
    void setVolume(int nVolume);
    void setMute(bool bMute);
    void setButtonEnable(bool bFlag);
    void setIcon(const QIcon&);

signals:
    void entered();
    void leaved();
    void requestVolumeUp();
    void requestVolumeDown();
    /**
      * @brief 功能不支持信号
      */
    void sigUnsupported();

protected:
    void enterEvent(QEvent *ev) override;
    void leaveEvent(QEvent *ev) override;
    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *wev) override;
    void focusOutEvent(QFocusEvent *ev) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    QString _name;
    int m_nVolume;
    bool m_bMute;
    QIcon m_icon;
};

}

#endif /* ifndef _DMR_TOOLBUTTON_H */

