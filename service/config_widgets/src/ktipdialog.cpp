/*
    SPDX-FileCopyrightText: 2000-2003 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    SPDX-FileCopyrightText: 2000-2003 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2000-2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>

    SPDX-License-Identifier: MIT
*/

#include "ktipdialog.h"
#include "kconfigwidgets_debug.h"

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 83)

#include <QApplication>
#include <QCheckBox>
#include <QFile>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QScreen>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KRandom>
#include <KSeparator>
#include <KSharedConfig>
#include <KStandardGuiItem>

class KTipDatabasePrivate
{
public:
    void loadTips(const QString &tipFile);
    void addTips(const QString &tipFile);

    QStringList tips;
    int currentTip;
};

void KTipDatabasePrivate::loadTips(const QString &tipFile)
{
    tips.clear();
    addTips(tipFile);
}

/**
 * If you change something here, please update the script
 * preparetips5, which depends on extracting exactly the same
 * text as done here.
 */
void KTipDatabasePrivate::addTips(const QString &tipFile)
{
    const QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, tipFile);

    if (fileName.isEmpty()) {
        qCDebug(KCONFIG_WIDGETS_LOG) << "KTipDatabase::addTips: can't find '" << tipFile << "' in standard dirs";
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(KCONFIG_WIDGETS_LOG) << "KTipDatabase::addTips: can't open '" << fileName << "' for reading";
        return;
    }

    QByteArray data = file.readAll();
    QString content = QString::fromUtf8(data.constData(), data.size());
    const QRegularExpression rx(QStringLiteral("\\n{2,}"));

    int pos = -1;
    const QLatin1Char newline('\n');
    while ((pos = content.indexOf(QLatin1String("<html>"), pos + 1, Qt::CaseInsensitive)) != -1) {
        /**
         * To make translations work, tip extraction here must exactly
         * match what is done by the preparetips5 script.
         */
        QString tip = content.mid(pos + 6, content.indexOf(QLatin1String("</html>"), pos, Qt::CaseInsensitive) - pos - 6).replace(rx, QStringLiteral("\n"));

        if (!tip.endsWith(newline)) {
            tip += newline;
        }

        if (tip.startsWith(newline)) {
            tip.remove(0, 1);
        }

        if (tip.isEmpty()) {
            qCDebug(KCONFIG_WIDGETS_LOG) << "Empty tip found! Skipping! " << pos;
            continue;
        }

        tips.append(tip);
    }

    file.close();
}

KTipDatabase::KTipDatabase(const QString &_tipFile)
    : d(new KTipDatabasePrivate)
{
    QString tipFile = _tipFile;

    if (tipFile.isEmpty()) {
        tipFile = QCoreApplication::applicationName() + QStringLiteral("/tips");
    }

    d->loadTips(tipFile);

    if (!d->tips.isEmpty()) {
        d->currentTip = QRandomGenerator::global()->bounded(d->tips.count());
    }
}

KTipDatabase::KTipDatabase(const QStringList &tipsFiles)
    : d(new KTipDatabasePrivate)
{
    if (tipsFiles.isEmpty() || ((tipsFiles.count() == 1) && tipsFiles.first().isEmpty())) {
        d->addTips(QCoreApplication::applicationName() + QStringLiteral("/tips"));
    } else {
        for (QStringList::ConstIterator it = tipsFiles.begin(); it != tipsFiles.end(); ++it) {
            d->addTips(*it);
        }
    }

    if (!d->tips.isEmpty()) {
        d->currentTip = QRandomGenerator::global()->bounded(d->tips.count());
    }
}

KTipDatabase::~KTipDatabase() = default;

void KTipDatabase::nextTip()
{
    if (d->tips.isEmpty()) {
        return;
    }

    d->currentTip += 1;

    if (d->currentTip >= d->tips.count()) {
        d->currentTip = 0;
    }
}

void KTipDatabase::prevTip()
{
    if (d->tips.isEmpty()) {
        return;
    }

    d->currentTip -= 1;

    if (d->currentTip < 0) {
        d->currentTip = d->tips.count() - 1;
    }
}

