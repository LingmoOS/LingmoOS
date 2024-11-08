/*
 * KWin Style LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QIcon>
#include <QDialog>
#include <QMessageBox>
#include <QAbstractButton>
#include <qpa/qplatformdialoghelper.h>
#include <QMenu>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include "private/qdialog_p.h"
#include <QtWidgets/qdialogbuttonbox.h>

class QLabel;
class QAbstractButton;
class MessageBoxHelper;
class MessageBoxPrivate;
class MessageBoxOptionsPrivate;

typedef QMessageBox::Icon               Icon;
typedef QMessageBox::StandardButtons    StandardButtons;

class MessageBox : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(Icon mIcon READ icon WRITE setIcon)
    Q_PROPERTY(QString mText READ text WRITE setText)
    Q_PROPERTY(Qt::TextFormat textFormat READ textFormat WRITE setTextFormat)
    Q_PROPERTY(QPixmap mIconPixmap READ iconPixmap WRITE setIconPixmap)
    Q_PROPERTY(QString mInformativeText READ informativeText WRITE setInformativeText)
    Q_PROPERTY(StandardButtons mStandardButtons READ standardButtons WRITE setStandardButtons)
    Q_PROPERTY(Qt::TextInteractionFlags textInteractionFlags READ textInteractionFlags WRITE setTextInteractionFlags)
    friend class MessageBoxHelper;
public:
    explicit MessageBox(QWidget *parent = nullptr);
    ~MessageBox();

    void setCheckBox(QCheckBox *cb);
    QCheckBox* checkBox() const;

    QMessageBox::Icon icon ();
    void setIcon (QMessageBox::Icon icon);

    QPixmap iconPixmap() const;
    void setIconPixmap(const QPixmap &pixmap);

    QString text();
    void setText (const QString& text);

    QString informativeText() const;
    void setInformativeText(const QString &text);

    QString detailedText() const;
    void setDetailedText(const QString &text);

    QString buttonText(int button) const;
    void setButtonText(int button, const QString &text);

    Qt::TextFormat textFormat() const;
    void setTextFormat(Qt::TextFormat format);

    void setTextInteractionFlags(Qt::TextInteractionFlags flags);
    Qt::TextInteractionFlags textInteractionFlags() const;

    void addButton(QAbstractButton *button, QMessageBox::ButtonRole role);
    QPushButton* addButton(const QString &text, QMessageBox::ButtonRole role);
    QPushButton* addButton(QMessageBox::StandardButton button);
    void removeButton(QAbstractButton *button);

    QAbstractButton* button (QMessageBox::StandardButton which) const;

    QList<QAbstractButton*> buttons() const;
    QMessageBox::ButtonRole buttonRole(QAbstractButton *button) const;

    QMessageBox::StandardButtons standardButtons() const;
    void setStandardButtons(QMessageBox::StandardButtons buttons);
    QMessageBox::StandardButton standardButton(QAbstractButton *button) const;

    QPushButton* defaultButton() const;
    void setDefaultButton(QPushButton *button);
    void setDefaultButton(QMessageBox::StandardButton button);

    QAbstractButton* escapeButton() const;
    void setEscapeButton(QAbstractButton *button);
    void setEscapeButton(QMessageBox::StandardButton button);

    QAbstractButton* clickedButton() const;

    void setWindowTitle(const QString &title);
    void setWindowModality(Qt::WindowModality windowModality);

    static QPixmap standardIcon(QMessageBox::Icon icon);

    void setWindowIcon(const QIcon &icon);

protected:
    bool event(QEvent *e) override;
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent (QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initHelper(QPlatformMessageDialogHelper*);
    void setuplayout();

public slots:
    void tableModeChanged(bool isTableMode);

Q_SIGNALS:
    void buttonClicked(QAbstractButton* button);

private:
    Q_DISABLE_COPY(MessageBox)
    Q_DECLARE_PRIVATE(MessageBox)

    Q_PRIVATE_SLOT(d_func(), void _q_buttonClicked(QAbstractButton*))
    Q_PRIVATE_SLOT(d_func(), void _q_clicked(QPlatformDialogHelper::StandardButton, QPlatformDialogHelper::ButtonRole))
};

class MessageBoxHelper : public QPlatformMessageDialogHelper
{
    Q_OBJECT
public:
    explicit MessageBoxHelper();
    ~MessageBoxHelper() override;

    virtual void exec() override;
    virtual void hide() override;
    virtual bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent) override;

Q_SIGNALS:
    void clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role);

private:
    void initDialog (Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent);

private:
    MessageBox*                                 mMessageBox = nullptr;
};



class TextEdit : public QTextEdit
{
public:
    TextEdit(QWidget *parent=0) : QTextEdit(parent) { }
    void contextMenuEvent(QContextMenuEvent * e) override
    {
        QMenu *menu = createStandardContextMenu();
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->popup(e->globalPos());
    }
};


class MessageBoxPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(MessageBox)

public:
    MessageBoxPrivate ();
    ~MessageBoxPrivate ();

    void init (const QString &title = QString(), const QString &text = QString());

    void setupLayout ();

    void _q_buttonClicked(QAbstractButton*);
    void _q_clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role);

    void setClickedButton(QAbstractButton *button);
    QAbstractButton* findButton(int button0, int button1, int button2, int flags);
    void addOldButtons(int button0, int button1, int button2);

    QAbstractButton *abstractButtonForId(int id) const;
    int execReturnCode(QAbstractButton *button);

    int dialogCodeForButton(QAbstractButton *button) const;

    void updateSize();
    int layoutMinimumWidth();
    void detectEscapeButton();

    static QMessageBox::StandardButton newButton(int button);
    static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text, int button0, int button1, int button2);
    static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text, const QString &btn0Text, const QString &btn1Text, const QString &btn2Text, int defBtnNum, int espBtnNum);

    static QPixmap standardIcon(QMessageBox::Icon icon, MessageBox *mb);
    static QMessageBox::StandardButton showNewMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString& title, const QString& text, QMessageBox::StandardButtons btn, QMessageBox::StandardButton defBtn);

private:
    void initHelper(QPlatformDialogHelper*) override;
    void helperPrepareShow(QPlatformDialogHelper*) override;
    void helperDone(QDialog::DialogCode, QPlatformDialogHelper*) override;

public:
    QLabel*                                             mLabel = nullptr;
    QLabel*                                             mInformativeLabel = nullptr;
    TextEdit*                                           mDetail = nullptr;                    // qt 显示 label 暂定使用富文本框
    QCheckBox*                                          mCheckbox = nullptr;                  // qt checkbox
    QLabel*                                             mIconLabel = nullptr;                 // qt 显示图标
    QDialogButtonBox*                                   mButtonBox = nullptr;                 // qt 按钮框
    QPushButton*                                        mDetailButton = nullptr;              // 详细情况按钮
    QPushButton*                                        mCloseButtton = nullptr;              //标题栏关闭按钮
    QLabel*                                             mTitleText = nullptr;                 //标题文本
    QLabel*                                             mTitleIcon = nullptr;                 //标题icon

    QByteArray                                          mMemberToDisconnectOnClose;
    QByteArray                                          mSignalToDisconnectOnClose;
    QPointer<QObject>                                   mReceiverToDisconnectOnClose;


    QString                                             mTipString;                 // 原始的需要展示的文本
    QMessageBox::Icon                                   mIcon;
    QList<QAbstractButton*>                             mCustomButtonList;          // 自定义按钮
    QAbstractButton*                                    mEscapeButton = nullptr;
    QPushButton*                                        mDefaultButton = nullptr;
    QAbstractButton*                                    mClickedButton = nullptr;             // 复选框按钮

    bool                                                mCompatMode;
    bool                                                mShowDetail;                // 是否显示详细信息
    bool                                                mAutoAddOkButton;

//    QTextEdit* informativeTextEdit;
    QAbstractButton*                                    mDetectedEscapeButton = nullptr;
    QSharedPointer<QMessageDialogOptions>               mOptions;

private:
    int                             mRadius = 0;
    int                             mIconSize = 24;
};

#endif // MESSAGEBOX_H

#define QT_REQUIRE_VERSION(argc, argv, str) { QString s = QString::fromLatin1(str);\
QString sq = QString::fromLatin1(qVersion()); \
if ((sq.section(QChar::fromLatin1('.'),0,0).toInt()<<16)+\
(sq.section(QChar::fromLatin1('.'),1,1).toInt()<<8)+\
sq.section(QChar::fromLatin1('.'),2,2).toInt()<(s.section(QChar::fromLatin1('.'),0,0).toInt()<<16)+\
(s.section(QChar::fromLatin1('.'),1,1).toInt()<<8)+\
s.section(QChar::fromLatin1('.'),2,2).toInt()) { \
if (!qApp){ \
    new QApplication(argc,argv); \
} \
QString s = QApplication::tr("Executable '%1' requires Qt "\
 "%2, found Qt %3.").arg(qAppName()).arg(QString::fromLatin1(\
str)).arg(QString::fromLatin1(qVersion())); QMessageBox::critical(0, QApplication::tr(\
"Incompatible Qt Library Error"), s, QMessageBox::Abort, 0); qFatal("%s", s.toLatin1().data()); }}
