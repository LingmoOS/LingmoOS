/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2012 David Faure <faure+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kmessagebox.h"
#include "kmessagebox_p.h"

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextBrowser>
#include <QWindow>

#include <qapplication.h>
#if 0
// NOTE waiting for the notification framework plan
#include <knotification.h>
#endif
#include <ksqueezedtextlabel.h>

#include <KCollapsibleGroupBox>

namespace KMessageBox
{
/**
 * @private Prevent kapidox's doxygen config to pick up this namespace variable
 * this static is used by the createKMessageBox function to enqueue dialogs
 * FIXME what should we do about this static?
 */
QDialogButtonBox::StandardButton KWIDGETSADDONS_EXPORT (*KMessageBox_exec_hook)(QDialog *) = nullptr;

/// @private Prevent kapidox's doxygen config pick up this namespace method
static void applyOptions(QDialog *dialog, KMessageBox::Options options)
{
    if (options & KMessageBox::WindowModal) {
        dialog->setWindowModality(Qt::WindowModal);
    }
    dialog->setModal(true);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
// This method has been copied from KWindowSystem to avoid depending on it
static void setMainWindow(QWidget *subWidget, WId mainWindowId)
{
#ifdef Q_OS_OSX
    if (!QWidget::find(mainWindowId)) {
        return;
    }
#endif
    // Set the WA_NativeWindow attribute to force the creation of the QWindow.
    // Without this QWidget::windowHandle() returns 0.
    subWidget->setAttribute(Qt::WA_NativeWindow, true);
    QWindow *subWindow = subWidget->windowHandle();
    Q_ASSERT(subWindow);

    QWindow *mainWindow = QWindow::fromWinId(mainWindowId);
    if (!mainWindow) {
        // foreign windows not supported on all platforms
        return;
    }
    // mainWindow is not the child of any object, so make sure it gets deleted at some point
    QObject::connect(subWidget, &QObject::destroyed, mainWindow, &QObject::deleteLater);
    subWindow->setTransientParent(mainWindow);
}

/**
 * @private Prevent kapidox's doxygen config pick up this namespace method
 * Create a QDialog whose parent is a foreign window
 */
static QDialog *createWIdDialog(WId parent_id)
{
    QWidget *parent = QWidget::find(parent_id);
    QDialog *dialog = new QDialog(parent, Qt::Dialog);
    if (!parent && parent_id) {
        setMainWindow(dialog, parent_id);
    }
    return dialog;
}

class DialogButtonsHelper : public QObject
{
    Q_OBJECT
public:
    DialogButtonsHelper(QDialog *dialog, QDialogButtonBox *buttons)
        : QObject(dialog)
        , m_dialog(dialog)
        , m_buttons(buttons)
        , m_details(nullptr)
    {
        connect(m_buttons, &QDialogButtonBox::clicked, this, &DialogButtonsHelper::onButtonClicked);
    }

    void setDetailsWidget(QWidget *widget)
    {
        m_details = widget;
    }

public Q_SLOTS:
    void onButtonClicked(QAbstractButton *button)
    {
        QDialogButtonBox::StandardButton code = m_buttons->standardButton(button);
        if (code != QDialogButtonBox::NoButton) {
            m_dialog->done(code);
        }
    }

private:
    QDialog *const m_dialog;
    QDialogButtonBox *const m_buttons;
    QWidget *m_details;
};

QDialogButtonBox::StandardButton createKMessageBox(QDialog *dialog,
                                                   QDialogButtonBox *buttons,
                                                   QMessageBox::Icon icon,
                                                   const QString &text,
                                                   const QStringList &strlist,
                                                   const QString &ask,
                                                   bool *checkboxReturn,
                                                   Options options,
                                                   const QString &details)
{
    QIcon tmpIcon;
    QStyle *style = dialog ? dialog->style() : QApplication::style();
    switch (icon) {
    case QMessageBox::Information:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, nullptr, dialog);
        break;
    case QMessageBox::Warning:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, nullptr, dialog);
        break;
    case QMessageBox::Critical:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, nullptr, dialog);
        break;
    case QMessageBox::Question:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, nullptr, dialog);
        break;
    default:
        break;
    }

    return createKMessageBox(dialog, buttons, tmpIcon, text, strlist, ask, checkboxReturn, options, details, icon);
}

