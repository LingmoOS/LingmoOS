/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  SPDX-FileCopyrightText: 2001 Michael v.Ostheim <MvOstheim@web.de>
 */

#include "kgamma.h"

#include "gammactrl.h"
#include "xf86configpath.h"
#include <unistd.h>

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QProcess>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVBoxLayout>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include "xvidextwrap.h"

extern "C" {
bool test_kgamma()
{
    bool retval;
    (void)new XVidExtWrap(&retval, nullptr);
    return retval;
}
}

K_PLUGIN_CLASS_WITH_JSON(KGamma, "kcm_kgamma.json")

KGamma::KGamma(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , rootProcess(nullptr)
{
    bool ok;
    GammaCorrection = false;
    xv = new XVidExtWrap(&ok, nullptr);
    if (ok) { /* KDE 4: Uneccessary test, when all KCM wrappers do conditional loading */
        xv->getGamma(XVidExtWrap::Red, &ok);
        if (ok) {
            ScreenCount = xv->_ScreenCount();
            currentScreen = xv->getScreen();
            xv->setGammaLimits(0.4, 3.5);

            for (int i = 0; i < ScreenCount; i++) {
                assign << 0;
                rgamma << QString();
                ggamma << QString();
                bgamma << QString();

                // Store the current gamma values
                xv->setScreen(i);
                rbak << xv->getGamma(XVidExtWrap::Red);
                gbak << xv->getGamma(XVidExtWrap::Green);
                bbak << xv->getGamma(XVidExtWrap::Blue);
            }
            xv->setScreen(currentScreen);

            rootProcess = new QProcess;
            GammaCorrection = true;
            setupUI();
            saved = false;

            if (!loadSettings()) { // try to load gamma values from config file
                // if failed, take current gamma values
                for (int i = 0; i < ScreenCount; i++) {
                    rgamma[i].setNum(rbak[i], 'f', 2);
                    ggamma[i].setNum(gbak[i], 'f', 2);
                    bgamma[i].setNum(bbak[i], 'f', 2);
                }
            }
            load();
        }
    }
    if (!GammaCorrection) { // something is wrong, show only error message
        setupUI();
    }
}

KGamma::~KGamma()
{
    // Restore the old gamma settings, if the user has not saved
    // and there is no valid kgammarc.
    // Existing user settings overwrite system settings
    if (GammaCorrection) {
        // Do not emit signals during destruction (bug 221611)
        bool blocked = blockSignals(true);
        if (loadUserSettings()) {
            load();
        } else if (!saved) {
            for (int i = 0; i < ScreenCount; i++) {
                xv->setScreen(i);
                xv->setGamma(XVidExtWrap::Red, rbak[i]);
                xv->setGamma(XVidExtWrap::Green, gbak[i]);
                xv->setGamma(XVidExtWrap::Blue, bbak[i]);
            }
        }
        delete rootProcess;
        blockSignals(blocked);
    }
    delete xv;
}

/** User interface */
void KGamma::setupUI()
{
    QBoxLayout *topLayout = new QVBoxLayout(widget());
    topLayout->setContentsMargins(0, 0, 0, 0);

    if (GammaCorrection) {
        QHBoxLayout *hbox = new QHBoxLayout();
        topLayout->addLayout(hbox);
        QLabel *label = new QLabel(i18n("&Select test picture:"), widget());
        QComboBox *combo = new QComboBox(widget());
        label->setBuddy(combo);

        QStringList list;
        list << i18n("Gray Scale") << i18n("RGB Scale") << i18n("CMY Scale") << i18n("Dark Gray") << i18n("Mid Gray") << i18n("Light Gray");
        combo->addItems(list);

        hbox->addWidget(label);
        hbox->addWidget(combo);
        hbox->addStretch();

        QStackedWidget *stack = new QStackedWidget(widget());
        stack->setFrameStyle(QFrame::Box | QFrame::Raised);

        connect(combo, QOverload<int>::of(&QComboBox::activated), stack, &QStackedWidget::setCurrentIndex);

        QLabel *pic1 = new QLabel(stack);
        pic1->setMinimumSize(530, 171);
        pic1->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kgamma/pics/greyscale.png"))));
        pic1->setAlignment(Qt::AlignCenter);
        stack->insertWidget(0, pic1);

        QLabel *pic2 = new QLabel(stack);
        pic2->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kgamma/pics/rgbscale.png"))));
        pic2->setAlignment(Qt::AlignCenter);
        stack->insertWidget(1, pic2);

        QLabel *pic3 = new QLabel(stack);
        pic3->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kgamma/pics/cmyscale.png"))));
        pic3->setAlignment(Qt::AlignCenter);
        stack->insertWidget(2, pic3);

        QLabel *pic4 = new QLabel(stack);
        pic4->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kgamma/pics/darkgrey.png"))));
        pic4->setAlignment(Qt::AlignCenter);
        stack->insertWidget(3, pic4);

        QLabel *pic5 = new QLabel(stack);
        pic5->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kgamma/pics/midgrey.png"))));
        pic5->setAlignment(Qt::AlignCenter);
        stack->insertWidget(4, pic5);

        QLabel *pic6 = new QLabel(stack);
        pic6->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kgamma/pics/lightgrey.png"))));
        pic6->setAlignment(Qt::AlignCenter);
        stack->insertWidget(5, pic6);

        topLayout->addWidget(stack, 10);

        // Sliders for gamma correction

        QLabel *gammalabel = new QLabel(widget());
        gammalabel->setText(i18n("Gamma:"));

        QLabel *redlabel = new QLabel(widget());
        redlabel->setText(i18n("Red:"));

        QLabel *greenlabel = new QLabel(widget());
        greenlabel->setText(i18n("Green:"));

        QLabel *bluelabel = new QLabel(widget());
        bluelabel->setText(i18n("Blue:"));

        gctrl = new GammaCtrl(widget(), xv);
        connect(gctrl, &GammaCtrl::gammaChanged, this, &KCModule::markAsChanged);
        connect(gctrl, &GammaCtrl::gammaChanged, this, &KGamma::SyncScreens);
        gammalabel->setBuddy(gctrl);

        rgctrl = new GammaCtrl(widget(), xv, XVidExtWrap::Red);
        connect(rgctrl, &GammaCtrl::gammaChanged, this, &KCModule::markAsChanged);
        connect(rgctrl, &GammaCtrl::gammaChanged, this, &KGamma::SyncScreens);
        connect(gctrl, SIGNAL(gammaChanged(int)), rgctrl, SLOT(setCtrl(int)));
        connect(rgctrl, &GammaCtrl::gammaChanged, gctrl, &GammaCtrl::suspend);
        redlabel->setBuddy(rgctrl);

        ggctrl = new GammaCtrl(widget(), xv, XVidExtWrap::Green);
        connect(ggctrl, &GammaCtrl::gammaChanged, this, &KCModule::markAsChanged);
        connect(ggctrl, &GammaCtrl::gammaChanged, this, &KGamma::SyncScreens);
        connect(gctrl, SIGNAL(gammaChanged(int)), ggctrl, SLOT(setCtrl(int)));
        connect(ggctrl, &GammaCtrl::gammaChanged, gctrl, &GammaCtrl::suspend);
        greenlabel->setBuddy(ggctrl);

        bgctrl = new GammaCtrl(widget(), xv, XVidExtWrap::Blue);
        connect(bgctrl, &GammaCtrl::gammaChanged, this, &KCModule::markAsChanged);
        connect(bgctrl, &GammaCtrl::gammaChanged, this, &KGamma::SyncScreens);
        connect(gctrl, SIGNAL(gammaChanged(int)), bgctrl, SLOT(setCtrl(int)));
        connect(bgctrl, &GammaCtrl::gammaChanged, gctrl, &GammaCtrl::suspend);
        bluelabel->setBuddy(bgctrl);

        QFormLayout *form = new QFormLayout;
        form->addRow(gammalabel, gctrl);
        form->addItem(new QSpacerItem(0, gammalabel->sizeHint().height() / 3));
        form->addRow(redlabel, rgctrl);
        form->addRow(greenlabel, ggctrl);
        form->addRow(bluelabel, bgctrl);

        topLayout->addLayout(form);

        // Options
        QWidget *options = new QWidget(widget());
        QHBoxLayout *optionsHBoxLayout = new QHBoxLayout(options);
        optionsHBoxLayout->setContentsMargins(0, 0, 0, 0);

        xf86cfgbox = new QCheckBox(i18n("Save settings system wide"), options);
        optionsHBoxLayout->addWidget(xf86cfgbox);
        connect(xf86cfgbox, &QAbstractButton::clicked, this, &KGamma::changeConfig);

        syncbox = new QCheckBox(i18n("Sync screens"), options);
        optionsHBoxLayout->addWidget(syncbox);
        connect(syncbox, &QAbstractButton::clicked, this, &KGamma::SyncScreens);
        connect(syncbox, &QAbstractButton::clicked, this, &KCModule::markAsChanged);

        screenselect = new QComboBox(options);
        optionsHBoxLayout->addWidget(screenselect);
        for (int i = 0; i < ScreenCount; i++) {
            screenselect->addItem(i18n("Screen %1", i + 1));
        }
        screenselect->setCurrentIndex(currentScreen);
        if (ScreenCount <= 1) {
            screenselect->setEnabled(false);
        } else {
            connect(screenselect, QOverload<int>::of(&QComboBox::activated), this, &KGamma::changeScreen);
        }

        optionsHBoxLayout->setSpacing(10);
        optionsHBoxLayout->setStretchFactor(xf86cfgbox, 10);
        optionsHBoxLayout->setStretchFactor(syncbox, 1);
        optionsHBoxLayout->setStretchFactor(screenselect, 1);

        topLayout->addWidget(options);
    } else {
        QLabel *error = new QLabel(widget());
        error->setText(
            i18n("Gamma correction is not supported by your"
                 " graphics hardware or driver."));
        error->setAlignment(Qt::AlignCenter);
        topLayout->addWidget(error);
    }
}

/** Restore latest saved gamma values */
void KGamma::load()
{
    if (GammaCorrection) {
        KConfig *config = new KConfig(QStringLiteral("kgammarc"));
        KConfigGroup group = config->group("ConfigFile");

        // save checkbox status
        if (xf86cfgbox->isChecked()) {
            group.writeEntry("use", "XF86Config");
        } else {
            group.writeEntry("use", "kgammarc");
        }

        // load syncbox status
        group = config->group("SyncBox");
        if (group.readEntry("sync") == QLatin1String("yes")) {
            syncbox->setChecked(true);
        } else {
            syncbox->setChecked(false);
        }

        config->sync();
        delete config;

        for (int i = 0; i < ScreenCount; i++) {
            xv->setScreen(i);
            if (rgamma[i] == ggamma[i] && rgamma[i] == bgamma[i]) {
                if (i == currentScreen) {
                    gctrl->setGamma(rgamma[i]);
                } else {
                    xv->setGamma(XVidExtWrap::Value, rgamma[i].toFloat());
                }
            } else {
                if (i == currentScreen) {
                    rgctrl->setGamma(rgamma[i]);
                    ggctrl->setGamma(ggamma[i]);
                    bgctrl->setGamma(bgamma[i]);
                    gctrl->suspend();
                } else {
                    xv->setGamma(XVidExtWrap::Red, rgamma[i].toFloat());
                    xv->setGamma(XVidExtWrap::Green, ggamma[i].toFloat());
                    xv->setGamma(XVidExtWrap::Blue, bgamma[i].toFloat());
                }
            }
        }
        xv->setScreen(currentScreen);

        setNeedsSave(false);
    }
}

void KGamma::save()
{
    if (GammaCorrection) {
        for (int i = 0; i < ScreenCount; i++) {
            xv->setScreen(i);
            rgamma[i] = rgctrl->gamma(2);
            ggamma[i] = ggctrl->gamma(2);
            bgamma[i] = bgctrl->gamma(2);
        }
        xv->setScreen(currentScreen);

        KConfig *config = new KConfig(QStringLiteral("kgammarc"));
        KConfigGroup group = config->group("SyncBox");
        if (syncbox->isChecked()) {
            group.writeEntry("sync", "yes");
        } else {
            group.writeEntry("sync", "no");
        }

        if (!xf86cfgbox->isChecked()) { // write gamma settings to the users config
            for (int i = 0; i < ScreenCount; i++) {
                KConfigGroup screenGroup = config->group(QStringLiteral("Screen %1").arg(i));
                screenGroup.writeEntry("rgamma", rgamma[i]);
                screenGroup.writeEntry("ggamma", ggamma[i]);
                screenGroup.writeEntry("bgamma", bgamma[i]);
            }
            KConfigGroup confGroup = config->group("ConfigFile");
            confGroup.writeEntry("use", "kgammarc");
        } else { // write gamma settings to section "Monitor" of XF86Config
            KConfigGroup x86group = config->group("ConfigFile");
            x86group.writeEntry("use", "XF86Config");

            if (!(rootProcess->state() == QProcess::Running)) {
                QString Arguments = QStringLiteral("xf86gammacfg ");
                for (int i = 0; i < ScreenCount; i++) {
                    Arguments += rgamma[assign[i]] + QLatin1Char(' ') + ggamma[assign[i]] + QLatin1Char(' ') + bgamma[assign[i]] + QLatin1Char(' ');
                }
                rootProcess->setProgram(QStandardPaths::findExecutable(QStringLiteral("kdesu")));
                rootProcess->setArguments(Arguments.split(QLatin1Char(' ')));
                rootProcess->start();
            }
        }
        config->sync();
        delete config;
        saved = true;
        setNeedsSave(false);
    }
}

void KGamma::defaults()
{
    if (GammaCorrection) {
        for (int i = 0; i < ScreenCount; i++) {
            xv->setScreen(i);
            gctrl->setGamma(QStringLiteral("1.00"));
        }
        xv->setScreen(currentScreen);
    }
    xf86cfgbox->setChecked(false);
    syncbox->setChecked(false);
}

bool KGamma::loadSettings()
{
    KConfig *config = new KConfig(QStringLiteral("kgammarc"));
    KConfigGroup grp = config->group("ConfigFile");
    QString ConfigFile(grp.readEntry("use"));
    KConfigGroup syncGroup = config->group("SyncBox");
    if (syncGroup.readEntry("sync") == QLatin1String("yes")) {
        syncbox->setChecked(true);
    }
    delete config;

    if (ConfigFile == QLatin1String("XF86Config")) { // parse XF86Config
        bool validGlobalConfig = loadSystemSettings();
        xf86cfgbox->setChecked(validGlobalConfig);
        return (validGlobalConfig);
    } else { // get gamma settings from user config
        return (loadUserSettings());
    }
}

bool KGamma::loadUserSettings()
{
    KConfig *config = new KConfig(QStringLiteral("kgammarc"));

    for (int i = 0; i < ScreenCount; i++) {
        KConfigGroup screenGroup = config->group(QStringLiteral("Screen %1").arg(i));
        rgamma[i] = screenGroup.readEntry("rgamma");
        ggamma[i] = screenGroup.readEntry("ggamma");
        bgamma[i] = screenGroup.readEntry("bgamma");
    }
    delete config;

    return (validateGammaValues());
}

bool KGamma::loadSystemSettings()
{
    QStringList Monitor, Screen, ScreenLayout, ScreenMonitor, Gamma;
    QList<int> ScreenNr;
    QString Section;
    XF86ConfigPath Path;
    QFile f(QString::fromUtf8(Path.get()));
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream t(&f);
        QString s;
        int sn = 0;
        bool gm = false;

        // Analyze Screen<->Monitor assignments of multi-head configurations
        while (!t.atEnd()) {
            s = (t.readLine()).simplified();
            QStringList words = s.split(QLatin1Char(' '));
            if (!words.empty()) {
                if (words[0] == QLatin1String("Section") && words.size() > 1) {
                    if ((Section = words[1]) == QLatin1String("\"Monitor\"")) {
                        gm = false;
                    }
                } else if (words[0] == QLatin1String("EndSection")) {
                    if (Section == QLatin1String("\"Monitor\"") && !gm) {
                        Gamma << QString();
                        gm = false;
                    }
                    Section = QString();
                } else if (words[0] == QLatin1String("Identifier") && words.size() > 1) {
                    if (Section == QLatin1String("\"Monitor\"")) {
                        Monitor << words[1];
                    } else if (Section == QLatin1String("\"Screen\"")) {
                        Screen << words[1];
                    }
                } else if (words[0] == QLatin1String("Screen") && words.size() > 1) {
                    if (Section == QLatin1String("\"ServerLayout\"")) {
                        bool ok;
                        int i = words[1].toInt(&ok);
                        if (ok && words.size() > 2) {
                            ScreenNr << i;
                            ScreenLayout << words[2];
                        } else {
                            ScreenNr << sn++;
                            ScreenLayout << words[1];
                        }
                    }
                } else if (words[0] == QLatin1String("Monitor") && words.size() > 1) {
                    if (Section == QLatin1String("\"Screen\"")) {
                        ScreenMonitor << words[1];
                    }
                } else if (words[0] == QLatin1String("Gamma")) {
                    if (Section == QLatin1String("\"Monitor\"")) {
                        Gamma << s;
                        gm = true;
                    }
                }
            }
        } // End while
        f.close();
        if (!Monitor.isEmpty() && !ScreenMonitor.isEmpty() && !ScreenLayout.isEmpty()) {
            for (int i = 0; i < ScreenCount; i++) {
                for (int j = 0; j < ScreenCount; j++) {
                    if (ScreenLayout[i] == Screen[j]) {
                        for (int k = 0; k < ScreenCount; k++) {
                            if (Monitor[k] == ScreenMonitor[j]) {
                                assign[ScreenNr[i]] = k;
                            }
                        }
                    }
                }
            }
            // Extract gamma values
            if (gm) {
                for (int i = 0; i < ScreenCount; i++) {
                    rgamma[i] = ggamma[i] = bgamma[i] = QString();

                    QStringList words = Gamma[assign[i]].split(QLatin1Char(' '));
                    QStringList::ConstIterator it = words.constBegin();
                    if (words.size() < 4) {
                        rgamma[i] = ggamma[i] = bgamma[i] = *(++it); // single gamma value
                    } else {
                        rgamma[i] = *(++it); // eventually rgb gamma values
                        ggamma[i] = *(++it);
                        bgamma[i] = *(++it);
                    }
                }
            }
        }
    }
    return (validateGammaValues());
}

