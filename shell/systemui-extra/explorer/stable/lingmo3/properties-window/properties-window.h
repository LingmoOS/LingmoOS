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

#ifndef STABLEPROPERTIESWINDOW_H
#define STABLEPROPERTIESWINDOW_H

#include <QTabWidget>
#include <QMainWindow>

#include "properties-window_global.h"
#include "properties-window-tab-iface.h"

#include <QMap>
#include <QMutex>
#include <QSize>
#include <QProxyStyle>
#include <QtConcurrent>
#include <file-info.h>

namespace Peony {

class PropertiesWindowTabPagePluginIface;

class PropertiesWindowPluginManager : public QObject
{
    friend class PropertiesWindow;

Q_OBJECT

public:
    static PropertiesWindowPluginManager* getInstance();

    void release();
    void setOpenFromDesktop();

    void setOpenFromPeony();

    bool registerFactory(PropertiesWindowTabPagePluginIface *factory);
    bool unregisterFactory(PropertiesWindowTabPagePluginIface *factory);

    const QStringList getFactoryNames();

    PropertiesWindowTabPagePluginIface *getFactory(const QString &id);
    void initFactory();

private:
    explicit PropertiesWindowPluginManager(QObject *parent = nullptr);

    ~PropertiesWindowPluginManager() override;

    QHash<QString, PropertiesWindowTabPagePluginIface *> m_factory_hash;
    QMap<int, QString> m_sorted_factory_map;
    QMutex m_mutex;
};

class STABLEPROPERTIESWINDOW_EXPORT StablePropertiesWindow : public QMainWindow
{
Q_OBJECT
public:
    explicit StablePropertiesWindow(const QStringList &uris, QWidget *parent = nullptr);

    void setWindowTitleTextAndIcon();

    /*!
     * 判断当前的uris中是否'不存在'目录
     * \brief notDir
     * \return
     */
    void notDir();

    void gotoAboutComputer();

    void show();

    void init();

    /*!
     * 初始化底部按钮栏
     * \brief initStatusBar
     */
    void initStatusBar();

    /*!
     * 设置tab栏的样式。
     * FIX:目前tab样式不能跟随主题
     * \brief initTabPage
     * \param uris
     */
    void initTabPage(const QStringList &uris);


    /*!
     * \brief 获取一些桌面文件的图标 - Get the icons of some desktop files
     * \param fileInfo
     * \return
     */
    QString getIconName();

    QStringList &getUris()
    {
        return m_uris;
    }

    /*!
     * 检查当前的uris是否已经在窗口中打开
     * 如果已经打开，那么返回true,
     * 如果没有打开，那么返回false
     * \brief checkUriIsOpen
     * \param uris
     * \return
     */
    static bool checkUriIsOpen(QStringList &uris, StablePropertiesWindow *newWindow);

    /*!
     * 返回指定uris在 openPropertiesWindows 中的索引，如果不存在，返回 -1
     * Returns the index of the specified uris in openPropertiesWindows, if it does not exist, returns -1
     * \brief getOpenUriIndex
     * \return
     */
    static qint64 getOpenUriIndex(QStringList &uris);

    /*!
     * 从已打开窗口列表中删除索引的窗口
     * \brief removeThisWindow
     * \param index
     */
    static void removeThisWindow(qint64 index);

    /*!
     * 存放当前窗口的所有tab页面
     * \brief addTabPage
     * \param tabPage
     */
    void addTabPage(PropertiesWindowTabIface *tabPage)
    {

        if (tabPage) {
            m_openTabPage.append(tabPage);
            connect(tabPage, &PropertiesWindowTabIface::requestCloseMainWindow, this, [=]() {
                Q_ASSERT(tabPage);
                this->close();
            });
        }
    }

    /*!
     * 响应确认按钮
     * \brief saveAllChanged
     */
    void saveAllChanged();

    /**
     * @brief 收藏夹中的文件夹是以favorite:// 开头的，但是该uri的schema无法正常确定文件的位置，
     * 所以在该uri最后附带了query列表 schema=xxx以说明该文件的实际schema。
     * 该函数通过解析query列表获取正确的uri。
     * 修改直接在引用的参数 uri上进行。
     * @param uri
     * @return
     */
    QString rebuildUriBySchema(QString &uri);

    /**
     * @brief 对kmre-vfs中的文件进行特殊处理，以获取真实路径
     * @param uri
     * @return
     */
    bool handleKMREUri(QString &uri);

protected:
    /**
     * 在窗口关闭时，将存储的窗口指针从openPropertiesWindows中删除
     * @brief closeEvent
     * @param event
     */
    void closeEvent(QCloseEvent *event);

private:
    bool m_notDir = true;
    std::shared_ptr<FileInfo> m_fileInfo = nullptr;

    bool m_destroyThis = false;
    QStringList m_uris;
    QList<PropertiesWindowTabIface *> m_openTabPage;

public:
    //init in properties-window.cpp
    static const qint32 s_windowWidth;
    static const qint32 s_windowHeightFolder;
    static const qint32 s_windowHeightOther;
    static const QSize s_bottomButtonSize;

    // QWidget interface

};

class tabStyle : public QProxyStyle
{
    // QStyle interface
public:
    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter,
                     const QWidget *widget) const;

    QSize sizeFromContents(QStyle::ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
                           const QWidget *w) const;
};

class PropertiesWindowPrivate : public QTabWidget
{
    friend class StablePropertiesWindow;

Q_OBJECT
private:
    explicit PropertiesWindowPrivate(const QStringList &uris, QWidget *parent = nullptr);

};

}

#endif // DEVELPROPERTIESWINDOW_H
