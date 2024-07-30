/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *   SPDX-FileCopyrightText: 2009 Mathias Soeken <msoeken@informatik.uni-bremen.de>
 *   SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ModuleView.h"
#include "ExternalAppModule.h"
#include "MenuItem.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QLoggingCategory>
#include <QMap>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include <QWhatsThis>

#include <KAboutData>
#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KAuthorized>
#include <KCModule>
#include <KCModuleLoader>
#include <KColorScheme>
#include <KMessageDialog>
#include <KPageWidget>
#include <KPluginMetaData>
#include <KSharedConfig>
#include <KStandardGuiItem>
#include <KTitleWidget>
#include <LingmoActivities/ResourceInstance>

#include <cmath>

class CustomTitle : public KTitleWidget
{
public:
    explicit CustomTitle(QWidget *parent = nullptr);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void colorsChanged();
};

CustomTitle::CustomTitle(QWidget *parent)
    : KTitleWidget(parent)
{
    // Use the same left margin as QML titles for consistency (LingmoUI/AbstractPageHeader.qml)
    // 18px is Standard LingmoUI gridUnit for 10pt Noto Sans.
    // TODO: make this use a real gridUnit so it will adjust to the user's font,
    // once we have a QmlEngine object such that  using it won't risk crashes!
    setContentsMargins(18,
                       style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                       style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                       style()->pixelMetric(QStyle::PM_LayoutBottomMargin));

    colorsChanged();
}

bool CustomTitle::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::PaletteChange) {
        this->colorsChanged();
    }
    return QWidget::event(event);
}

void CustomTitle::colorsChanged()
{
    auto config = KSharedConfig::openConfig();
    auto active = KColorScheme(QPalette::Active, KColorScheme::Header, config);
    auto inactive = KColorScheme(QPalette::Inactive, KColorScheme::Header, config);
    auto disabled = KColorScheme(QPalette::Disabled, KColorScheme::Header, config);

    QPalette palette = KColorScheme::createApplicationPalette(config);

    palette.setBrush(QPalette::Active, QPalette::Window, active.background());
    palette.setBrush(QPalette::Active, QPalette::WindowText, active.foreground());
    palette.setBrush(QPalette::Disabled, QPalette::Window, disabled.background());
    palette.setBrush(QPalette::Disabled, QPalette::WindowText, disabled.foreground());
    palette.setBrush(QPalette::Inactive, QPalette::Window, inactive.background());
    palette.setBrush(QPalette::Inactive, QPalette::WindowText, inactive.foreground());

    setPalette(palette);
}

void CustomTitle::paintEvent(QPaintEvent *event)
{
    KTitleWidget::paintEvent(event);

    auto linearlyInterpolateDouble = [](double one, double two, double factor) {
        return one + (two - one) * factor;
    };

    QPainter p(this);

    const QColor window = palette().color(QPalette::Window);
    const QColor text = palette().color(QPalette::Text);
    const qreal balance = 0.2;

    const QColor separator = QColor::fromHsv(std::fmod(linearlyInterpolateDouble(window.hue(), text.hue(), balance), 360.0),
                                             qBound(0.0, linearlyInterpolateDouble(window.saturation(), text.saturation(), balance), 255.0),
                                             qBound(0.0, linearlyInterpolateDouble(window.value(), text.value(), balance), 255.0),
                                             qBound(0.0, linearlyInterpolateDouble(window.alpha(), text.alpha(), balance), 255.0));
    p.fillRect(event->rect(), window);
    p.fillRect(QRect(QPoint(0, height() - 1), QSize(width(), 1)), separator);
}

class ModuleView::Private
{
public:
    Private()
    {
    }
    QMap<KPageWidgetItem *, KCModule *> mPages;
    QMap<KPageWidgetItem *, QString> mPagesPluginIdMap;
    KPageWidget *mPageWidget = nullptr;
    CustomTitle *mCustomHeader = nullptr;
    QVBoxLayout *mLayout = nullptr;
    QDialogButtonBox *mButtons = nullptr;
    QPushButton *mApply = nullptr;
    QPushButton *mReset = nullptr;
    QPushButton *mDefault = nullptr;
    QPushButton *mHelp = nullptr;
    std::shared_ptr<QQmlEngine> engine;
    QIcon mApplyIcon;
    KMessageDialog *mResolvingChangesDialog = nullptr;
    bool pageChangeSupressed = false;
    bool mSaveStatistics = true;
    bool mDefaultsIndicatorsVisible = false;
    KCModule::Buttons mButtonMask = ~KCModule::Buttons(KCModule::NoAdditionalButton);
    KAuth::Action authAction;
};

