/*
 *   SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef basictab_h
#define basictab_h

#include <KService>
#include <QKeySequence>
#include <QTabWidget>

class KKeySequenceWidget;
class QLineEdit;
class KIconButton;
class QCheckBox;
class QGroupBox;
class QLabel;
class KUrlRequester;
class KLineSpellChecking;

class MenuFolderInfo;
class MenuEntryInfo;

class BasicTab : public QTabWidget
{
    Q_OBJECT

public:
    explicit BasicTab(QWidget *parent = nullptr);

    void apply();

    void updateHiddenEntry(bool show);

Q_SIGNALS:
    void changed(MenuFolderInfo *);
    void changed(MenuEntryInfo *);
    void findServiceShortcut(const QKeySequence &, KService::Ptr &);

public Q_SLOTS:
    void setFolderInfo(MenuFolderInfo *folderInfo);
    void setEntryInfo(MenuEntryInfo *entryInfo);
    void slotDisableAction();
protected Q_SLOTS:
    void slotChanged();
    void launchcb_clicked();
    void termcb_clicked();
    void uidcb_clicked();
    void slotCapturedKeySequence(const QKeySequence &);
    void slotExecSelected();
    void onlyshowcb_clicked();
    void hiddenentrycb_clicked();

protected:
    /**
     * @brief Initializes the general tab.
     */
    void initGeneralTab();

    /**
     * @brief Initializes the advanced tab.
     */
    void initAdvancedTab();

    /**
     * @brief Initializes connections.
     */
    void initConnections();
    void enableWidgets(bool isDF, bool isDeleted);

protected:
    QLineEdit *_nameEdit = nullptr;
    KLineSpellChecking *_commentEdit = nullptr;
    KLineSpellChecking *_descriptionEdit = nullptr;
    KKeySequenceWidget *_keyBindingEdit = nullptr;
    KUrlRequester *_programEdit, *_pathEdit = nullptr;
    QLineEdit *_envarsEdit = nullptr;
    QLineEdit *_argsEdit = nullptr;
    QLineEdit *_terminalOptionsEdit = nullptr;
    QLineEdit *_userNameEdit = nullptr;
    QCheckBox *_terminalCB = nullptr;
    QCheckBox *_userCB = nullptr;
    QCheckBox *_launchCB = nullptr;
    QCheckBox *_onlyShowInKdeCB = nullptr;
    QCheckBox *_hiddenEntryCB = nullptr;
    KIconButton *_iconButton = nullptr;
    QGroupBox *_workPathGroup = nullptr;
    QGroupBox *_terminalGroup = nullptr;
    QGroupBox *_userGroup = nullptr;
    QGroupBox *_keyBindingGroup = nullptr;
    QLabel *_terminalOptionsLabel = nullptr;
    QLabel *_userNameLabel = nullptr;
    QLabel *_pathLabel = nullptr;
    QLabel *_nameLabel = nullptr;
    QLabel *_commentLabel = nullptr;
    QLabel *_programLabel = nullptr;
    QLabel *_envarsLabel = nullptr;
    QLabel *_argsLabel = nullptr;
    QLabel *_keyBindingLabel = nullptr;
    QLabel *_descriptionLabel = nullptr;

    MenuFolderInfo *_menuFolderInfo = nullptr;
    MenuEntryInfo *_menuEntryInfo = nullptr;
};

#endif