bool KGamma::validateGammaValues()
{
    bool rOk, gOk, bOk, result;

    result = true;
    for (int i = 0; i < ScreenCount; i++) {
        rgamma[i].toFloat(&rOk);
        ggamma[i].toFloat(&gOk);
        bgamma[i].toFloat(&bOk);

        if (!(rOk && gOk && bOk)) {
            if (rOk) {
                ggamma[i] = bgamma[i] = rgamma[i];
            } else {
                result = false;
            }
        }
    }
    return (result);
}

void KGamma::changeConfig()
{
    bool Ok = false;

    if (xf86cfgbox->isChecked()) {
        Ok = loadSystemSettings();
    } else {
        Ok = loadUserSettings();
    }

    if (!Ok) {
        for (int i = 0; i < ScreenCount; i++) {
            xv->setScreen(i);
            rgamma[i].setNum(xv->getGamma(XVidExtWrap::Red), 'f', 2);
            ggamma[i].setNum(xv->getGamma(XVidExtWrap::Green), 'f', 2);
            bgamma[i].setNum(xv->getGamma(XVidExtWrap::Blue), 'f', 2);
        }
        xv->setScreen(currentScreen);
    }
    load();
}

void KGamma::SyncScreens()
{
    if (syncbox->isChecked()) {
        float rg = xv->getGamma(XVidExtWrap::Red);
        float gg = xv->getGamma(XVidExtWrap::Green);
        float bg = xv->getGamma(XVidExtWrap::Blue);

        for (int i = 0; i < ScreenCount; i++) {
            if (i != currentScreen) {
                xv->setScreen(i);
                xv->setGamma(XVidExtWrap::Red, rg);
                xv->setGamma(XVidExtWrap::Green, gg);
                xv->setGamma(XVidExtWrap::Blue, bg);
            }
        }
        xv->setScreen(currentScreen);
    }
}