QString KTipDatabase::tip() const
{
    if (d->tips.isEmpty()) {
        return QString();
    }

    return d->tips[d->currentTip];
}

class KTipDialogPrivate
{
public:
    KTipDialogPrivate(KTipDialog *qq)
        : q(qq)
    {
    }
    ~KTipDialogPrivate()
    {
        delete database;
    }

    void _k_nextTip();
    void _k_prevTip();
    void _k_showOnStart(bool);

    KTipDialog *const q;
    KTipDatabase *database;
    QCheckBox *tipOnStart;
    QTextBrowser *tipText;

    static KTipDialog *mInstance;
};

KTipDialog *KTipDialogPrivate::mInstance = nullptr;

void KTipDialogPrivate::_k_prevTip()
{
    database->prevTip();
    tipText->setHtml(
        QStringLiteral("<html><body>%1</body></html>").arg(i18nd(KLocalizedString::applicationDomain().constData(), database->tip().toUtf8().constData())));
}

void KTipDialogPrivate::_k_nextTip()
{
    database->nextTip();
    tipText->setHtml(
        QStringLiteral("<html><body>%1</body></html>").arg(i18nd(KLocalizedString::applicationDomain().constData(), database->tip().toUtf8().constData())));
}

void KTipDialogPrivate::_k_showOnStart(bool on)
{
    q->setShowOnStart(on);
}

