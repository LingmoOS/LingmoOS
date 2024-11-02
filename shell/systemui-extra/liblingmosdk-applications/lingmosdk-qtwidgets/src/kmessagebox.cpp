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

#include "kmessagebox.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QTextDocument>
#include <QStyle>
#include <QMessageBox>
#include <QApplication>
#include <QStyleOption>
#include <QPainter>
#include <QScreen>
#include <QVBoxLayout>
#include <QCheckBox>
#include "themeController.h"

namespace kdk {

enum Button { Old_Ok = 1, Old_Cancel = 2, Old_Yes = 3, Old_No = 4, Old_Abort = 5, Old_Retry = 6,
              Old_Ignore = 7, Old_YesAll = 8, Old_NoAll = 9, Old_ButtonMask = 0xFF,
              NewButtonMask = 0xFFFFFC00 };

static QString iconName="";

class KMessageBoxPrivate: public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KMessageBox)

public:
    KMessageBoxPrivate(KMessageBox*parent);

    void init(const QString &title = QString(), const QString &text = QString());
    static QPixmap standardIcon(KMessageBox::Icon icon, KMessageBox *mb);
    void setupLayout();
    void updateSize();
    void setClickedButton(QAbstractButton *button);
    int execReturnCode(QAbstractButton *button);
    int dialogCodeForButton(QAbstractButton *button) const;
    void addOldButtons(int button0, int button1, int button2);
    QAbstractButton *findButton(int button0, int button1, int button2, int flags);

public Q_SLOTS:
    void _q_buttonClicked(QAbstractButton *);

protected:
    void changeTheme();

private:
    KMessageBox *q_ptr;
    QLabel *iconLabel;
    QLabel *label;
    QLabel *informativeLabel;
    QDialogButtonBox *buttonBox;
    QList<QAbstractButton *> customButtonList;
    KMessageBox::Icon icon;
    QAbstractButton *clickedButton;
    QCheckBox *checkbox;
    QPushButton *defaultButton;
    bool autoAddOkButton;
    bool compatMode;
    QString m_iconName;
};

KMessageBox::KMessageBox(QWidget *parent)
    : KDialog(parent),
      d_ptr(new KMessageBoxPrivate(this))
{
    Q_D(KMessageBox);
    d->init();
    d->changeTheme();
    connect(d->m_gsetting,&QGSettings::changed,this,[=](){d->changeTheme();});
}

KMessageBox::~KMessageBox()
{
}

void KMessageBox::setCustomIcon(const QIcon &icon)
{
    Q_D(KMessageBox);
    d->m_iconName = icon.name();
    this->setIconPixmap(icon.pixmap(24,24));
}

void KMessageBox::addButton(QAbstractButton *button, ButtonRole role)
{
    Q_D(KMessageBox);
    if (!button)
        return;
    removeButton(button);
    d->buttonBox->addButton(button, (QDialogButtonBox::ButtonRole)role);
    d->customButtonList.append(button);
    d->autoAddOkButton = false;
}

QPushButton *KMessageBox::addButton(const QString &text, ButtonRole role)
{
    Q_D(KMessageBox);
    QPushButton *pushButton = new QPushButton(text);
    addButton(pushButton, role);
    return pushButton;
}

QPushButton *KMessageBox::addButton(StandardButton button)
{
    Q_D(KMessageBox);
    QPushButton *pushButton = d->buttonBox->addButton((QDialogButtonBox::StandardButton)button);
    pushButton->setIcon(QIcon());
    if (pushButton)
        d->autoAddOkButton = false;
    return pushButton;
}

void KMessageBox::removeButton(QAbstractButton *button)
{
    Q_D(KMessageBox);
    d->customButtonList.removeAll(button);
    if (d->defaultButton == button)
        d->defaultButton = 0;
    d->buttonBox->removeButton(button);
}

QAbstractButton *KMessageBox::button(KMessageBox::StandardButton which) const
{
    Q_D(const KMessageBox);
    return d->buttonBox->button(QDialogButtonBox::StandardButton(which));
}

QList<QAbstractButton *> KMessageBox::buttons() const
{
    Q_D(const KMessageBox);
    return d->buttonBox->buttons();
}

KMessageBox::ButtonRole KMessageBox::buttonRole(QAbstractButton *button) const
{
    Q_D(const KMessageBox);
    return KMessageBox::ButtonRole(d->buttonBox->buttonRole(button));
}

QCheckBox *KMessageBox::checkBox() const
{
    Q_D(const KMessageBox);
    return d->checkbox;
}