QDialogButtonBox::StandardButton createKMessageBox(QDialog *dialog,
                                                   QDialogButtonBox *buttons,
                                                   const QIcon &icon,
                                                   const QString &text,
                                                   const QStringList &strlist,
                                                   const QString &ask,
                                                   bool *checkboxReturn,
                                                   Options options,
                                                   const QString &details,
                                                   QMessageBox::Icon notifyType)
{
    if (!isNotifyInterfaceLoaded()) {
        // This is annoying. Loading the interface later (or at any point) will
        // install the qm translation of the knotifications library which will cause a LanguageChange event to be sent
        // Which will end up in QDialogButtonBoxPrivate::retranslateStrings and will overwrite the texts
        // we have just set. So here we save our texts, force load the interface and reset the texts
        // For people wondering if they can remove this bit of code later, this seems to only be an issue
        // in non Plasma desktops, emulate by running something like XDG_CURRENT_DESKTOP=X-Cinnamon okular
        // on plasma desktops the platform plugin already uses knotifications so it's qm has been loaded on startup
        QMap<QPointer<QAbstractButton>, QString> buttonTexts;
        for (QAbstractButton *b : buttons->buttons()) {
            buttonTexts[b] = b->text();
        }
        notifyInterface();
        QApplication::processEvents(); // We need to force the LanguageChange to be processed
        for (auto it = buttonTexts.constBegin(); it != buttonTexts.constEnd(); ++it) {
            if (it.key()) {
                it.key()->setText(it.value());
            }
        }
    }

    DialogButtonsHelper *buttonsHelper = new DialogButtonsHelper(dialog, buttons);

    QWidget *mainWidget = new QWidget(dialog);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    const int verticalSpacing = mainWidget->style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing);
    const int horizontalSpacing = mainWidget->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
    mainLayout->setSpacing(verticalSpacing * 2); // provide extra spacing
    mainLayout->setContentsMargins(0, 0, 0, 0);
    buttons->setParent(dialog);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(-1); // use default spacing
    mainLayout->addLayout(hLayout, 5);

    QLabel *iconLabel = new QLabel(mainWidget);

    if (!icon.isNull()) {
        QStyleOption option;
        option.initFrom(mainWidget);
        iconLabel->setPixmap(icon.pixmap(mainWidget->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, mainWidget)));
    }

    QVBoxLayout *iconLayout = new QVBoxLayout();
    iconLayout->addStretch(1);
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch(5);

    hLayout->addLayout(iconLayout, 0);
    hLayout->addSpacing(horizontalSpacing);

    QLabel *messageLabel = new QLabel(text, mainWidget);
    messageLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
    Qt::TextInteractionFlags flags = Qt::TextSelectableByMouse;
    if (options & KMessageBox::AllowLink) {
        flags |= Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
    }
    messageLabel->setTextInteractionFlags(flags);

    QRect desktop = dialog->screen()->geometry();
    bool usingSqueezedTextLabel = false;
    if (messageLabel->sizeHint().width() > desktop.width() * 0.5) {
        // enable automatic wrapping of messages which are longer than 50% of screen width
        messageLabel->setWordWrap(true);
        // use a squeezed label if text is still too wide
        usingSqueezedTextLabel = messageLabel->sizeHint().width() > desktop.width() * 0.85;
        if (usingSqueezedTextLabel) {
            delete messageLabel;
            messageLabel = new KSqueezedTextLabel(text, mainWidget);
            messageLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
            messageLabel->setTextInteractionFlags(flags);
        }
    }

    QPalette messagePal(messageLabel->palette());
    messagePal.setColor(QPalette::Window, Qt::transparent);
    messageLabel->setPalette(messagePal);

    bool usingScrollArea = desktop.height() / 3 < messageLabel->sizeHint().height();
    if (usingScrollArea) {
        QScrollArea *messageScrollArea = new QScrollArea(mainWidget);
        messageScrollArea->setWidget(messageLabel);
        messageScrollArea->setFrameShape(QFrame::NoFrame);
        messageScrollArea->setWidgetResizable(true);
        QPalette scrollPal(messageScrollArea->palette());
        scrollPal.setColor(QPalette::Window, Qt::transparent);
        messageScrollArea->viewport()->setPalette(scrollPal);
        hLayout->addWidget(messageScrollArea, 5);
    } else {
        hLayout->addWidget(messageLabel, 5);
    }

    const bool usingListWidget = !strlist.isEmpty();
    if (usingListWidget) {
        // enable automatic wrapping since the listwidget has already a good initial width
        messageLabel->setWordWrap(true);
        QListWidget *listWidget = new QListWidget(mainWidget);
        listWidget->addItems(strlist);

        QStyleOptionViewItem styleOption;
        styleOption.initFrom(listWidget);
        QFontMetrics fm(styleOption.font);
        int w = listWidget->width();
        for (const QString &str : strlist) {
            w = qMax(w, fm.boundingRect(str).width());
        }
        const int borderWidth = listWidget->width() - listWidget->viewport()->width() + listWidget->verticalScrollBar()->height();
        w += borderWidth;
        if (w > desktop.width() * 0.85) { // limit listWidget size to 85% of screen width
            w = qRound(desktop.width() * 0.85);
        }
        listWidget->setMinimumWidth(w);

        mainLayout->addWidget(listWidget, usingScrollArea ? 10 : 50);
        listWidget->setSelectionMode(QListWidget::NoSelection);
        messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    } else if (!usingScrollArea) {
        mainLayout->addStretch(15);
    }

    QPointer<QCheckBox> checkbox = nullptr;
    if (!ask.isEmpty()) {
        checkbox = new QCheckBox(ask, mainWidget);
        mainLayout->addWidget(checkbox);
        if (checkboxReturn) {
            checkbox->setChecked(*checkboxReturn);
        }
    }

    QVBoxLayout *topLayout = new QVBoxLayout(dialog);
    topLayout->addWidget(mainWidget);

    if (!details.isEmpty()) {
        QHBoxLayout *detailsHLayout = new QHBoxLayout();
        detailsHLayout->addSpacing(2 * horizontalSpacing + iconLayout->sizeHint().width());
        KCollapsibleGroupBox *detailsGroup = new KCollapsibleGroupBox();
        detailsGroup->setTitle(QApplication::translate("KMessageBox", "Details"));
        QVBoxLayout *detailsLayout = new QVBoxLayout(detailsGroup);
        if (details.length() < 512) {
            QLabel *detailsLabel = new QLabel(details);
            detailsLabel->setOpenExternalLinks(options & KMessageBox::AllowLink);
            Qt::TextInteractionFlags flags = Qt::TextSelectableByMouse;
            if (options & KMessageBox::AllowLink) {
                flags |= Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard;
            };
            detailsLabel->setTextInteractionFlags(flags);
            detailsLabel->setWordWrap(true);
            detailsLabel->setMinimumSize(detailsLabel->sizeHint());
            detailsLayout->addWidget(detailsLabel, 50);
        } else {
            QTextBrowser *detailTextEdit = new QTextBrowser();
            detailTextEdit->setText(details);
            detailTextEdit->setReadOnly(true);
            detailTextEdit->setMinimumHeight(detailTextEdit->fontMetrics().lineSpacing() * 11);
            detailsLayout->addWidget(detailTextEdit, 50);
        }
        if (!usingListWidget) {
            mainLayout->setStretchFactor(hLayout, 10);
        }
        detailsHLayout->addWidget(detailsGroup);
        topLayout->addLayout(detailsHLayout);
        buttonsHelper->setDetailsWidget(detailsGroup);
    }

    topLayout->addWidget(buttons);
    topLayout->setSizeConstraint(QLayout::SetFixedSize);

    if (!usingListWidget && !usingScrollArea && !usingSqueezedTextLabel && details.isEmpty()) {
        dialog->setFixedSize(dialog->sizeHint() + QSize(10, 10));
    }

    if ((options & KMessageBox::Dangerous)) {
        QPushButton *cancelButton = buttons->button(QDialogButtonBox::Cancel);
        QPushButton *noButton = buttons->button(QDialogButtonBox::No);

        if (cancelButton && cancelButton->isEnabled()) {
            cancelButton->setDefault(true);
            cancelButton->setFocus();
        } else if (noButton && noButton->isEnabled()) {
            noButton->setDefault(true);
            noButton->setFocus();
        }
    }