void KGamma::changeScreen(int sn)
{
    QString red, green, blue;

    xv->setScreen(sn);
    currentScreen = sn;

    red.setNum(xv->getGamma(XVidExtWrap::Red), 'f', 2);
    green.setNum(xv->getGamma(XVidExtWrap::Green), 'f', 2);
    blue.setNum(xv->getGamma(XVidExtWrap::Blue), 'f', 2);

    gctrl->setControl(red);
    rgctrl->setControl(red);
    ggctrl->setControl(green);
    bgctrl->setControl(blue);
    if (red != green || red != blue) {
        gctrl->suspend();
    }
}

int KGamma::buttons()
{
    return Default | Apply | Help;
}

// ------------------------------------------------------------------------

extern "C" {
// Restore the user gamma settings
Q_DECL_EXPORT void kcminit()
{
    bool ok;
    XVidExtWrap xv(&ok);

    if (ok) {
        xv.setGammaLimits(0.4, 3.5);
        float rgamma, ggamma, bgamma;
        KConfig *config = new KConfig(QStringLiteral("kgammarc"));

        for (int i = 0; i < xv._ScreenCount(); i++) {
            xv.setScreen(i);
            KConfigGroup screenGroup = config->group(QStringLiteral("Screen %1").arg(i));

            if ((rgamma = screenGroup.readEntry("rgamma").toFloat())) {
                xv.setGamma(XVidExtWrap::Red, rgamma);
            }
            if ((ggamma = screenGroup.readEntry("ggamma").toFloat())) {
                xv.setGamma(XVidExtWrap::Green, ggamma);
            }
            if ((bgamma = screenGroup.readEntry("bgamma").toFloat())) {
                xv.setGamma(XVidExtWrap::Blue, bgamma);
            }
        }
        delete config;
    }
}
}

#include "kgamma.moc"

#include "moc_kgamma.cpp"