KTipDialog::KTipDialog(KTipDatabase *database, QWidget *parent)
    : QDialog(parent)
    , d(new KTipDialogPrivate(this))
{
    setWindowTitle(i18nc("@title:window", "Tip of the Day"));

    /**
     * Parent is 0L when TipDialog is used as a mainWidget. This should
     * be the case only in ktip, so let's use the ktip layout.
     */
    bool isTipDialog = (parent != nullptr);

    d->database = database;

    setWindowIcon(QIcon::fromTheme(QStringLiteral("ktip")));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    if (isTipDialog) {
        QLabel *titleLabel = new QLabel(this);
        titleLabel->setText(i18nc("@title", "Did you know...?\n"));
        titleLabel->setFont(QFont(qApp->font().family(), 20, QFont::Bold));
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
    }

    QHBoxLayout *browserLayout = new QHBoxLayout();
    mainLayout->addLayout(browserLayout);

    d->tipText = new QTextBrowser(this);

    d->tipText->setOpenExternalLinks(true);

    d->tipText->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    QStringList paths;
    paths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory)
          << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kdewizard/pics"), QStandardPaths::LocateDirectory);

    d->tipText->setSearchPaths(paths);

    d->tipText->setFrameStyle(QFrame::NoFrame);
    d->tipText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPalette tipPal(d->tipText->palette());
    tipPal.setColor(QPalette::Base, Qt::transparent);
    tipPal.setColor(QPalette::Text, tipPal.color(QPalette::WindowText));
    d->tipText->setPalette(tipPal);

    browserLayout->addWidget(d->tipText);

    QLabel *label = new QLabel(this);
    label->setPixmap(QStringLiteral(":/kconfigwidgets/pics/ktip-bulb.png"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    browserLayout->addWidget(label);

    if (!isTipDialog) {
        resize(520, 280);
        QSize sh = size();

        QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
        if (screen) {
            const QRect rect = screen->geometry();
            move(rect.x() + (rect.width() - sh.width()) / 2, rect.y() + (rect.height() - sh.height()) / 2);
        }
    }

    KSeparator *sep = new KSeparator(Qt::Horizontal);
    mainLayout->addWidget(sep);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    mainLayout->addLayout(buttonLayout);

    d->tipOnStart = new QCheckBox(i18nc("@option:check", "&Show tips on startup"));
    buttonLayout->addWidget(d->tipOnStart, 1);

    QPushButton *prev = new QPushButton;
    KGuiItem::assign(prev, KStandardGuiItem::back(KStandardGuiItem::UseRTL));
    prev->setText(i18nc("@action:button Goes to previous tip", "&Previous"));
    buttonLayout->addWidget(prev);

    QPushButton *next = new QPushButton;
    KGuiItem::assign(next, KStandardGuiItem::forward(KStandardGuiItem::UseRTL));
    next->setText(i18nc("@action:button Goes to next tip, opposite to previous", "&Next"));
    buttonLayout->addWidget(next);

    QPushButton *ok = new QPushButton;
    KGuiItem::assign(ok, KStandardGuiItem::close());
    ok->setDefault(true);
    buttonLayout->addWidget(ok);

    KConfigGroup config(KSharedConfig::openConfig(), "TipOfDay");
    d->tipOnStart->setChecked(config.readEntry("RunOnStart", true));

    connect(next, &QPushButton::clicked, this, [this]() {
        d->_k_nextTip();
    });
    connect(prev, &QPushButton::clicked, this, [this]() {
        d->_k_prevTip();
    });
    connect(ok, &QAbstractButton::clicked, this, &QDialog::accept);
    connect(d->tipOnStart, &QCheckBox::toggled, this, [this](bool state) {
        d->_k_showOnStart(state);
    });

    ok->setFocus();

    d->_k_nextTip();
}

KTipDialog::~KTipDialog()
{
    if (KTipDialogPrivate::mInstance == this) {
        KTipDialogPrivate::mInstance = nullptr;
    }
}

/**
 * use the one with a parent window as parameter instead of this one
 * or you will get focus problems
 */
void KTipDialog::showTip(const QString &tipFile, bool force)
{
    showTip(nullptr, tipFile, force);
}

void KTipDialog::showTip(QWidget *parent, const QString &tipFile, bool force)
{
    showMultiTip(parent, QStringList(tipFile), force);
}

void KTipDialog::showMultiTip(QWidget *parent, const QStringList &tipFiles, bool force)
{
    KConfigGroup configGroup(KSharedConfig::openConfig(), "TipOfDay");

    const bool runOnStart = configGroup.readEntry("RunOnStart", true);

    if (!force) {
        if (!runOnStart) {
            return;
        }

        // showing the tooltips on startup suggests the tooltip
        // will be shown *each time* on startup, not $random days later
        // TODO either remove or uncomment this code, but make the situation clear
        /*bool hasLastShown = configGroup.hasKey( "TipLastShown" );
        if ( hasLastShown ) {
          const int oneDay = 24 * 60 * 60;
          QDateTime lastShown = configGroup.readEntry( "TipLastShown", QDateTime() );

          // Show tip roughly once a week
          if ( lastShown.secsTo( QDateTime::currentDateTime() ) < (oneDay + (QRandomGenerator::global()->bounded(10 * oneDay))) )
            return;
        }

        configGroup.writeEntry( "TipLastShown", QDateTime::currentDateTime() );

        if ( !hasLastShown )
          return; // Don't show tip on first start*/
    }

    if (!KTipDialogPrivate::mInstance) {
        KTipDialogPrivate::mInstance = new KTipDialog(new KTipDatabase(tipFiles), parent);
    } else
    // The application might have changed the RunOnStart option in its own
    // configuration dialog, so we should update the checkbox.
    {
        KTipDialogPrivate::mInstance->d->tipOnStart->setChecked(runOnStart);
    }

    KTipDialogPrivate::mInstance->show();
    KTipDialogPrivate::mInstance->raise();
}

void KTipDialog::setShowOnStart(bool on)
{
    KConfigGroup config(KSharedConfig::openConfig(), "TipOfDay");
    config.writeEntry("RunOnStart", on);
}

bool KTipDialog::eventFilter(QObject *object, QEvent *event)
{
    if (object == d->tipText && event->type() == QEvent::KeyPress
        && (((QKeyEvent *)event)->key() == Qt::Key_Return || ((QKeyEvent *)event)->key() == Qt::Key_Space)) {
        accept();
    }

    /**
     * If the user presses Return or Space, we close the dialog as if the
     * default button was pressed even if the QTextBrowser has the keyboard
     * focus. This could have the bad side-effect that the user cannot use the
     * keyboard to open urls in the QTextBrowser, so we just let it handle
     * the key event _additionally_. (Antonio)
     */

    return QWidget::eventFilter(object, event);
}

#include "moc_ktipdialog.cpp"
#endif