#ifndef Q_OS_WIN // FIXME problems with KNotify on Windows
    if ((options & KMessageBox::Notify)) {
        QString message = text;
        if (!strlist.isEmpty()) {
            message += QLatin1Char('\n') + strlist.join(QLatin1Char('\n'));
        }
        notifyInterface()->sendNotification(notifyType, message, dialog->topLevelWidget());
    }
#endif

    if (KMessageBox_exec_hook) {
        return KMessageBox_exec_hook(dialog);
    }

    if ((options & KMessageBox::NoExec)) {
        return QDialogButtonBox::NoButton; // We have to return something.
    }

    // We use a QPointer because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the QPointer will reset to 0.
    QPointer<QDialog> guardedDialog = dialog;

    const QDialogButtonBox::StandardButton result = QDialogButtonBox::StandardButton(guardedDialog->exec());
    if (checkbox && checkboxReturn) {
        *checkboxReturn = checkbox->isChecked();
    }

    delete guardedDialog;
    return result;
}

ButtonCode questionTwoActions(QWidget *parent,
                              const QString &text,
                              const QString &title,
                              const KGuiItem &primaryAction,
                              const KGuiItem &secondaryAction,
                              const QString &dontAskAgainName,
                              Options options)
{
    return questionTwoActionsList(parent, text, QStringList(), title, primaryAction, secondaryAction, dontAskAgainName, options);
}

bool shouldBeShownTwoActions(const QString &dontShowAgainName, ButtonCode &result)
{
    if (dontShowAgainName.isEmpty()) {
        return true;
    }
    return dontAskAgainInterface()->shouldBeShownTwoActions(dontShowAgainName, result);
}

