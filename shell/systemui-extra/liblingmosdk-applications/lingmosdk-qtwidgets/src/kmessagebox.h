/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KMESSAGEBOX_H
#define KMESSAGEBOX_H

#include <QDialog>
#include <QAbstractButton>
#include "gui_g.h"
#include "kdialog.h"
#include <QCheckBox>

namespace kdk {

/**
 *   @defgroup DialogBoxModule
 */
class KMessageBoxPrivate;

/**
 * @brief 继承自KDialog，提供一个可自定义设置按钮和图标的对话框，类似QMessageBox
 * @since 2.0
 */
class GUI_EXPORT KMessageBox : public KDialog
{
    Q_OBJECT
public:
    /**
     * @brief The Icon enum，icon的五种类型，与 QMessageDialogOptions::Icon 保持同步
     */
    enum Icon {
        // keep this in sync with QMessageDialogOptions::Icon
        NoIcon = 0,
        Information = 1,
        Warning = 2,
        Critical = 3,
        Question = 4,
    };
    Q_ENUM(Icon)

    /**
     * @brief The ButtonRole enum，可供设置的按钮role
     */
    enum ButtonRole {
        InvalidRole = -1,
        AcceptRole,
        RejectRole,
        DestructiveRole,
        ActionRole,
        HelpRole,
        YesRole,
        NoRole,
        ResetRole,
        ApplyRole,
        NRoles
    };

    /**
     * @brief The StandardButton enum，可供添加的按钮类型,与 QDialogButtonBox::StandardButton
     *        和 QPlatformDialogHelper::StandardButton 保持同步
     */
    enum StandardButton {
        // keep this in sync with QDialogButtonBox::StandardButton and QPlatformDialogHelper::StandardButton
        NoButton           = 0x00000000,
        Ok                 = 0x00000400,
        Save               = 0x00000800,
        SaveAll            = 0x00001000,
        Open               = 0x00002000,
        Yes                = 0x00004000,
        YesToAll           = 0x00008000,
        No                 = 0x00010000,
        NoToAll            = 0x00020000,
        Abort              = 0x00040000,
        Retry              = 0x00080000,
        Ignore             = 0x00100000,
        Close              = 0x00200000,
        Cancel             = 0x00400000,
        Discard            = 0x00800000,
        Help               = 0x01000000,
        Apply              = 0x02000000,
        Reset              = 0x04000000,
        RestoreDefaults    = 0x08000000,

        FirstButton        = Ok,                // internal
        LastButton         = RestoreDefaults,   // internal

        YesAll             = YesToAll,          // obsolete
        NoAll              = NoToAll,           // obsolete

        Default            = 0x00000100,        // obsolete
        Escape             = 0x00000200,        // obsolete
        FlagMask           = 0x00000300,        // obsolete
        ButtonMask         = ~FlagMask          // obsolete
    };
    typedef StandardButton Button;

    Q_DECLARE_FLAGS(StandardButtons, StandardButton)
    Q_FLAG(StandardButtons)

    KMessageBox(QWidget *parent = nullptr);
    ~KMessageBox();

    /**
     * @brief 自定义KMessageBox的提示图标
     * @param icon
     */
    void setCustomIcon(const QIcon&icon);

    /**
     * @brief 添加一个自定义按钮
     * @param button
     * @param role
     */
    void addButton(QAbstractButton *button, ButtonRole role);

    /**
     * @brief 添加设置好文本的按钮
     * @param text
     * @param role
     * @return
     */
    QPushButton* addButton(const QString &text, ButtonRole role);

    /**
     * @brief 添加一个标准按钮并且返回这个按钮
     * @param button
     * @return
     */
    QPushButton* addButton(StandardButton button);

    /**
     * @brief 移除一个按钮
     * @param button
     */
    void removeButton(QAbstractButton *button);

    /**
     * @brief 返回与标准按钮对应的指针，如果此消息框中不存在标准按钮，则返回0。
     * @param which
     * @return
     */
    QAbstractButton* button (StandardButton which) const;

    /**
     * @brief 返回已添加到消息框中的所有按钮的列表
     * @return
     */
    QList<QAbstractButton*> buttons() const;

    /**
     * @brief 返回指定按钮的按钮角色，如果按钮为0或尚未添加到消息框中，此函数将返回InvalidRole
     * @param button
     * @return
     */
    KMessageBox::ButtonRole buttonRole(QAbstractButton *button) const;

