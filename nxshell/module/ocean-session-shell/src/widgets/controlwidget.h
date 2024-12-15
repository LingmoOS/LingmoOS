// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <dtkwidget_global.h>
#include <dtkcore_global.h>
#include "userinfo.h"

#include <DFloatingButton>
#include <DBlurEffectWidget>

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>

namespace dss {
namespace module {
class BaseModuleInterface;
}
} // namespace dss

DWIDGET_USE_NAMESPACE

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

class MediaWidget;
class QHBoxLayout;
class QPropertyAnimation;
class QLabel;
class QMenu;
class SessionBaseModel;
class KBLayoutListView;
class TipsWidget;
class TipContentWidget;
class SessionPopupWidget;
class UserListPopupWidget;
class RoundPopupWidget;

const int BlurRadius = 15;
const int BlurTransparency = 70;

class FloatingButton : public DFloatingButton
{
    Q_OBJECT
public:
    explicit FloatingButton(QWidget *parent = nullptr)
        : DFloatingButton(parent) {
        installEventFilter(this);
    }
    explicit FloatingButton(QStyle::StandardPixmap iconType = static_cast<QStyle::StandardPixmap>(-1), QWidget *parent = nullptr)
        : DFloatingButton(iconType, parent) {
        installEventFilter(this);
    }
    explicit FloatingButton(DStyle::StandardPixmap iconType = static_cast<DStyle::StandardPixmap>(-1), QWidget *parent = nullptr)
        : DFloatingButton(iconType, parent) {
        installEventFilter(this);
    }
    explicit FloatingButton(const QString &text, QWidget *parent = nullptr)
        : DFloatingButton(text, parent) {
        installEventFilter(this);
    }
    FloatingButton(const QIcon& icon, const QString &text = QString(), QWidget *parent = nullptr)
        : DFloatingButton(icon, text, parent) {
        installEventFilter(this);
    }

    void setTipText(const QString &tipText) {
        m_tipText = tipText;
    }
    QString tipText() const {
        return m_tipText;
    }

    void setPluginItem(QWidget *item) { m_item = item; }
    QWidget *pluginItem() const { return m_item; }

  Q_SIGNALS:
    void requestShowMenu();
    void requestShowTips();
    void requestHideTips();

protected:
    bool eventFilter(QObject *watch, QEvent *event) override;

private:
  void showCustomWidget() const;
  QString m_tipText;
  QWidget *m_item{nullptr};
};
class ControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlWidget(const SessionBaseModel *model, QWidget *parent = nullptr);

    void setModel(const SessionBaseModel *model);
    void setUser(std::shared_ptr<User> user);

    void initKeyboardLayoutList();

signals:
    void requestSwitchUser(std::shared_ptr<User> user);
    void requestShutdown();
    void requestSwitchSession(const QString &session);
    void requestTogglePopup(QPoint globalPos, QWidget *popup);
    void requestHidePopup();

public slots:
    void addModule(dss::module::BaseModuleInterface *module);
    void setUserSwitchEnable(const bool visible);
    void setSessionSwitchEnable(const bool visible);
    void chooseToSession(const QString &session);
    void setKBLayoutVisible();
    void setKeyboardType(const QString& str);
    void setKeyboardList(const QStringList& str);
    void onItemClicked(const QString& str);
    void showSessionPopup();
    void showUserListPopupWidget();

protected:
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnect();
    void updateLayout();
    void updateTapOrder();
    int focusedBtnIndex();
    void toggleButtonPopup(const FloatingButton *button, QWidget *popup);

private slots:
    void showInfoTips();
    void hideInfoTips();

private:
    QList<FloatingButton *> m_showedBtnList;

    QHBoxLayout *m_mainLayout;
    FloatingButton *m_switchUserBtn;
    FloatingButton *m_powerBtn;
    FloatingButton *m_sessionBtn;
    FloatingButton *m_keyboardBtn;         // 键盘布局按钮

    std::shared_ptr<User> m_curUser;
    const SessionBaseModel *m_model;
    QList<QMetaObject::Connection> m_connectionList;

    QMenu *m_contextMenu;
    QMap<QString, QWidget *> m_modules;

    TipContentWidget *m_tipContentWidget;       // 显示按钮文字tip
    TipsWidget *m_tipsWidget;                   // 显示插件提供widget tip

    RoundPopupWidget *m_roundPopupWidget;       // 圆角弹窗
    KBLayoutListView *m_kbLayoutListView;       // 键盘布局列表
    SessionPopupWidget *m_sessionPopupWidget;   // session 列表
    UserListPopupWidget *m_userListPopupWidget; // 用户列表
};

#endif // CONTROLWIDGET_H