bool shouldBeShownContinue(const QString &dontShowAgainName)
{
    if (dontShowAgainName.isEmpty()) {
        return true;
    }
    return dontAskAgainInterface()->shouldBeShownContinue(dontShowAgainName);
}

void saveDontShowAgainTwoActions(const QString &dontShowAgainName, ButtonCode result)
{
    if (dontShowAgainName.isEmpty()) {
        return;
    }
    dontAskAgainInterface()->saveDontShowAgainTwoActions(dontShowAgainName, result);
}

void saveDontShowAgainContinue(const QString &dontShowAgainName)
{
    if (dontShowAgainName.isEmpty()) {
        return;
    }
    dontAskAgainInterface()->saveDontShowAgainContinue(dontShowAgainName);
}

void enableAllMessages()
{
    dontAskAgainInterface()->enableAllMessages();
}

void enableMessage(const QString &dontShowAgainName)
{
    dontAskAgainInterface()->enableMessage(dontShowAgainName);
}

void setDontShowAgainConfig(KConfig *cfg)
{
    dontAskAgainInterface()->setConfig(cfg);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static ButtonCode questionTwoActionsListInternal(QDialog *dialog,
                                                 const QString &text,
                                                 const QStringList &strlist,
                                                 const QString &title,
                                                 const KGuiItem &primaryAction,
                                                 const KGuiItem &secondaryAction,
                                                 const QString &dontAskAgainName,
                                                 Options options)
{
    ButtonCode res;
    if (!shouldBeShownTwoActions(dontAskAgainName, res)) {
        delete dialog;
        return res;
    }

    dialog->setWindowTitle(title.isEmpty() ? QApplication::translate("KMessageBox", "Question") : title);
    dialog->setObjectName(QStringLiteral("questionTwoActions"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), primaryAction);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), secondaryAction);

    applyOptions(dialog, options);

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog,
                                         buttonBox,
                                         QMessageBox::Question,
                                         text,
                                         strlist,
                                         dontAskAgainName.isEmpty() ? QString() : QApplication::translate("KMessageBox", "Do not ask again"),
                                         &checkboxResult,
                                         options);
    res = (result == QDialogButtonBox::Yes ? PrimaryAction : SecondaryAction);

    if (checkboxResult) {
        saveDontShowAgainTwoActions(dontAskAgainName, res);
    }
    return res;
}

ButtonCode questionTwoActionsList(QWidget *parent,
                                  const QString &text,
                                  const QStringList &strlist,
                                  const QString &title,
                                  const KGuiItem &primaryAction,
                                  const KGuiItem &secondaryAction,
                                  const QString &dontAskAgainName,
                                  Options options)
{
    return questionTwoActionsListInternal(new QDialog(parent), text, strlist, title, primaryAction, secondaryAction, dontAskAgainName, options);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static ButtonCode questionTwoActionsCancelInternal(QDialog *dialog,
                                                   const QString &text,
                                                   const QString &title,
                                                   const KGuiItem &primaryAction,
                                                   const KGuiItem &secondaryAction,
                                                   const KGuiItem &cancelAction,
                                                   const QString &dontAskAgainName,
                                                   Options options)
{
    ButtonCode res;
    if (!shouldBeShownTwoActions(dontAskAgainName, res)) {
        delete dialog;
        return res;
    }

    dialog->setWindowTitle(title.isEmpty() ? QApplication::translate("KMessageBox", "Question") : title);
    dialog->setObjectName(QStringLiteral("QuestionTwoActionsCancel"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), primaryAction);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), secondaryAction);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), cancelAction);

    applyOptions(dialog, options);

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog,
                                         buttonBox,
                                         QMessageBox::Question,
                                         text,
                                         QStringList(),
                                         dontAskAgainName.isEmpty() ? QString() : QApplication::translate("KMessageBox", "Do not ask again"),
                                         &checkboxResult,
                                         options);

    if (result == QDialogButtonBox::Yes) {
        res = PrimaryAction;
    } else if (result == QDialogButtonBox::No) {
        res = SecondaryAction;
    } else {
        return Cancel;
    }

    if (checkboxResult) {
        saveDontShowAgainTwoActions(dontAskAgainName, res);
    }
    return res;
}

ButtonCode questionTwoActionsCancel(QWidget *parent,
                                    const QString &text,
                                    const QString &title,
                                    const KGuiItem &primaryAction,
                                    const KGuiItem &secondaryAction,
                                    const KGuiItem &cancelAction,
                                    const QString &dontAskAgainName,
                                    Options options)
{
    return questionTwoActionsCancelInternal(new QDialog(parent), text, title, primaryAction, secondaryAction, cancelAction, dontAskAgainName, options);
}

