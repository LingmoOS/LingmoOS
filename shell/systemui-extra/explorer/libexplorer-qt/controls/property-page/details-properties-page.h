/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#ifndef DETAILSPROPERTIESPAGE_H
#define DETAILSPROPERTIESPAGE_H

#include "properties-window-tab-iface.h"

#include "file-info-job.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>

namespace Peony {

    class FileWatcher;

class DetailsPropertiesPage : public PropertiesWindowTabIface
{
    Q_OBJECT
public:
    explicit DetailsPropertiesPage(const QString &uri, QWidget *parent = nullptr);
    ~DetailsPropertiesPage();
    /*!
     * \brief 统一创建新的label
     * \param minWidth
     * \param minHeight
     * \param text
     * \param parent
     * \return
     */
    QLabel *createFixedLabel(quint64 minWidth, quint64 minHeight, QString text, QWidget *parent);

    void initDetailsPropertiesPage();
    void saveAllChange() override;

public:
    //
    void getFIleInfo();

Q_SIGNALS:
    void fileInfoReady();

public Q_SLOTS:
    void updateDateFormat(QString dateFormat);

private:
    QString m_uri = nullptr;
    std::shared_ptr<FileInfo> m_fileInfo = nullptr;
    std::shared_ptr<FileWatcher> m_watcher;

    QString m_systemTimeFormat  = nullptr;
    QVBoxLayout *m_layout       = nullptr;
    QTableWidget *m_tableWidget = nullptr;

    QLabel *m_createDateLabel = nullptr;
    QLabel *m_modifyDateLabel = nullptr;
    //image file
    QLabel *m_imageWidthLabel  = nullptr;
    QLabel *m_imageHeightLabel = nullptr;
    QLabel *m_imageDepthLabel  = nullptr;

    QLabel *m_ownerLabel = nullptr;
    QLabel *m_computerLabel = nullptr;

    QLabel *m_localLabel = nullptr;
    QLabel *m_nameLabel = nullptr;

    QString m_date_format = "";

    QWidget *createTableRow(QString labelText, QString content);
    QWidget *createTableRow(QString labelText, QLabel *contentLabel);

    void addRow(QString labelText, QString content);
    void addRow(QString labelText, QLabel *contentLabel);

    void initTableWidget();

    void setSystemTimeFormat(QString format);

    void updateFileInfo(const QString &uri);
};

}

#endif //DETAILSPROPERTIESPAGE_H
