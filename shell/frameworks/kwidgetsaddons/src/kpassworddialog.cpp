/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2007 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#include "kpassworddialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QScreen>
#include <QStyleOption>
#include <QTimer>

#include <ktitlewidget.h>

#include "ui_kpassworddialog.h"

/** @internal */
class KPasswordDialogPrivate
{
public:
    KPasswordDialogPrivate(KPasswordDialog *qq)
        : q(qq)
    {
    }

    void actuallyAccept();
    void activated(const QString &userName);

    void updateFields();
    void init();

    KPasswordDialog *const q;
    Ui_KPasswordDialog ui;
    QMap<QString, QString> knownLogins;
    QComboBox *userEditCombo = nullptr;
    QIcon icon;
    KPasswordDialog::KPasswordDialogFlags m_flags;
    unsigned int commentRow = 0;
};

KPasswordDialog::KPasswordDialog(QWidget *parent, const KPasswordDialogFlags &flags)
    : QDialog(parent)
    , d(new KPasswordDialogPrivate(this))
{
    setWindowTitle(tr("Password", "@title:window"));
    setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-password"), windowIcon()));
    d->m_flags = flags;
    d->init();
}

KPasswordDialog::~KPasswordDialog() = default;

void KPasswordDialogPrivate::updateFields()
{
    if (m_flags & KPasswordDialog::UsernameReadOnly) {
        ui.userEdit->setReadOnly(true);
        ui.credentialsGroup->setFocusProxy(ui.passEdit);
    }
    ui.domainEdit->setReadOnly((m_flags & KPasswordDialog::DomainReadOnly));
    ui.credentialsGroup->setEnabled(!q->anonymousMode());
}

void KPasswordDialogPrivate::init()
{
    ui.setupUi(q);
    ui.buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    ui.errorMessage->setHidden(true);

    ui.userEditContextHelpButton->hide();
    ui.userEditContextHelpButton->setFlat(true);
    ui.userEditContextHelpButton->setIcon(QIcon::fromTheme(QStringLiteral("help-contextual")));
    ui.userEditContextHelpButton->setText(QString());
    const QString description = QApplication::translate("KPasswordDialog", "Show Contextual Help");
    ui.userEditContextHelpButton->setAccessibleName(description);
    ui.userEditContextHelpButton->setToolTip(description);
    QObject::connect(ui.userEditContextHelpButton, &QPushButton::released, q, [this] {
        QEvent ev(QEvent::WhatsThis);
        qApp->sendEvent(ui.userEdit, &ev);
    });

    // Row 4: Username field
    if (m_flags & KPasswordDialog::ShowUsernameLine) {
        ui.userEdit->setFocus();
        ui.credentialsGroup->setFocusProxy(ui.userEdit);
        QObject::connect(ui.userEdit, &QLineEdit::returnPressed, ui.passEdit, qOverload<>(&QWidget::setFocus));
    } else {
        ui.userNameLabel->hide();
        ui.userEdit->hide();
        ui.domainLabel->hide();
        ui.domainEdit->hide();
        ui.passEdit->setFocus();
        ui.credentialsGroup->setFocusProxy(ui.passEdit);
        ui.prompt->setText(QApplication::translate("KPasswordDialog", "Supply a password below."));
    }

    if (!(m_flags & KPasswordDialog::ShowAnonymousLoginCheckBox)) {
        ui.anonymousRadioButton->hide();
        ui.usePasswordButton->hide();
    }

    if (!(m_flags & KPasswordDialog::ShowDomainLine)) {
        ui.domainLabel->hide();
        ui.domainEdit->hide();
    }

    if (!(m_flags & KPasswordDialog::ShowKeepPassword)) {
        ui.keepCheckBox->hide();
    }

    updateFields();

    QRect desktop = q->topLevelWidget()->screen()->geometry();
    q->setMinimumWidth(qMin(1000, qMax(q->sizeHint().width(), desktop.width() / 4)));
    q->setIcon(QIcon::fromTheme(QStringLiteral("dialog-password")));
}

void KPasswordDialog::setIcon(const QIcon &icon)
{
    d->icon = icon;

    QStyleOption option;
    option.initFrom(this);
    const int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, this);
    d->ui.pixmapLabel->setPixmap(icon.pixmap(iconSize));
}

QIcon KPasswordDialog::icon() const
{
    return d->icon;
}

void KPasswordDialog::setUsername(const QString &user)
{
    d->ui.userEdit->setText(user);
    if (user.isEmpty()) {
        return;
    }

    d->activated(user);
    if (d->ui.userEdit->isVisibleTo(this)) {
        d->ui.passEdit->setFocus();
    }
}

QString KPasswordDialog::username() const
{
    return d->ui.userEdit->text();
}

QString KPasswordDialog::password() const
{
    return d->ui.passEdit->password();
}