ButtonCode warningTwoActions(QWidget *parent,
                             const QString &text,
                             const QString &title,
                             const KGuiItem &primaryAction,
                             const KGuiItem &secondaryAction,
                             const QString &dontAskAgainName,
                             Options options)
{
    return warningTwoActionsList(parent, text, QStringList(), title, primaryAction, secondaryAction, dontAskAgainName, options);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static ButtonCode warningTwoActionsListInternal(QDialog *dialog,
                                                const QString &text,
                                                const QStringList &strlist,
                                                const QString &title,
                                                const KGuiItem &primaryAction,
                                                const KGuiItem &secondaryAction,
                                                const QString &dontAskAgainName,
                                                Options options)
{
    ButtonCode res;
    if (!shouldBeShownTwoActions(dontAskAgainName, res)) {
        delete dialog;
        return res;
    }

    dialog->setWindowTitle(title.isEmpty() ? QApplication::translate("KMessageBox", "Warning") : title);
    dialog->setObjectName(QStringLiteral("warningTwoActionsList"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), primaryAction);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), secondaryAction);

    applyOptions(dialog, options);

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog,
                                         buttonBox,
                                         QMessageBox::Warning,
                                         text,
                                         strlist,
                                         dontAskAgainName.isEmpty() ? QString() : QApplication::translate("KMessageBox", "Do not ask again"),
                                         &checkboxResult,
                                         options);
    res = (result == QDialogButtonBox::Yes ? PrimaryAction : SecondaryAction);

    if (checkboxResult) {
        saveDontShowAgainTwoActions(dontAskAgainName, res);
    }
    return res;
}

ButtonCode warningTwoActionsList(QWidget *parent,
                                 const QString &text,
                                 const QStringList &strlist,
                                 const QString &title,
                                 const KGuiItem &primaryAction,
                                 const KGuiItem &secondaryAction,
                                 const QString &dontAskAgainName,
                                 Options options)
{
    return warningTwoActionsListInternal(new QDialog(parent), text, strlist, title, primaryAction, secondaryAction, dontAskAgainName, options);
}

ButtonCode warningContinueCancel(QWidget *parent,
                                 const QString &text,
                                 const QString &title,
                                 const KGuiItem &buttonContinue,
                                 const KGuiItem &buttonCancel,
                                 const QString &dontAskAgainName,
                                 Options options)
{
    return warningContinueCancelList(parent, text, QStringList(), title, buttonContinue, buttonCancel, dontAskAgainName, options);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static ButtonCode warningContinueCancelListInternal(QDialog *dialog,
                                                    const QString &text,
                                                    const QStringList &strlist,
                                                    const QString &title,
                                                    const KGuiItem &buttonContinue,
                                                    const KGuiItem &buttonCancel,
                                                    const QString &dontAskAgainName,
                                                    Options options,
                                                    const QString &details)
{
    if (!shouldBeShownContinue(dontAskAgainName)) {
        delete dialog;
        return Continue;
    }

    dialog->setWindowTitle(title.isEmpty() ? QApplication::translate("KMessageBox", "Warning") : title);
    dialog->setObjectName(QStringLiteral("warningYesNo"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), buttonContinue);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), buttonCancel);

    applyOptions(dialog, options);

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog,
                                         buttonBox,
                                         QMessageBox::Warning,
                                         text,
                                         strlist,
                                         dontAskAgainName.isEmpty() ? QString() : QApplication::translate("KMessageBox", "Do not ask again"),
                                         &checkboxResult,
                                         options,
                                         details);

    if (result != QDialogButtonBox::Yes) {
        return Cancel;
    }
    if (checkboxResult) {
        saveDontShowAgainContinue(dontAskAgainName);
    }
    return Continue;
}

ButtonCode warningContinueCancelList(QWidget *parent,
                                     const QString &text,
                                     const QStringList &strlist,
                                     const QString &title,
                                     const KGuiItem &buttonContinue,
                                     const KGuiItem &buttonCancel,
                                     const QString &dontAskAgainName,
                                     Options options)
{
    return warningContinueCancelListInternal(new QDialog(parent), text, strlist, title, buttonContinue, buttonCancel, dontAskAgainName, options, QString());
}

ButtonCode warningContinueCancelDetailed(QWidget *parent,
                                         const QString &text,
                                         const QString &title,
                                         const KGuiItem &buttonContinue,
                                         const KGuiItem &buttonCancel,
                                         const QString &dontAskAgainName,
                                         Options options,
                                         const QString &details)
{
    return warningContinueCancelListInternal(new QDialog(parent), text, QStringList(), title, buttonContinue, buttonCancel, dontAskAgainName, options, details);
}