    /**
     * @brief 返回KMessageBox中显示的复选框
     * @return
     */
    QCheckBox* checkBox() const;

    /**
     * @brief 设置KMessageBox显示的复选框，未设置则为0
     * @param cb
     */
    void setCheckBox(QCheckBox *cb);

    /**
     * @brief 获取KMessageBox的文本
     * @return
     */
    QString text() const;

    /**
     * @brief 设置KMessageBox的文本
     * @param text
     */
    void setText (const QString& text);

    /**
     * @brief 获取KMessageBox信息性文本的描述
     * @return
     */
    QString informativeText() const;

    /**
     * @brief 设置KMessageBox信息性文本的描述
     * @param text
     */
    void setInformativeText(const QString &text);

    /**
     * @brief 获取KMessageBox的图标
     * @return
     */
    Icon icon() const;

    /**
     * @brief 设置KMessageBox的图标
     * @param icon
     */
    void setIcon(Icon icon);

    /**
     * @brief 返回当前KMessageBox的icon
     * @return
     */
    QPixmap iconPixmap() const;

    /**
     * @brief 设置当前KMessageBox的icon
     * @param pixmap
     */
    void setIconPixmap(const QPixmap &pixmap);

    /**
     * @brief KMessageBox中标准按钮的集合
     * @return
     */
    KMessageBox::StandardButtons standardButtons() const;

    /**
     * @brief 设置多个标准按钮
     * @param buttons
     */
    void setStandardButtons(KMessageBox::StandardButtons buttons);

    /**
     * @brief 返回与给定按钮对应的标准按钮枚举值，如果给定按钮不是标准按钮，则返回NoButton
     * @param button
     * @return
     */
    KMessageBox::StandardButton standardButton(QAbstractButton *button) const;

    /**
     * @brief 返回KMessageBox的默认按钮
     * @return
     */
    QPushButton* defaultButton() const;

    /**
     * @brief 设置KMessageBox的默认按钮
     * @param button
     */
    void setDefaultButton(QPushButton *button);

    /**
     * @brief 设置KMessageBox的默认按钮
     * @param button
     */
    void setDefaultButton(KMessageBox::StandardButton button);

    /**
     * @brief 返回被点击的按钮
     * @return
     */
    QAbstractButton* clickedButton() const;

    /**
     * @brief 返回用于标准图标的pixmap。
     * @param icon
     * @return
     */
    static QPixmap standardIcon(Icon icon);

    /**
     * @brief 打开带有给定标题和文本的信息消息框。对话框最多可以有三个按钮
     * @param parent
     * @param title
     * @param text
     * @param buttons
     * @param defaultButton
     * @return
     */
    static StandardButton information(QWidget *parent, const QString &title,
         const QString &text, StandardButtons buttons = Ok,
         StandardButton defaultButton = NoButton);

    /**
     * @brief 打开一个带有给定标题和文本的问题消息框
     * @param parent
     * @param title
     * @param text
     * @param buttons
     * @param defaultButton
     * @return
     */
    static StandardButton question(QWidget *parent, const QString &title,
         const QString &text, StandardButtons buttons = StandardButtons(Yes | No),
         StandardButton defaultButton = NoButton);
    /**
     * @brief 打开一个带有给定标题和文本的警告消息框
     * @param parent
     * @param title
     * @param text
     * @param buttons
     * @param defaultButton
     * @return
     */
    static StandardButton warning(QWidget *parent, const QString &title,
         const QString &text, StandardButtons buttons = Ok,
         StandardButton defaultButton = NoButton);
    /**
     * @brief 用给定的标题和文本打开一个关键信息框
     * @param parent
     * @param title
     * @param text
     * @param buttons
     * @param defaultButton
     * @return
     */
    static StandardButton critical(QWidget *parent, const QString &title,
         const QString &text, StandardButtons buttons = Ok,
         StandardButton defaultButton = NoButton);
    /**
     * @brief 用给定的标题和文本打开一个成功消息框
     * @param parent
     * @param title
     * @param text
     * @param buttons
     * @param defaultButton
     * @return
     */
    static StandardButton success(QWidget *parent, const QString &title,
         const QString &text, StandardButtons buttons = Ok,
         StandardButton defaultButton = NoButton);


Q_SIGNALS:
    void buttonClicked(QAbstractButton *button);

protected:
    bool event(QEvent *e) override;

private:
    Q_DECLARE_PRIVATE(KMessageBox)
    KMessageBoxPrivate* const d_ptr;
};

}
#endif // KMESSAGEBOX_H