void KMessageBox::setCheckBox(QCheckBox *cb)
{
    Q_D(KMessageBox);

    if (cb == d->checkbox)
        return;

    if (d->checkbox) {
        d->checkbox->hide();
        layout()->removeWidget(d->checkbox);
        if (d->checkbox->parentWidget() == this) {
            d->checkbox->setParent(0);
            d->checkbox->deleteLater();
        }
    }
    d->checkbox = cb;
    if (d->checkbox) {
        QSizePolicy sp = d->checkbox->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        d->checkbox->setSizePolicy(sp);
    }
    d->setupLayout();
}

QString KMessageBox::text() const
{
    Q_D(const KMessageBox);
    return d->label->text();
}

void KMessageBox::setText(const QString &text)
{
    Q_D(KMessageBox);
    d->label->setText(text);
    d->label->setWordWrap(d->label->textFormat() == Qt::RichText
        || (d->label->textFormat() == Qt::AutoText && Qt::mightBeRichText(text)));
    d->updateSize();
}

QString KMessageBox::informativeText() const
{
    Q_D(const KMessageBox);
    return d->informativeLabel ? d->informativeLabel->text() : QString();
}

void KMessageBox::setInformativeText(const QString &text)
{
    Q_D(KMessageBox);
    if (text.isEmpty()) {
        if (d->informativeLabel) {
            d->informativeLabel->hide();
            d->informativeLabel->deleteLater();
        }
        d->informativeLabel = 0;
    } else {
        if (!d->informativeLabel) {
            QLabel *label = new QLabel;
            label->setObjectName(QLatin1String("qt_msgbox_informativelabel"));
            label->setTextInteractionFlags(Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, this)));
            label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            label->setOpenExternalLinks(true);
            label->setWordWrap(true);
            d->informativeLabel = label;
        }
        d->informativeLabel->setText(text);
    }
        d->setupLayout();
}

KMessageBox::Icon KMessageBox::icon() const
{
    Q_D(const KMessageBox);
    return d->icon;
}

void KMessageBox::setIcon(Icon icon)
{
    Q_D(KMessageBox);
    setIconPixmap(KMessageBoxPrivate::standardIcon((KMessageBox::Icon)icon,this));
    d->icon = icon;
}

QPixmap KMessageBox::iconPixmap() const
{
    Q_D(const KMessageBox);
    if (d->iconLabel && d->iconLabel->pixmap())
        return *d->iconLabel->pixmap();
    return QPixmap();
}

void KMessageBox::setIconPixmap(const QPixmap &pixmap)
{
    Q_D(KMessageBox);
    d->iconLabel->setPixmap(pixmap);
    d->icon = NoIcon;
    d->setupLayout();
}

KMessageBox::StandardButtons KMessageBox::standardButtons() const
{
    Q_D(const KMessageBox);
    return KMessageBox::StandardButtons(int(d->buttonBox->standardButtons()));
}

void KMessageBox::setStandardButtons(KMessageBox::StandardButtons buttons)
{
    Q_D(KMessageBox);
    d->buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));

    QList<QAbstractButton *> buttonList = d->buttonBox->buttons();
    if (!buttonList.contains(d->defaultButton))
       d->defaultButton = 0;
    d->autoAddOkButton = false;
}

KMessageBox::StandardButton KMessageBox::standardButton(QAbstractButton *button) const
{
    Q_D(const KMessageBox);
    return (KMessageBox::StandardButton)d->buttonBox->standardButton(button);
}

QPushButton *KMessageBox::defaultButton() const
{
    Q_D(const KMessageBox);
    return d->defaultButton;
}

void KMessageBox::setDefaultButton(QPushButton *button)
{
    Q_D(KMessageBox);
    if (!d->buttonBox->buttons().contains(button)){
        return;
    }

    d->defaultButton = button;
    button->setDefault(true);
    button->setFocus();
}

void KMessageBox::setDefaultButton(KMessageBox::StandardButton button)
{
    Q_D(KMessageBox);
    setDefaultButton(d->buttonBox->button(QDialogButtonBox::StandardButton(button)));
}

QAbstractButton *KMessageBox::clickedButton() const
{
    Q_D(const KMessageBox);
    return d->clickedButton;
}

QPixmap KMessageBox::standardIcon(KMessageBox::Icon icon)
{
    return KMessageBoxPrivate::standardIcon(icon, 0);
}