ButtonCode warningTwoActionsCancel(QWidget *parent,
                                   const QString &text,
                                   const QString &title,
                                   const KGuiItem &primaryAction,
                                   const KGuiItem &secondaryAction,
                                   const KGuiItem &cancelAction,
                                   const QString &dontAskAgainName,
                                   Options options)
{
    return warningTwoActionsCancelList(parent, text, QStringList(), title, primaryAction, secondaryAction, cancelAction, dontAskAgainName, options);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static ButtonCode warningTwoActionsCancelListInternal(QDialog *dialog,
                                                      const QString &text,
                                                      const QStringList &strlist,
                                                      const QString &title,
                                                      const KGuiItem &primaryAction,
                                                      const KGuiItem &secondaryAction,
                                                      const KGuiItem &cancelAction,
                                                      const QString &dontAskAgainName,
                                                      Options options)
{
    ButtonCode res;
    if (!shouldBeShownTwoActions(dontAskAgainName, res)) {
        delete dialog;
        return res;
    }

    dialog->setWindowTitle(title.isEmpty() ? QApplication::translate("KMessageBox", "Warning") : title);
    dialog->setObjectName(QStringLiteral("warningTwoActionsCancel"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), primaryAction);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), secondaryAction);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), cancelAction);

    applyOptions(dialog, options);

    bool checkboxResult = false;
    const int result = createKMessageBox(dialog,
                                         buttonBox,
                                         QMessageBox::Warning,
                                         text,
                                         strlist,
                                         dontAskAgainName.isEmpty() ? QString() : QApplication::translate("KMessageBox", "Do not ask again"),
                                         &checkboxResult,
                                         options);

    if (result == QDialogButtonBox::Yes) {
        res = PrimaryAction;
    } else if (result == QDialogButtonBox::No) {
        res = SecondaryAction;
    } else {
        return Cancel;
    }

    if (checkboxResult) {
        saveDontShowAgainTwoActions(dontAskAgainName, res);
    }
    return res;
}

