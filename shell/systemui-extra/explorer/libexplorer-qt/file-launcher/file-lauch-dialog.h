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

#ifndef FILELAUCHDIALOG_H
#define FILELAUCHDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QHash>
#include <QFileDialog>
#include <QLineEdit>

#include "explorer-core_global.h"
#include "properties-window-tab-iface.h"
#include "file-label-model.h"
#include "file-info-job.h"
#include "file-launch-action.h"
//#include "open-with-properties-page.h"

class QVBoxLayout;
class QListWidget;
class QCheckBox;
class QDialogButtonBox;
class QListWidgetItem;

namespace Peony {

class FileLaunchAction;
class PEONYCORESHARED_EXPORT FileLauchDialog;

//默认打开方式组件
class DefaultAcitonWidget : public QWidget
{
Q_OBJECT

private:
    QLabel*           m_appNameLabel = nullptr;
    QLabel*           m_appIconLabel = nullptr;
    QHBoxLayout*      m_layout       = nullptr;

public:
    explicit DefaultAcitonWidget(QWidget *parent = nullptr);
    ~DefaultAcitonWidget() override;

    /**
     * \brief 设置应用图标
     * \param appIcon
     */
    void setAppIcon(QIcon appIcon);

    /**
     * \brief 设置应用名称
     * \param appName
     */
    void setAppName(QString appName);

    void setLaunchAction(FileLaunchAction* launchAction);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

class ActionGlobalData : public QObject
{
    Q_OBJECT
public:
    explicit ActionGlobalData(QObject *parent = nullptr);

    DefaultAcitonWidget *createWidgetForUri(const QString &uri, QWidget *parent = nullptr);
    FileLaunchAction *getActionByUri(const QString &uri);
    void setActionForUri(FileLaunchAction *newAction, bool needUpdate = true);
    void removeAction(const QString &uri);

private:
    QMap<QString, QList<DefaultAcitonWidget*>*> m_openWithWidgetMap;
    QMap<QString, FileLaunchAction*> m_newActionMap;

};


/*!
 * \brief The FileLauchDialog class
 * provides the dialog for choosing which application to open a file.
 */

class PEONYCORESHARED_EXPORT FileLauchDialog : public QDialog
{
    Q_OBJECT
public:
    static ActionGlobalData *actionGlobalData;
    static DefaultAcitonWidget* createDefaultAcitonWidget(const QString &uri, QWidget *parent);
    static void setNewLaunchAction(FileLaunchAction *newAction, bool needUpdate);

    explicit FileLauchDialog(const QString &uri, QWidget *parent = nullptr);
    QSize sizeHint() const override {
        return QSize(400, 600);
    }

    /*!
     * \brief setDoLaunch
     * set dialog action while dialog accepted.
     * default action will do launch and this can be changed by
     * set do launch argument with false.
     *
     * \param doLaunch
     * weither launch app after choice an action.
     */
    void setDoLaunch(bool doLaunch = true);

    /*!
     * \brief selectedAction
     * after dialog apply with do not launch accepted action,
     * we may need to get selected action for next custom steps
     * for launching.
     *
     * \return
     * an file launch action contains application info.
     */
    FileLaunchAction *selectedAction();
Q_SIGNALS:
    void open(FileLaunchAction *action);

public Q_SLOTS:
    void chooseOtherApp();
    void openAppCenter();

protected:
    void init(const QString &uri);
    void initFloorOne();

    void initFloorTwo(const QString &uri);

    void initFloorThree();

    void initFloorFour();

    void addSeparator(){
        QFrame *separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        m_layout->addWidget(separator);
    }
    void getFIleInfo(QString uri);
    void createDefaultOpenWithWidget();
    void saveChange();

    void moreAction();

    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

private:
    QVBoxLayout *m_layout;
    std::shared_ptr<FileInfo> m_info = nullptr;
    //默认打开方式
    DefaultAcitonWidget* m_defaultOpenWithWidget = nullptr;

    QListWidget *m_view;
    QHash<QListWidgetItem*, FileLaunchAction*> m_hash;
    QCheckBox *m_check_box;
    QDialogButtonBox *m_button_box;

    QFileDialog *fd;
};

}

#endif // FILELAUCHDIALOG_H