ModuleView::ModuleView(const std::shared_ptr<QQmlEngine> &engine, QWidget *parent)
    : QWidget(parent)
    , d(new Private())
{
    auto rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    // Configure a layout first
    d->mLayout = new QVBoxLayout();
    // Create the Page Widget
    d->mPageWidget = new KPageWidget(this);
    d->mCustomHeader = new CustomTitle(this);
    d->mCustomHeader->setVisible(false);

    rootLayout->addWidget(d->mCustomHeader);
    rootLayout->addItem(d->mLayout);
    // d->mPageWidget->setPageHeader(d->mCustomHeader);
    d->mPageWidget->layout()->setContentsMargins(0, 0, 0, 0);

    // Zero out only the horizontal spacing (the vertical spacing is fine)
    auto gridLayout = qobject_cast<QGridLayout *>(d->mPageWidget->layout());

    gridLayout->setHorizontalSpacing(0);

    d->mLayout->addWidget(d->mPageWidget);
    // Create the dialog
    d->mButtons = new QDialogButtonBox(Qt::Horizontal, this);
    d->mLayout->addWidget(d->mButtons);

    // Create the buttons in it
    d->mApply = d->mButtons->addButton(QDialogButtonBox::Apply);
    d->mApplyIcon = d->mApply->icon();
    KGuiItem::assign(d->mApply, KStandardGuiItem::apply());
    d->mDefault = d->mButtons->addButton(QDialogButtonBox::RestoreDefaults);
    KGuiItem::assign(d->mDefault, KStandardGuiItem::defaults());
    d->mReset = d->mButtons->addButton(QDialogButtonBox::Reset);
    KGuiItem::assign(d->mReset, KStandardGuiItem::reset());
    d->mHelp = d->mButtons->addButton(QDialogButtonBox::Help);
    KGuiItem::assign(d->mHelp, KStandardGuiItem::help());
    // Set some more sensible tooltips
    d->mReset->setToolTip(i18n("Reset all current changes to previous values"));
    // Set Auto-Default mode ( KDE Bug #211187 )
    d->mApply->setAutoDefault(true);
    d->mDefault->setAutoDefault(true);
    d->mReset->setAutoDefault(true);
    d->mHelp->setAutoDefault(true);
    // Prevent the buttons from being used
    d->mApply->setEnabled(false);
    d->mDefault->setEnabled(false);
    d->mReset->setEnabled(false);
    d->mHelp->setEnabled(false);
    // Connect up the buttons
    connect(d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()));
    connect(d->mReset, &QAbstractButton::clicked, this, &ModuleView::moduleLoad);
    connect(d->mHelp, &QAbstractButton::clicked, this, &ModuleView::moduleHelp);
    connect(d->mDefault, &QAbstractButton::clicked, this, &ModuleView::moduleDefaults);
    // clang-format off
    connect(d->mPageWidget, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
             this, SLOT(activeModuleChanged(KPageWidgetItem*,KPageWidgetItem*)));
    // clang-format on

    connect(d->mApply, &QPushButton::clicked, this, [this] {
        if (d->authAction.isValid()) {
            KAuth::ExecuteJob *job = d->authAction.execute(KAuth::Action::AuthorizeOnlyMode);
            connect(job, &KAuth::ExecuteJob::statusChanged, this, [this](KAuth::Action::AuthStatus status) {
                authStatusChanged(status);
            });
            job->start();
        }
    });

    d->engine = engine;
}

ModuleView::~ModuleView()
{
    delete d;
}

QString ModuleView::activeModuleName() const
{
    return d->mPageWidget->currentPage() ? d->mPageWidget->currentPage()->name() : QString();
}

void ModuleView::loadModule(const QModelIndex &menuItem, const QStringList &args)
{
    if (!menuItem.isValid()) {
        return;
    }

    auto item = menuItem.data(Qt::UserRole).value<MenuItem *>();

    // if module has a main page (like in Appearance > Global Theme) we'll load that
    if (item->isLibrary() || item->isExternalAppModule()) {
        addModule(item, args);
    }
    // if module doesn't have a main page, we'll load the first subpage
    else if (menuItem.model()->rowCount(menuItem) > 0) {
        auto subpageItem = menuItem.model()->index(0, 0, menuItem).data(Qt::UserRole).value<MenuItem *>();
        addModule(subpageItem, args);
    }
}