KMessageBox::StandardButton KMessageBox::information(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons, KMessageBox::StandardButton defaultButton)
{
    KMessageBox msgBox(parent);
    msgBox.setIcon(KMessageBox::Icon::Information);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setStandardButtons(buttons);
    msgBox.setDefaultButton(defaultButton);
    msgBox.setParent(parent);
    QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox*>();
    Q_ASSERT(buttonBox != 0);

    if (msgBox.exec() == -1)
        return KMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

KMessageBox::StandardButton KMessageBox::question(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons, KMessageBox::StandardButton defaultButton)
{
        KMessageBox msgBox(parent);
        msgBox.setIcon(KMessageBox::Icon::Question);
        msgBox.setWindowTitle(title);
        msgBox.setText(text);
        msgBox.setStandardButtons(buttons);
        msgBox.setDefaultButton(defaultButton);
        msgBox.setParent(parent);
        QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox*>();
        Q_ASSERT(buttonBox != 0);

        if (msgBox.exec() == -1)
            return KMessageBox::Cancel;
        return msgBox.standardButton(msgBox.clickedButton());
}

KMessageBox::StandardButton KMessageBox::warning(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons, KMessageBox::StandardButton defaultButton)
{
        KMessageBox msgBox(parent);
        msgBox.setIcon(KMessageBox::Icon::Warning);
        msgBox.setWindowTitle(title);
        msgBox.setText(text);
        msgBox.setStandardButtons(buttons);
        msgBox.setDefaultButton(defaultButton);
        msgBox.setParent(parent);
        QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox*>();
        Q_ASSERT(buttonBox != 0);

        if (msgBox.exec() == -1)
            return KMessageBox::Cancel;
        return msgBox.standardButton(msgBox.clickedButton());
}

KMessageBox::StandardButton KMessageBox::critical(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons, KMessageBox::StandardButton defaultButton)
{
        KMessageBox msgBox(parent);
        msgBox.setIcon(KMessageBox::Icon::Critical);
        msgBox.setWindowTitle(title);
        msgBox.setText(text);
        msgBox.setStandardButtons(buttons);
        msgBox.setDefaultButton(defaultButton);
        msgBox.setParent(parent);
        QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox*>();
        Q_ASSERT(buttonBox != 0);

        if (msgBox.exec() == -1)
            return KMessageBox::Cancel;
        return msgBox.standardButton(msgBox.clickedButton());
}

KMessageBox::StandardButton KMessageBox::success(QWidget *parent, const QString &title, const QString &text, StandardButtons buttons, KMessageBox::StandardButton defaultButton)
{
        KMessageBox msgBox(parent);
        msgBox.setCustomIcon(QIcon::fromTheme("lingmo-dialog-success"));
        msgBox.setWindowTitle(title);
        msgBox.setText(text);
        msgBox.setStandardButtons(buttons);
        msgBox.setDefaultButton(defaultButton);
        msgBox.setParent(parent);
        QDialogButtonBox *buttonBox = msgBox.findChild<QDialogButtonBox*>();
        Q_ASSERT(buttonBox != 0);

        if (msgBox.exec() == -1)
            return KMessageBox::Cancel;
        return msgBox.standardButton(msgBox.clickedButton());
}

bool KMessageBox::event(QEvent *e)
{
    Q_D(KMessageBox);
    bool result =KDialog::event(e);
    switch (e->type()) {
        case QEvent::LayoutRequest:
            d->updateSize();
            break;
        case QEvent::FontChange:
            d->updateSize();
            break;
        default:
            break;
    }
    return result;
}

KMessageBoxPrivate::KMessageBoxPrivate(KMessageBox *parent):q_ptr(parent), informativeLabel(0),
    checkbox(0), compatMode(false), clickedButton(0), defaultButton(0)

{
    Q_Q(KMessageBox);
    setParent(parent);
}

void KMessageBoxPrivate::init(const QString &title, const QString &text)
{
    Q_Q(KMessageBox);

    label = new QLabel;
    label->setObjectName(QLatin1String("qt_kmsgbox_label"));
    label->setTextInteractionFlags(Qt::TextInteractionFlags(q->style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, q)));
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setOpenExternalLinks(true);
    iconLabel = new QLabel(q);
    iconLabel->setObjectName(QLatin1String("qt_kmsgbox_icon_label"));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    buttonBox = new QDialogButtonBox;
    buttonBox->setObjectName(QLatin1String("qt_kmsgbox_buttonbox"));
    buttonBox->setCenterButtons(q->style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, q));
    QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
                     this, SLOT(_q_buttonClicked(QAbstractButton*)));
    setupLayout();
    if (!title.isEmpty() || !text.isEmpty()) {
        q->setWindowTitle(title);
        q->setText(text);
    }
    q->setModal(true);
    icon = KMessageBox::NoIcon;
}

