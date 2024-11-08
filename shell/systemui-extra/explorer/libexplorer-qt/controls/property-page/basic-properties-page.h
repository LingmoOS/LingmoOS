/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef BASICPROPERTIESPAGE_H
#define BASICPROPERTIESPAGE_H

#include <QWidget>
#include "explorer-core_global.h"

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QThread>
#include <memory>
#include <QProxyStyle>
#include <QComboBox>
#include <QDBusInterface>

#include "properties-window-tab-iface.h"
#include "open-with-properties-page.h"

#define CELL1K 1024
#define CELL4K 4096
#define CELL1M 1048576
#define CELL1G 1073741824

#define TEMP_PERMISSIONS    "temp-permissions"

class QVBoxLayout;
class QFormLayout;
class QPushButton;
class QLabel;
class QLineEdit;

namespace Peony {

class FileInfo;
class FileWatcher;
class FileCountOperation;

class FileNameThread : public QThread {
    Q_OBJECT
private:
    const QStringList &m_uris;
public:
    FileNameThread(const QStringList &uris) : m_uris(uris){}

Q_SIGNALS:
    void fileNameReady(QString fileName);

protected:
    void run();
};

/*!
 * \brief The BasicPropertiesPage class
 * \todo
 * handle special files, such as divice or remote server.
 */
class BasicPropertiesPage : public PropertiesWindowTabIface
{
    Q_OBJECT
public:

    enum FileType{
        BP_Folder = 1,
        BP_File,
        BP_Application,
        BP_MultipleFIle /*选中多个文件*/
    };

    explicit BasicPropertiesPage(const QStringList &uris, QWidget *parent = nullptr);
    ~BasicPropertiesPage();

    void saveAllChange();

protected:
    /**
     * @brief 初始化ui组件
     */
    void initUI();

    /*!
     *
     * \brief formLayout 左侧label栏
     * \param minWidth
     * \param minHeight
     * \param text
     * \param parent
     * \return
     */
    QLabel *createFixedLabel(quint64 minWidth, quint64 minHeight, QString text, QWidget *parent = nullptr);
    QLabel *createFixedLabel(quint64 minWidth, quint64 minHeight, QWidget *parent = nullptr);
    /**
     * @brief 顶部图标，名称和地址信息区域
     */
    void initFloorOne();

    /**
     * @brief 日期，大小信息区域
     */
    void initFloorTwo();

    void addSeparator();

    void loadData();
    void loadPartOne();
    void loadPartTwo();
    void loadOptionalData();

    void updateCountInfo(bool isDone = false);
    void addOpenWithLayout(QWidget *parent = nullptr);
    BasicPropertiesPage::FileType checkFileType(const QStringList &uris);
    void chooseFileIcon();
    void changeFileIcon();
    bool isNameChanged();
    void setSysTimeFormat();
    QString elideText(QFont font,int width,QString strInfo);
    QString elideTextAndToolTip(QFont font, int width, QString strInfo, Qt::TextElideMode mode, QWidget* widgt);

protected Q_SLOTS:
    void getFIleInfo(QString uri);
    void onSingleFileChanged(const QString &oldUri, const QString &newUri);
    void countFilesAsync(const QStringList &uris);
    void onFileCountOne(const QString &uri, quint64 size);
    void cancelCount();
    void updateInfo(const QString &uri);
    void updateDateFormat(QString dateFormat);

private:
    QVBoxLayout                 *m_layout = nullptr;
    std::shared_ptr<FileInfo>    m_info   = nullptr;
    QStringList                  m_uris;
    QFutureWatcher<void>        *m_futureWatcher = nullptr;
    std::shared_ptr<FileWatcher> m_watcher;
    std::shared_ptr<FileWatcher> m_thumbnail_watcher;

    FileType m_fileType        = BP_File;
    qint64   m_fileDoneCount   = 0;

    //floor1
    QPushButton *m_iconButton       = nullptr;    //文件图标
    QString     m_newFileIconPath;                //文件新图标
    //**new version
    QLineEdit   *m_displayNameEdit  = nullptr;    //文件名称
    QLineEdit   *m_locationEdit     = nullptr;    //文件路径

    //floor2  --  public
    QLabel *m_fileTypeLabel         = nullptr;    //文件类型
    QLabel *m_fileSizeLabel         = nullptr;    //文件大小
    QLabel *m_fileTotalSizeLabel    = nullptr;    //文件占用空间

    quint64 m_fileSizeCount        = 0;
    quint64 m_fileTotalSizeCount   = 0;

    //folder type
    QLabel *m_folderContainLabel = nullptr;       //文件夹下文件统计Label

    quint64 m_folderContainFiles   = 0;      //文件夹下文件数量
    quint64 m_folderContainFolders = 0;      //文件夹下文件夹数量

    //file , zip
    QHBoxLayout           *m_openWithLayout        = nullptr;    //文件打开方式
    DefaultOpenWithWidget *m_defaultOpenWithWidget = nullptr;
    QComboBox             *m_moreAppBox            = nullptr;

    //application
    QLabel *m_descrptionLabel = nullptr;       //应用程序描述

    //floor3
    QString m_systemTimeFormat  = "";
    QLabel *m_timeCreatedLabel  = nullptr;
    QLabel *m_timeModifiedLabel = nullptr;
    QLabel *m_timeAccessLabel   = nullptr;

    quint64 m_timeCreated  = 0;
    quint64 m_timeModified = 0;
    quint64 m_timeAccess   = 0;

    //floor4
    QCheckBox *m_readOnly = nullptr;
    QCheckBox *m_hidden   = nullptr;
    //
    FileCountOperation *m_countOp = nullptr;

    QTimer *m_timer = nullptr;
    bool m_isReadOnly = false;

    QString m_date_format = "";
    QDBusInterface  *mDbusDateServer;
};

class PushButtonStyle : public QProxyStyle
{

public:
    static PushButtonStyle *getStyle();

    PushButtonStyle() : QProxyStyle() {}

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override;

    QRect subElementRect(SubElement element,
                         const QStyleOption *option,
                         const QWidget *widget = nullptr) const;
};

}

#endif // BASICPROPERTIESPAGE_H
