/*
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
 *
 */
#ifndef SEARCH_H
#define SEARCH_H

#include <QObject>
#include <QGSettings>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QSettings>
#include <QFileDialog>
#include <QTextCodec>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>

#include <kswitchbutton.h>
#include <kborderlessbutton.h>

#include <ukcc/interface/interface.h>
#include <ukcc/widgets/comboboxitem.h>
#include <ukcc/widgets/switchbutton.h>
#include <ukcc/widgets/hoverwidget.h>
#include <ukcc/widgets/switchbutton.h>
#include <ukcc/widgets/comboxframe.h>
#include <ukcc/widgets/titlelabel.h>
#include <ukcc/widgets/addbtn.h>

#include <ai-base/datamanagement.h>

//TODO
#define CONFIG_FILE "/.config/org.lingmo/lingmo-search/lingmo-search-block-dirs.conf"

enum ReturnCode {
    DirWatcherError = -1,
    Successful,
    Duplicated,
    UnderBlackList,
    RepeatMount1,
    RepeatMount2,
    NotExists,
    HasBeenBlocked,
    Hidden,
    PermissionDenied
};

class Search : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    explicit Search();
    virtual ~Search() = default;

    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget * pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    QString translationPath() const;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

    static DataManagementSession s_session;
    static bool s_failed;

public Q_SLOTS:
    void refreshSearchDirsUi();

private:
    QWidget * m_pluginWidget = nullptr;
    QString m_pluginName = "";
    int m_pluginType = 0;
    bool isModelReady = false;

    QGSettings * m_gsettings = nullptr;

    void initUi();
    QVBoxLayout * m_mainLyt = nullptr;
    TitleLabel * m_titleLabel = nullptr;

    //设置搜索引擎
//    TitleLabel * m_webEngineLabel = nullptr;
    ComboxFrame * m_webEngineFrame = nullptr;
    QVBoxLayout * m_webEngineLyt = nullptr;

    //索引开关设置
    TitleLabel *m_indexTitleLabel = nullptr;
    QFrame *m_fileIndexFrame = nullptr;
    QHBoxLayout *m_fileIndexLyt = nullptr;
    QLabel *m_fileIndexLabel = nullptr;
    kdk::KSwitchButton *m_fileIndexBtn = nullptr;

    //内容索引开关设置
    QFrame *m_setFrame = nullptr;
    QVBoxLayout *m_setFrameLyt = nullptr;
    QLabel *m_contentIndexLabel = nullptr;
    QFrame *m_contentIndexFrame = nullptr;
    QHBoxLayout *m_contentIndexLyt = nullptr;
//    QLabel *m_searchMethodLabel = nullptr;
    kdk::KSwitchButton *m_contentIndexBtn = nullptr;
    QMessageBox *m_contentIndexMsgBox = nullptr;
    //设置索引搜索模式
    QFrame *m_indexSetFrame = nullptr;
    QVBoxLayout *m_indexSetLyt = nullptr;
    //模糊搜索开关
    QFrame *m_indexMethodFrame = nullptr;
    QVBoxLayout *m_indexMethodLyt = nullptr;
    QLabel *m_indexMethodDescLabel = nullptr;
    //模糊搜索按钮
    QButtonGroup *m_indexMethodBtnGroup = nullptr;
    QFrame *m_fuzzyBtnFrame = nullptr;
    QHBoxLayout *m_fuzzyBtnLyt = nullptr;
    QRadioButton *m_fuzzyBtn = nullptr;
    QLabel *m_fuzzyDescLabel = nullptr;
    //精确搜索按钮
    QFrame *m_preciseBtnFrame = nullptr;
    QHBoxLayout *m_preciseBtnLyt = nullptr;
    QRadioButton *m_preciseBtn = nullptr;
    QLabel *m_preciseDescLabel = nullptr;
    //OCR开关
    QLabel *m_ocrSwitchLabel = nullptr;
    QFrame *m_ocrSwitchFrame = nullptr;
    QHBoxLayout *m_ocrSwitchLyt = nullptr;
    kdk::KSwitchButton *m_ocrSwitchBtn = nullptr;

    //AI索引开关设置
    QFrame *m_aiIndexFrame = nullptr;
    QHBoxLayout *m_aiIndexLyt = nullptr;
    QLabel *m_aiIndexLabel = nullptr;
    kdk::KBorderlessButton *m_getModelBtn = nullptr;
    kdk::KSwitchButton *m_aiIndexBtn = nullptr;

    //设置当前搜索目录
    TitleLabel *m_searchDirTitleLabel = nullptr;
    QLabel *m_searchDirDescLabel = nullptr;
    QFrame *m_searchDirsFrame = nullptr;
    QVBoxLayout *m_searchDirLyt = nullptr;
    QLabel *m_searchDirLabel = nullptr;
    AddBtn *m_addSearchDirBtn = nullptr;
    QFrame *m_addSearchDirFrame = nullptr;

    //设置黑名单
    TitleLabel * m_blockDirTitleLabel = nullptr;
    QLabel * m_blockDirDescLabel = nullptr;
    QFrame * m_blockDirsFrame = nullptr;
    QVBoxLayout * m_blockDirsLyt = nullptr;
//    QPushButton * m_addBlockDirWidget = nullptr;
    AddBtn * m_addBlockDirWidget = nullptr;
    QLabel * m_addBlockDirIcon = nullptr;
    QLabel * m_addBlockDirLabel = nullptr;
    QHBoxLayout * m_addBlockDirLyt = nullptr;

    QString m_lastBlockDialogDir;
    QString m_lastSearchDialogDir;

    QStringList m_blockDirs;
    void getBlockDirs();
    int setBlockDir(const QString &dirPath, const bool &is_add = true);
    void appendBlockDirToList(const QString &path);
    void removeBlockDirFromList(const QString &path);
    void initBlockDirsList();
//    void refreshBlockDirsList();

    QDBusInterface *m_interface = nullptr;
    QDBusInterface *m_setSearchDirInterface = nullptr;
    void initSearchDirs();
    int setSearchDir(const QString &dirPath, const bool isAdd);
    void appendSearchDirToList(const QString &path);
    void removeSearchDirFromList(const QString &path);

    void setupConnection();
    void initAiSettings();

private slots:
    void onBtnAddBlockFolderClicked();
    void onAddSearchDirBtnClicked();
    void onGetAiModelBtnClicked();
};

#endif // SEARCH_H