QPixmap KMessageBoxPrivate::standardIcon(KMessageBox::Icon icon, KMessageBox *mb)
{
    QStyle *style = mb ? mb->style() : QApplication::style();
    int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, mb);
    QIcon tmpIcon;
    switch (icon) {
    case KMessageBox::Information:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, nullptr, mb);
        iconName = tmpIcon.name();
        break;
    case KMessageBox::Warning:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, nullptr, mb);
        iconName = tmpIcon.name();
        break;
    case KMessageBox::Critical:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, nullptr, mb);
        iconName = tmpIcon.name();
        break;
    case KMessageBox::Question:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, nullptr, mb);
        iconName = tmpIcon.name();
    default:
        break;
    }
    if (!tmpIcon.isNull()) {
        QWindow *window = nullptr;
        if (mb) {
            window = mb->windowHandle();
            if (!window) {
                if (const QWidget *nativeParent = mb->nativeParentWidget())
                    window = nativeParent->windowHandle();
            }
        }
        return tmpIcon.pixmap(window, QSize(iconSize, iconSize));
    }

    return QPixmap();
}

void KMessageBoxPrivate::setupLayout()
{
    Q_Q(KMessageBox);
    if(q->mainWidget()->layout())
        delete q->mainWidget()->layout();

    QGridLayout *grid = new QGridLayout;
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0,0,0,0);
    grid->setHorizontalSpacing(8);
    if(informativeLabel)
        grid->setVerticalSpacing(8);
    else
        grid->setVerticalSpacing(0);
    grid->setContentsMargins(0,0,0,0);

    bool hasIcon = iconLabel->pixmap() && !iconLabel->pixmap()->isNull();
    if (hasIcon)
        grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
    iconLabel->setVisible(hasIcon);
    grid->addWidget(label, 0, hasIcon ? 2 : 1, 1, 1);

    if (informativeLabel) {
        grid->addWidget(informativeLabel, 1, hasIcon ? 2 : 1, 1, 1);
    }
    grid->setSizeConstraint(QLayout::SetNoConstraint);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(24,0,24,24);

    buttonLayout->setSizeConstraint(QLayout::SetNoConstraint);
    if(checkbox)
        buttonLayout->addWidget(checkbox, 0, Qt::AlignLeft | Qt::AlignVCenter);
    buttonLayout->addWidget(buttonBox,0, Qt::AlignRight | Qt::AlignVCenter);

    layout->setSpacing(0);
    layout->addLayout(grid);
    layout->addSpacing(32);
    layout->addLayout(buttonLayout);

    q->mainWidget()->setLayout(layout);
    updateSize();
}

void KMessageBoxPrivate::updateSize()
{
    Q_Q(KMessageBox);

    if(!q->isVisible())
        return;

    while (buttonBox->layout()->count() < buttonBox->buttons().count() + 1) {
        QEvent event(QEvent::StyleChange);
        QGuiApplication::sendEvent(buttonBox, &event);
    }

    QSize minSize(424, 156);
    QSize size;
    QSize screenSize = QGuiApplication::screenAt(QCursor::pos())->availableGeometry().size();
    QSize maxSize(screenSize.width() * 0.8, screenSize.height() * 0.8);

    label->setWordWrap(false);
    if (informativeLabel) {
        informativeLabel->setWordWrap(false);
    }

    q->mainWidget()->layout()->activate();

    if (q->sizeHint().width() > qMax(buttonBox->sizeHint().width() + 24 + 24, 452)) {
        label->setWordWrap(true);
        if (informativeLabel) {
            informativeLabel->setWordWrap(true);
        }
    }

    q->mainWidget()->layout()->activate();

    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    size.setWidth(qMax(qMin(maxSize.width(), q->sizeHint().width()), minSize.width()));
    size.setHeight(qMax(qMin(maxSize.height(), q->layout()->hasHeightForWidth() ? q->layout()->totalHeightForWidth(size.width())
                                                                                : q->layout()->totalMinimumSize().height()), minSize.height()));
    q->setFixedSize(size);

    QCoreApplication::removePostedEvents(q, QEvent::LayoutRequest);

}

void KMessageBoxPrivate::_q_buttonClicked(QAbstractButton *button)
{
    Q_Q(KMessageBox);
    setClickedButton(button);
}