void ModuleView::addModule(MenuItem *item, const QStringList &args)
{
    const KPluginMetaData data = item->metaData();
    if (!KAuthorized::authorizeControlModule(data.pluginId())) {
        qWarning() << "Not authorised to load module";
        return;
    }
    if (data.isHidden()) {
        return;
    }

    if (KPageWidgetItem *page = d->mPagesPluginIdMap.key(data.pluginId())) {
        activeModuleChanged(page, d->mPageWidget->currentPage());
        return;
    }

    // Create the scroller
    auto moduleScroll = new QScrollArea(this);
    // Prepare the scroll area
    moduleScroll->setWidgetResizable(true);
    moduleScroll->setFrameStyle(QFrame::NoFrame);
    moduleScroll->viewport()->setAutoFillBackground(false);
    // Create the page
    auto page = new KPageWidgetItem(moduleScroll, data.name());
    // Provide information to the users

    if (item->isExternalAppModule()) {
        auto externalWidget = new ExternalAppModule(KService::Ptr(new KService(item->metaData().fileName())));
        moduleScroll->setWidget(externalWidget);
        d->mCustomHeader->setText(item->metaData().name()); // We have to set this manually, BUG: 448672
        page->setName(QString());
    } else { // It must be a normal module then
        auto kcm = KCModuleLoader::loadModule(data, moduleScroll, QVariantList(args.begin(), args.end()), d->engine);
        moduleScroll->setWidget(kcm->widget());
        kcm->widget()->setAutoFillBackground(false);
        kcm->load();
        connect(kcm, &KCModule::needsSaveChanged, this, &ModuleView::stateChanged);
        connect(kcm, &KCModule::representsDefaultsChanged, this, [this, kcm]() {
            if (kcm == d->mPages.value(d->mPageWidget->currentPage()) && kcm->buttons() & d->mButtonMask & KCModule::Default) {
                d->mDefault->setEnabled(!kcm->representsDefaults());
            }
        });

        d->mPages.insert(page, kcm);
    }

    d->mPagesPluginIdMap.insert(page, data.pluginId());
    updatePageIconHeader(page);
    // Add the new page
    d->mPageWidget->addPage(page);
}

void ModuleView::updatePageIconHeader(KPageWidgetItem *page)
{
    if (!page) {
        // Page is invalid. Probably means we have a race condition during closure of everyone so do nothing
        return;
    }

    KCModule *kcm = d->mPages.value(page);
    if (!kcm) {
        // Seems like we have some form of a race condition going on here...
        return;
    }

    const bool isQml = kcm->inherits("KCModuleQml");

    if (!kcm->metaData().isValid()) {
        // KCModule was (incorrectly) created with a constructor that didn't store metadata
        // Never use the custom header here because we don't know the module name
        page->setHeaderVisible(!isQml);
        if (d->mPageWidget->currentPage() == page) {
            d->mCustomHeader->hide();
        }
        return;
    }

    const QString &moduleName = kcm->metaData().name();
    page->setHeader(moduleName);
    page->setIcon(QIcon::fromTheme(kcm->metaData().iconName()));

    page->setHeaderVisible(false);

    // Use the custom header only for QWidgets KCMs on Sidebar mode
    // Only affect visibility if it's the current page
    if (d->mPageWidget->currentPage() == page) {
        if (!isQml) {
            d->mCustomHeader->setText(moduleName); // also includes show()
        } else {
            d->mCustomHeader->hide();
        }
    }
}

bool ModuleView::resolveChanges()
{
    KCModule *kcm = d->mPages.value(d->mPageWidget->currentPage());
    return resolveChanges(kcm);
}

bool ModuleView::resolveChanges(KCModule *kcm)
{
    if (!kcm || !kcm->needsSave()) {
        return true;
    }

    // if we are already resolving changes handle it like a cancel
    if (d->mResolvingChangesDialog) {
        d->mResolvingChangesDialog->reject();
    }

    // Let the user decide
    d->mResolvingChangesDialog = new KMessageDialog(KMessageDialog::WarningTwoActionsCancel,
                                                    i18n("The settings of the current module have changed.\n"
                                                         "Do you want to apply the changes or discard them?"),
                                                    this);
    d->mResolvingChangesDialog->setAttribute(Qt::WA_DeleteOnClose);
    d->mResolvingChangesDialog->setButtons(KStandardGuiItem::apply(), KStandardGuiItem::discard(), KStandardGuiItem::cancel());
    d->mResolvingChangesDialog->setCaption(i18n("Apply Settings"));
    d->mResolvingChangesDialog->setIcon(QIcon()); // Use default message box warning icon.
    int result = d->mResolvingChangesDialog->exec();
    d->mResolvingChangesDialog = nullptr;

    switch (result) {
    case KMessageDialog::PrimaryAction:
        return moduleSave(kcm);
    case KMessageDialog::SecondaryAction:
        kcm->load();
        return true;
    case KMessageDialog::Cancel:
        return false;
    default:
        Q_ASSERT(false);
        return false;
    }
}