void KPasswordDialog::setDomain(const QString &domain)
{
    d->ui.domainEdit->setText(domain);
}

QString KPasswordDialog::domain() const
{
    return d->ui.domainEdit->text();
}

void KPasswordDialog::setAnonymousMode(bool anonymous)
{
    if (anonymous && !(d->m_flags & KPasswordDialog::ShowAnonymousLoginCheckBox)) {
        // This is an error case, but we can at least let user see what's about
        // to happen if they proceed.
        d->ui.anonymousRadioButton->setVisible(true);

        d->ui.usePasswordButton->setVisible(true);
        d->ui.usePasswordButton->setEnabled(false);
    }

    d->ui.anonymousRadioButton->setChecked(anonymous);
}

bool KPasswordDialog::anonymousMode() const
{
    return d->ui.anonymousRadioButton->isChecked();
}

void KPasswordDialog::setKeepPassword(bool b)
{
    d->ui.keepCheckBox->setChecked(b);
}

bool KPasswordDialog::keepPassword() const
{
    return d->ui.keepCheckBox->isChecked();
}

void KPasswordDialog::addCommentLine(const QString &label, const QString &comment)
{
    int gridMarginLeft;
    int gridMarginTop;
    int gridMarginRight;
    int gridMarginBottom;
    d->ui.formLayout->getContentsMargins(&gridMarginLeft, &gridMarginTop, &gridMarginRight, &gridMarginBottom);

    int spacing = d->ui.formLayout->horizontalSpacing();
    if (spacing < 0) {
        // same inter-column spacing for all rows, see comment in qformlayout.cpp
        spacing = style()->combinedLayoutSpacing(QSizePolicy::Label, QSizePolicy::LineEdit, Qt::Horizontal, nullptr, this);
    }

    QLabel *c = new QLabel(comment, this);
    c->setWordWrap(true);
    c->setTextInteractionFlags(Qt::TextBrowserInteraction);

    d->ui.formLayout->insertRow(d->commentRow, label, c);
    ++d->commentRow;

    // cycle through column 0 widgets and see the max width so we can set the minimum height of
    // column 2 wordwrapable labels
    int firstColumnWidth = 0;
    for (int i = 0; i < d->ui.formLayout->rowCount(); ++i) {
        QLayoutItem *li = d->ui.formLayout->itemAt(i, QFormLayout::LabelRole);
        if (li) {
            QWidget *w = li->widget();
            if (w && !w->isHidden()) {
                firstColumnWidth = qMax(firstColumnWidth, w->sizeHint().width());
            }
        }
    }
    for (int i = 0; i < d->ui.formLayout->rowCount(); ++i) {
        QLayoutItem *li = d->ui.formLayout->itemAt(i, QFormLayout::FieldRole);
        if (li) {
            QLabel *l = qobject_cast<QLabel *>(li->widget());
            if (l && l->wordWrap()) {
                auto *style = this->style();
                const int leftMargin = style->pixelMetric(QStyle::PM_LayoutLeftMargin);
                const int rightMargin = style->pixelMetric(QStyle::PM_LayoutRightMargin);
                int w = sizeHint().width() - firstColumnWidth - leftMargin - rightMargin - gridMarginLeft - gridMarginRight - spacing;
                l->setMinimumSize(w, l->heightForWidth(w));
            }
        }
    }
}

void KPasswordDialog::showErrorMessage(const QString &message, const ErrorType type)
{
    d->ui.errorMessage->setText(message, KTitleWidget::ErrorMessage);

    QFont bold = font();
    bold.setBold(true);
    switch (type) {
    case PasswordError:
        d->ui.passwordLabel->setFont(bold);
        d->ui.passEdit->clear();
        d->ui.passEdit->setFocus();
        break;
    case UsernameError:
        if (d->ui.userEdit->isVisibleTo(this)) {
            d->ui.userNameLabel->setFont(bold);
            d->ui.userEdit->setFocus();
        }
        break;
    case DomainError:
        if (d->ui.domainEdit->isVisibleTo(this)) {
            d->ui.domainLabel->setFont(bold);
            d->ui.domainEdit->setFocus();
        }
        break;
    case FatalError:
        d->ui.userNameLabel->setEnabled(false);
        d->ui.userEdit->setEnabled(false);
        d->ui.passwordLabel->setEnabled(false);
        d->ui.passEdit->setEnabled(false);
        d->ui.keepCheckBox->setEnabled(false);
        d->ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        break;
    default:
        break;
    }
    adjustSize();
}

void KPasswordDialog::setPrompt(const QString &prompt)
{
    d->ui.prompt->setText(prompt);
    d->ui.prompt->setWordWrap(true);
    auto *style = this->style();
    const int leftMarginHint = style->pixelMetric(QStyle::PM_LayoutLeftMargin);
    const int rightMarginHint = style->pixelMetric(QStyle::PM_LayoutRightMargin);
    d->ui.prompt->setMinimumHeight(d->ui.prompt->heightForWidth(width() - leftMarginHint - rightMarginHint));
}

