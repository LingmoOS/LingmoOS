/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  SPDX-FileCopyrightText: 2001 Michael v.Ostheim <MvOstheim@web.de>
 */

#ifndef KGAMMA_H_
#define KGAMMA_H_

#include <KCModule>
// Added by qt3to4:
#include <QList>

class GammaCtrl;
class QCheckBox;
class QComboBox;
class QProcess;
class XVidExtWrap;

class KGamma : public KCModule
{
    Q_OBJECT
public:
    KGamma(QObject *parent, const KPluginMetaData &data);
    ~KGamma() override;

    void load() override;
    void save() override;
    void defaults() override;
    int buttons();

protected: // Protected methods
    /** The user interface */
    void setupUI();
    /** Decides if to load settings from user or system config */
    bool loadSettings();
    /** Load settings from kgammarc */
    bool loadUserSettings();
    /** Load settings from XF86Config */
    bool loadSystemSettings();
    /** Validate the loaded gamma values */
    bool validateGammaValues();

private Q_SLOTS:
    /** Called if the user marked or unmarked the XF86Config checkbox */
    void changeConfig();
    /** Called if the user marked or unmarked the sync screen checkbox */
    void SyncScreens();
    /** Called if the user chooses a new screen */
    void changeScreen(int sn);

private:
    bool saved, GammaCorrection;
    int ScreenCount, currentScreen;
    QStringList rgamma, ggamma, bgamma;
    QList<int> assign;
    QList<float> rbak, gbak, bbak;
    GammaCtrl *gctrl, *rgctrl, *ggctrl, *bgctrl;
    QCheckBox *xf86cfgbox, *syncbox;
    QComboBox *screenselect;
    QProcess *rootProcess;
    XVidExtWrap *xv;
};

#endif