static int oldButton(int button)
{
    switch (button & KMessageBox::ButtonMask) {
    case KMessageBox::Ok:
        return Old_Ok;
    case KMessageBox::Cancel:
        return Old_Cancel;
    case KMessageBox::Yes:
        return Old_Yes;
    case KMessageBox::No:
        return Old_No;
    case KMessageBox::Abort:
        return Old_Abort;
    case KMessageBox::Retry:
        return Old_Retry;
    case KMessageBox::Ignore:
        return Old_Ignore;
    case KMessageBox::YesToAll:
        return Old_YesAll;
    case KMessageBox::NoToAll:
        return Old_NoAll;
    default:
        return 0;
    }
}

void KMessageBoxPrivate::setClickedButton(QAbstractButton *button)
{
    Q_Q(KMessageBox);

    clickedButton = button;
    emit q->buttonClicked(clickedButton);

    int resultCode = execReturnCode(button);
    q->hide();
    q->close();
    int dialogCode = dialogCodeForButton(button);
    if (dialogCode == QDialog::Accepted)
        emit q->accepted();
    else if (dialogCode == QDialog::Rejected)
        emit q->rejected();
    emit q->finished(resultCode);
}

int KMessageBoxPrivate::execReturnCode(QAbstractButton *button)
{
    int ret = buttonBox->standardButton(button);
    if (ret == KMessageBox::NoButton) {
        ret = customButtonList.indexOf(button); // if button == 0, correctly sets ret = -1
    } else if (compatMode) {
        ret = oldButton(ret);
    }
    return ret;
}

int KMessageBoxPrivate::dialogCodeForButton(QAbstractButton *button) const
{
    Q_Q(const KMessageBox);

    switch (q->buttonRole(button)) {
    case KMessageBox::AcceptRole:
    case KMessageBox::YesRole:
        return KDialog::Accepted;
    case KMessageBox::RejectRole:
    case KMessageBox::NoRole:
        return KDialog::Rejected;
    default:
        return -1;
    }
}

static KMessageBox::StandardButton newButton(int button)
{
    // this is needed for source compatibility with Qt 4.0 and 4.1
    if (button == KMessageBox::NoButton || (button & NewButtonMask))
        return KMessageBox::StandardButton(button & KMessageBox::ButtonMask);

#if QT_VERSION < 0x050000
    // this is needed for binary compatibility with Qt 4.0 and 4.1
    switch (button & Old_ButtonMask) {
    case Old_Ok:
        return KMessageBox::Ok;
    case Old_Cancel:
        return KMessageBox::Cancel;
    case Old_Yes:
        return KMessageBox::Yes;
    case Old_No:
        return KMessageBox::No;
    case Old_Abort:
        return KMessageBox::Abort;
    case Old_Retry:
        return KMessageBox::Retry;
    case Old_Ignore:
        return KMessageBox::Ignore;
    case Old_YesAll:
        return KMessageBox::YesToAll;
    case Old_NoAll:
        return KMessageBox::NoToAll;
    default:
        return KMessageBox::NoButton;
    }
#else
    return KMessageBox::NoButton;
#endif
}

static bool detectedCompat(int button0, int button1, int button2)
{
    if (button0 != 0 && !(button0 & NewButtonMask))
        return true;
    if (button1 != 0 && !(button1 & NewButtonMask))
        return true;
    if (button2 != 0 && !(button2 & NewButtonMask))
        return true;
    return false;
}

void KMessageBoxPrivate::addOldButtons(int button0, int button1, int button2)
{
    Q_Q(KMessageBox);
    q->addButton(newButton(button0));
    q->addButton(newButton(button1));
    q->addButton(newButton(button2));
    q->setDefaultButton(
        static_cast<QPushButton *>(findButton(button0, button1, button2, KMessageBox::Default)));
    compatMode = detectedCompat(button0, button1, button2);
}

QAbstractButton *KMessageBoxPrivate::findButton(int button0, int button1, int button2, int flags)
{
    Q_Q(KMessageBox);
    int button = 0;

    if (button0 & flags) {
        button = button0;
    } else if (button1 & flags) {
        button = button1;
    } else if (button2 & flags) {
        button = button2;
    }
    return q->button(newButton(button));
}

void KMessageBoxPrivate::changeTheme()
{
    Q_Q(KMessageBox);
    if(iconName != "")
        m_iconName = iconName;
    q->setIconPixmap(QIcon::fromTheme(m_iconName).pixmap(24,24));
}
}

#include "moc_kmessagebox.cpp"
#include "kmessagebox.moc"