QString KPasswordDialog::prompt() const
{
    return d->ui.prompt->text();
}

void KPasswordDialog::setPassword(const QString &p)
{
    d->ui.passEdit->setPassword(p);
}

void KPasswordDialog::setUsernameReadOnly(bool readOnly)
{
    d->ui.userEdit->setReadOnly(readOnly);

    if (readOnly && d->ui.userEdit->hasFocus()) {
        d->ui.passEdit->setFocus();
    }
}

void KPasswordDialog::setKnownLogins(const QMap<QString, QString> &knownLogins)
{
    const int nr = knownLogins.count();
    if (nr == 0) {
        return;
    }

    if (nr == 1) {
        d->ui.userEdit->setText(knownLogins.begin().key());
        setPassword(knownLogins.begin().value());
        return;
    }

    Q_ASSERT(!d->ui.userEdit->isReadOnly());
    if (!d->userEditCombo) {
        int row = -1;
        QFormLayout::ItemRole userEditRole = QFormLayout::FieldRole;

        d->ui.formLayout->getWidgetPosition(d->ui.userEdit, &row, &userEditRole);
        d->ui.formLayout->removeWidget(d->ui.userEdit);
        delete d->ui.userEdit;
        d->userEditCombo = new QComboBox(d->ui.credentialsGroup);
        d->userEditCombo->setEditable(true);
        d->ui.userEdit = d->userEditCombo->lineEdit();
        d->ui.userNameLabel->setBuddy(d->userEditCombo);
        d->ui.formLayout->setWidget(row > -1 ? row : 0, userEditRole, d->userEditCombo);

        setTabOrder(d->ui.userEdit, d->ui.anonymousRadioButton);
        setTabOrder(d->ui.anonymousRadioButton, d->ui.domainEdit);
        setTabOrder(d->ui.domainEdit, d->ui.passEdit);
        setTabOrder(d->ui.passEdit, d->ui.keepCheckBox);
        connect(d->ui.userEdit, &QLineEdit::returnPressed, d->ui.passEdit, qOverload<>(&QWidget::setFocus));
    }

    d->knownLogins = knownLogins;
    d->userEditCombo->addItems(knownLogins.keys());
    d->userEditCombo->setFocus();

    connect(d->userEditCombo, &QComboBox::textActivated, this, [this](const QString &text) {
        d->activated(text);
    });
}

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void KPasswordDialog::setRevealPasswordAvailable(bool reveal)
{
    d->ui.passEdit->setRevealPasswordAvailable(reveal);
}

bool KPasswordDialog::isRevealPasswordAvailable() const
{
    return d->ui.passEdit->isRevealPasswordAvailable();
}
#pragma GCC diagnostic pop
#endif

KPassword::RevealMode KPasswordDialog::revealPasswordMode() const
{
    return d->ui.passEdit->revealPasswordMode();
}

void KPasswordDialog::setRevealPasswordMode(KPassword::RevealMode revealPasswordMode)
{
    d->ui.passEdit->setRevealPasswordMode(revealPasswordMode);
}

void KPasswordDialogPrivate::activated(const QString &userName)
{
    QMap<QString, QString>::ConstIterator it = knownLogins.constFind(userName);
    if (it != knownLogins.constEnd()) {
        q->setPassword(it.value());
    }
}

void KPasswordDialog::accept()
{
    if (!d->ui.errorMessage->isHidden()) {
        d->ui.errorMessage->setText(QString());
    }

    // reset the font in case we had an error previously
    if (!d->ui.passwordLabel->isHidden()) {
        d->ui.passwordLabel->setFont(font());
        d->ui.userNameLabel->setFont(font());
    }

    // we do this to allow the error message, if any, to go away
    // checkPassword() may block for a period of time
    QTimer::singleShot(0, this, [this] {
        d->actuallyAccept();
    });
}

void KPasswordDialogPrivate::actuallyAccept()
{
    if (!q->checkPassword()) {
        return;
    }

    bool keep = ui.keepCheckBox->isVisibleTo(q) && ui.keepCheckBox->isChecked();
    Q_EMIT q->gotPassword(q->password(), keep);

    if (ui.userEdit->isVisibleTo(q)) {
        Q_EMIT q->gotUsernameAndPassword(q->username(), q->password(), keep);
    }

    q->QDialog::accept();
}

bool KPasswordDialog::checkPassword()
{
    return true;
}

QDialogButtonBox *KPasswordDialog::buttonBox() const
{
    return d->ui.buttonBox;
}

void KPasswordDialog::setUsernameContextHelp(const QString &help)
{
    d->ui.userEditContextHelpButton->setVisible(true);
    d->ui.userEdit->setWhatsThis(help);
}

#include "moc_kpassworddialog.cpp"