ButtonCode warningTwoActionsCancelList(QWidget *parent,
                                       const QString &text,
                                       const QStringList &strlist,
                                       const QString &title,
                                       const KGuiItem &primaryAction,
                                       const KGuiItem &secondaryAction,
                                       const KGuiItem &cancelAction,
                                       const QString &dontAskAgainName,
                                       Options options)
{
    return warningTwoActionsCancelListInternal(new QDialog(parent),
                                               text,
                                               strlist,
                                               title,
                                               primaryAction,
                                               secondaryAction,
                                               cancelAction,
                                               dontAskAgainName,
                                               options);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static void errorInternal(QDialog *dialog, const QString &text, const QString &title, const KGuiItem &buttonOk, Options options)
{
    dialog->setWindowTitle(title.isEmpty() ? QApplication::translate("KMessageBox", "Error") : title);
    dialog->setObjectName(QStringLiteral("error"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), buttonOk);

    applyOptions(dialog, options);

    createKMessageBox(dialog, buttonBox, QMessageBox::Critical, text, QStringList(), QString(), nullptr, options);
}

void error(QWidget *parent, const QString &text, const QString &title, Options options)
{
    errorInternal(new QDialog(parent), text, title, KStandardGuiItem::ok(), options);
}

void error(QWidget *parent, const QString &text, const QString &title, const KGuiItem &buttonOk, Options options)
{
    errorInternal(new QDialog(parent), text, title, buttonOk, options);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static void errorListInternal(QDialog *dialog, const QString &text, const QStringList &strlist, const QString &title, Options options)
{
    dialog->setWindowTitle(title.isEmpty() ? QApplication::translate("KMessageBox", "Error") : title);
    dialog->setObjectName(QStringLiteral("error"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions(dialog, options);

    createKMessageBox(dialog, buttonBox, QMessageBox::Critical, text, strlist, QString(), nullptr, options);
}

void errorList(QWidget *parent, const QString &text, const QStringList &strlist, const QString &title, Options options)
{
    errorListInternal(new QDialog(parent), text, strlist, title, options);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static void detailedErrorInternal(QDialog *dialog, const QString &text, const QString &details, const QString &title, const KGuiItem &buttonOk, Options options)
{
    dialog->setWindowTitle(title.isEmpty() ? QApplication::translate("KMessageBox", "Error") : title);
    dialog->setObjectName(QStringLiteral("error"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->addButton(QDialogButtonBox::Ok);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), buttonOk);
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();

    applyOptions(dialog, options);

    createKMessageBox(dialog, buttonBox, QMessageBox::Critical, text, QStringList(), QString(), nullptr, options, details);
}

void detailedError(QWidget *parent, const QString &text, const QString &details, const QString &title, Options options)
{
    detailedErrorInternal(new QDialog(parent), text, details, title, KStandardGuiItem::ok(), options);
}

void detailedError(QWidget *parent, const QString &text, const QString &details, const QString &title, const KGuiItem &buttonOk, Options options)
{
    detailedErrorInternal(new QDialog(parent), text, details, title, buttonOk, options);
}

void information(QWidget *parent, const QString &text, const QString &title, const QString &dontShowAgainName, Options options)
{
    informationList(parent, text, QStringList(), title, dontShowAgainName, options);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static void informationListInternal(QDialog *dialog,
                                    const QString &text,
                                    const QStringList &strlist,
                                    const QString &title,
                                    const QString &dontShowAgainName,
                                    Options options)
{
    if (!shouldBeShownContinue(dontShowAgainName)) {
        delete dialog;
        return;
    }

    dialog->setWindowTitle(title.isEmpty() ? QApplication::translate("KMessageBox", "Information") : title);
    dialog->setObjectName(QStringLiteral("information"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    applyOptions(dialog, options);

    bool checkboxResult = false;

    createKMessageBox(dialog,
                      buttonBox,
                      QMessageBox::Information,
                      text,
                      strlist,
                      dontShowAgainName.isEmpty() ? QString() : QApplication::translate("KMessageBox", "Do not show this message again"),
                      &checkboxResult,
                      options);

    if (checkboxResult) {
        saveDontShowAgainContinue(dontShowAgainName);
    }
}

void informationList(QWidget *parent, const QString &text, const QStringList &strlist, const QString &title, const QString &dontShowAgainName, Options options)
{
    informationListInternal(new QDialog(parent), text, strlist, title, dontShowAgainName, options);
}

/// @private Prevent kapidox's doxygen config pick up this namespace method
static ButtonCode messageBoxInternal(QDialog *dialog,
                                     DialogType type,
                                     const QString &text,
                                     const QString &title,
                                     const KGuiItem &primaryAction,
                                     const KGuiItem &secondaryAction,
                                     const KGuiItem &cancelAction,
                                     const QString &dontShow,
                                     Options options)
{
    switch (type) {
    case QuestionTwoActions:
        return questionTwoActionsListInternal(dialog, text, QStringList(), title, primaryAction, secondaryAction, dontShow, options);
    case QuestionTwoActionsCancel:
        return questionTwoActionsCancelInternal(dialog, text, title, primaryAction, secondaryAction, cancelAction, dontShow, options);
    case WarningTwoActions:
        return warningTwoActionsListInternal(dialog, text, QStringList(), title, primaryAction, secondaryAction, dontShow, options);
    case WarningContinueCancel:
        return warningContinueCancelListInternal(dialog,
                                                 text,
                                                 QStringList(),
                                                 title,
                                                 KGuiItem(primaryAction.text()),
                                                 cancelAction,
                                                 dontShow,
                                                 options,
                                                 QString());
    case WarningTwoActionsCancel:
        return warningTwoActionsCancelListInternal(dialog, text, QStringList(), title, primaryAction, secondaryAction, cancelAction, dontShow, options);
    case Information:
        informationListInternal(dialog, text, QStringList(), title, dontShow, options);
        return KMessageBox::Ok;

    case Error:
        errorListInternal(dialog, text, QStringList(), title, options);
        return KMessageBox::Ok;
    }
    return KMessageBox::Cancel;
}

ButtonCode messageBox(QWidget *parent,
                      DialogType type,
                      const QString &text,
                      const QString &title,
                      const KGuiItem &primaryAction,
                      const KGuiItem &secondaryAction,
                      const KGuiItem &buttonCancel,
                      const QString &dontShow,
                      Options options)
{
    return messageBoxInternal(new QDialog(parent), type, text, title, primaryAction, secondaryAction, buttonCancel, dontShow, options);
}

ButtonCode questionTwoActionsWId(WId parent_id,
                                 const QString &text,
                                 const QString &title,
                                 const KGuiItem &primaryAction,
                                 const KGuiItem &secondaryAction,
                                 const QString &dontAskAgainName,
                                 Options options)
{
    return questionTwoActionsListWId(parent_id, text, QStringList(), title, primaryAction, secondaryAction, dontAskAgainName, options);
}

ButtonCode questionTwoActionsListWId(WId parent_id,
                                     const QString &text,
                                     const QStringList &strlist,
                                     const QString &title,
                                     const KGuiItem &primaryAction,
                                     const KGuiItem &secondaryAction,
                                     const QString &dontAskAgainName,
                                     Options options)
{
    return questionTwoActionsListInternal(createWIdDialog(parent_id), text, strlist, title, primaryAction, secondaryAction, dontAskAgainName, options);
}

ButtonCode questionTwoActionsCancelWId(WId parent_id,
                                       const QString &text,
                                       const QString &title,
                                       const KGuiItem &primaryAction,
                                       const KGuiItem &secondaryAction,
                                       const KGuiItem &cancelAction,
                                       const QString &dontAskAgainName,
                                       Options options)
{
    return questionTwoActionsCancelInternal(createWIdDialog(parent_id), text, title, primaryAction, secondaryAction, cancelAction, dontAskAgainName, options);
}

ButtonCode warningTwoActionsWId(WId parent_id,
                                const QString &text,
                                const QString &title,
                                const KGuiItem &primaryAction,
                                const KGuiItem &secondaryAction,
                                const QString &dontAskAgainName,
                                Options options)
{
    return warningTwoActionsListWId(parent_id, text, QStringList(), title, primaryAction, secondaryAction, dontAskAgainName, options);
}

ButtonCode warningTwoActionsListWId(WId parent_id,
                                    const QString &text,
                                    const QStringList &strlist,
                                    const QString &title,
                                    const KGuiItem &primaryAction,
                                    const KGuiItem &secondaryAction,
                                    const QString &dontAskAgainName,
                                    Options options)
{
    return warningTwoActionsListInternal(createWIdDialog(parent_id), text, strlist, title, primaryAction, secondaryAction, dontAskAgainName, options);
}

ButtonCode warningContinueCancelWId(WId parent_id,
                                    const QString &text,
                                    const QString &title,
                                    const KGuiItem &buttonContinue,
                                    const KGuiItem &buttonCancel,
                                    const QString &dontAskAgainName,
                                    Options options)
{
    return warningContinueCancelListWId(parent_id, text, QStringList(), title, buttonContinue, buttonCancel, dontAskAgainName, options);
}

ButtonCode warningContinueCancelListWId(WId parent_id,
                                        const QString &text,
                                        const QStringList &strlist,
                                        const QString &title,
                                        const KGuiItem &buttonContinue,
                                        const KGuiItem &buttonCancel,
                                        const QString &dontAskAgainName,
                                        Options options)
{
    return warningContinueCancelListInternal(createWIdDialog(parent_id),
                                             text,
                                             strlist,
                                             title,
                                             buttonContinue,
                                             buttonCancel,
                                             dontAskAgainName,
                                             options,
                                             QString());
}

ButtonCode warningTwoActionsCancelWId(WId parent_id,
                                      const QString &text,
                                      const QString &title,
                                      const KGuiItem &primaryAction,
                                      const KGuiItem &secondaryAction,
                                      const KGuiItem &buttonCancel,
                                      const QString &dontAskAgainName,
                                      Options options)
{
    return warningTwoActionsCancelListWId(parent_id, text, QStringList(), title, primaryAction, secondaryAction, buttonCancel, dontAskAgainName, options);
}

ButtonCode warningTwoActionsCancelListWId(WId parent_id,
                                          const QString &text,
                                          const QStringList &strlist,
                                          const QString &title,
                                          const KGuiItem &primaryAction,
                                          const KGuiItem &secondaryAction,
                                          const KGuiItem &buttonCancel,
                                          const QString &dontAskAgainName,
                                          Options options)
{
    return warningTwoActionsCancelList(createWIdDialog(parent_id),
                                       text,
                                       strlist,
                                       title,
                                       primaryAction,
                                       secondaryAction,
                                       buttonCancel,
                                       dontAskAgainName,
                                       options);
}

void errorWId(WId parent_id, const QString &text, const QString &title, Options options)
{
    errorListWId(parent_id, text, QStringList(), title, options);
}

void errorListWId(WId parent_id, const QString &text, const QStringList &strlist, const QString &title, Options options)
{
    errorListInternal(createWIdDialog(parent_id), text, strlist, title, options);
}

void detailedErrorWId(WId parent_id, const QString &text, const QString &details, const QString &title, Options options)
{
    detailedErrorInternal(createWIdDialog(parent_id), text, details, title, KStandardGuiItem::ok(), options);
}

void detailedErrorWId(WId parent_id, const QString &text, const QString &details, const QString &title, const KGuiItem &buttonOk, Options options)
{
    detailedErrorInternal(createWIdDialog(parent_id), text, details, title, buttonOk, options);
}

void informationWId(WId parent_id, const QString &text, const QString &title, const QString &dontShowAgainName, Options options)
{
    informationListWId(parent_id, text, QStringList(), title, dontShowAgainName, options);
}

void informationListWId(WId parent_id, const QString &text, const QStringList &strlist, const QString &title, const QString &dontShowAgainName, Options options)
{
    informationListInternal(createWIdDialog(parent_id), text, strlist, title, dontShowAgainName, options);
}

ButtonCode messageBoxWId(WId parent_id,
                         DialogType type,
                         const QString &text,
                         const QString &title,
                         const KGuiItem &primaryAction,
                         const KGuiItem &secondaryAction,
                         const KGuiItem &cancelAction,
                         const QString &dontShow,
                         Options options)
{
    return messageBoxInternal(createWIdDialog(parent_id), type, text, title, primaryAction, secondaryAction, cancelAction, dontShow, options);
}

} // namespace

#include "kmessagebox.moc"