void ModuleView::closeModules()
{
    d->pageChangeSupressed = true;
    d->authAction = KAuth::Action();
    for (auto page = d->mPagesPluginIdMap.cbegin(); page != d->mPagesPluginIdMap.cend(); ++page) {
        // Delete the KCM first, because e.g. the KFontInst KCM accesses it's widgets in the destructor
        delete d->mPages.value(page.key());
        d->mPageWidget->removePage(page.key());
    }

    d->mPages.clear();
    d->mPagesPluginIdMap.clear();
    d->pageChangeSupressed = false;
}

bool ModuleView::moduleSave()
{
    KCModule *moduleProxy = d->mPages.value(d->mPageWidget->currentPage());
    return moduleSave(moduleProxy);
}

bool ModuleView::moduleSave(KCModule *module)
{
    if (!module) {
        return false;
    }

    module->save();
    Q_EMIT moduleSaved();
    return true;
}

void ModuleView::moduleLoad()
{
    KCModule *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (activeModule) {
        activeModule->load();
    }
}

void ModuleView::moduleDefaults()
{
    KCModule *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (activeModule) {
        activeModule->defaults();
    }
}

void ModuleView::moduleHelp()
{
    KCModule *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (!activeModule) {
        return;
    }

    const QString docPath = activeModule->metaData().value(QStringLiteral("X-DocPath"));
    if (docPath.isEmpty()) {
        return;
    }

    // UrlHandler from KGUIAddons sets a handler for help:/ urls, which opens khelpcenter
    // if it's available or falls back to opening the relevant page at docs.kde.org
    QDesktopServices::openUrl(QUrl(QStringLiteral("help:/") + docPath));
}

void ModuleView::activeModuleChanged(KPageWidgetItem *current, KPageWidgetItem *previous)
{
    d->mPageWidget->blockSignals(true);
    d->mPageWidget->setCurrentPage(previous);
    KCModule *previousModule = d->mPages.value(previous);
    if (resolveChanges(previousModule)) {
        d->mPageWidget->setCurrentPage(current);
    }
    d->mPageWidget->blockSignals(false);
    if (d->pageChangeSupressed) {
        return;
    }

    // We need to get the state of the now active module
    stateChanged();

    KCModule *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (!activeModule) {
        return;
    }

    // TODO: if we'll ever need statistics for kinfocenter modules, save them with an URL like "kinfo:"
    if (d->mSaveStatistics && activeModule->metaData().pluginId() != QStringLiteral("kcm_landingpage")) {
        KActivities::ResourceInstance::notifyAccessed(QUrl(QStringLiteral("kcm:") + activeModule->metaData().pluginId()),
                                                      QStringLiteral("org.kde.systemsettings"));
    }

    d->mLayout->setContentsMargins(0, 0, 0, 0);
    d->mLayout->setSpacing(0);
    d->mButtons->setContentsMargins(style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                    style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                                    style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                    style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
    d->mButtons->setProperty("_ocean_force_frame", true);
    d->mPageWidget->layout()->setSpacing(0);
    if (auto titleWidget = qobject_cast<KTitleWidget *>(d->mPageWidget->pageHeader())) {
        // 18px is Standard LingmoUI gridUnit for 10pt Noto Sans.
        // TODO: make this use a real gridUnit so it will adjust to the user's font,
        // once we have a QmlEngine object such that  using it won't risk crashes!
        titleWidget->layout()->setContentsMargins(18,
                                                  style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                                  style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                                  style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
    }

    updatePageIconHeader(current);
    moduleShowDefaultsIndicators(d->mDefaultsIndicatorsVisible);
}

void ModuleView::stateChanged()
{
    updatePageIconHeader(d->mPageWidget->currentPage());
    updateButtons();

    KCModule *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    Q_EMIT moduleChanged(activeModule && activeModule->needsSave());
}

void ModuleView::updateButtons()
{
    KCModule *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    bool change = false;
    bool defaulted = false;
    KCModule::Buttons buttons = KCModule::NoAdditionalButton;

    if (activeModule) {
        buttons = activeModule->buttons() & d->mButtonMask;
        change = activeModule->needsSave();
        defaulted = activeModule->representsDefaults();

        d->authAction = KAuth::Action(activeModule->authActionName());
        authStatusChanged(d->authAction.status());

        // Do not display Help button if there is no docPath available
        if (activeModule->metaData().value(QStringLiteral("X-DocPath")).isEmpty()) {
            buttons &= ~KCModule::Help;
        }

        disconnect(d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()));
        connect(d->mApply, SIGNAL(clicked()), this, SLOT(moduleSave()));
    } else {
        d->authAction = KAuth::Action();
    }

    d->mDefault->setEnabled(!defaulted);
    d->mDefault->setVisible(buttons & KCModule::Default);
    d->mApply->setEnabled(change);
    d->mApply->setVisible(buttons & KCModule::Apply);
    d->mReset->setEnabled(change);
    d->mReset->setVisible(buttons & KCModule::Apply);
    d->mHelp->setEnabled(buttons & KCModule::Help);
    d->mHelp->setVisible(buttons & KCModule::Help);

    d->mButtons->setVisible(buttons != KCModule::NoAdditionalButton);
}

void ModuleView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1 && d->mHelp->isVisible() && d->mHelp->isEnabled()) {
        d->mHelp->animateClick();
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Escape) {
        event->accept();
        Q_EMIT closeRequest();
        return;
    } else if (event->key() == Qt::Key_F1 && event->modifiers() == Qt::ShiftModifier) {
        QWhatsThis::enterWhatsThisMode();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void ModuleView::setFaceType(KPageView::FaceType type)
{
    d->mPageWidget->setFaceType(type);
}

void ModuleView::setSaveStatistics(bool save)
{
    d->mSaveStatistics = save;
}

bool ModuleView::saveStatistics() const
{
    return d->mSaveStatistics;
}

void ModuleView::setApplyVisible(bool visible)
{
    d->mButtonMask.setFlag(KCModule::Apply, visible);
    updateButtons();
}

bool ModuleView::isApplyVisible() const
{
    return d->mApply->isVisible();
}

void ModuleView::setDefaultsVisible(bool visible)
{
    d->mButtonMask.setFlag(KCModule::Default, visible);
    updateButtons();
}

bool ModuleView::isDefaultsVisible() const
{
    return d->mDefault->isVisible();
}

bool ModuleView::isResetVisible() const
{
    return d->mReset->isVisible();
}

void ModuleView::moduleShowDefaultsIndicators(bool show)
{
    d->mDefaultsIndicatorsVisible = show;
    KCModule *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (activeModule) {
        activeModule->setDefaultsIndicatorsVisible(show);
    }
}

void ModuleView::setHeaderHeight(qreal height)
{
    if (height == d->mCustomHeader->minimumHeight()) {
        return;
    }

    d->mCustomHeader->setMinimumHeight(height);
}

qreal ModuleView::headerHeight() const
{
    return d->mCustomHeader->minimumHeight();
}

void ModuleView::setActiveModule(const QString &pluginId)
{
    const auto pageList = d->mPagesPluginIdMap.keys();
    for (const auto page : pageList) {
        if (d->mPagesPluginIdMap.value(page) == pluginId) {
            d->mPageWidget->setCurrentPage(page);
            break;
        }
    }
}

void ModuleView::requestActivation(const QVariantList &args)
{
    KCModule *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (activeModule) {
        Q_EMIT activeModule->activationRequested(args);
    }
}

KPluginMetaData ModuleView::activeModuleMetadata() const
{
    KCModule *activeModule = d->mPages.value(d->mPageWidget->currentPage());
    if (!activeModule) {
        return {};
    }
    return activeModule->metaData();
}

void ModuleView::authStatusChanged(KAuth::Action::AuthStatus status)
{
    switch (status) {
    case KAuth::Action::AuthorizedStatus:
        d->mApply->setEnabled(true);
        d->mApply->setIcon(d->mApplyIcon);
        break;
    case KAuth::Action::AuthRequiredStatus:
        d->mApply->setEnabled(true);
        d->mApply->setIcon(QIcon::fromTheme(QStringLiteral("dialog-password")));
        break;
    default:
        d->mApply->setEnabled(false);
        d->mApply->setIcon(d->mApplyIcon);
    }
}

#include "moc_ModuleView.cpp"
