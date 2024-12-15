// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTING_H
#define SETTING_H

#include "globaldefine.h"
#include<QObject>

class QSettings;
enum ESaveFormat {
    PNG = 0,
    JPG,
    BMP,
    TIF,
    PDF,
    PPM,
    XBM,
    XPM
};

enum ESavePath {
    Pictures = 0,
    Documents,
    Downloads,
    Desktop,
    Videos,
    Music,
    UsrSelect,
    Other
};
class DRAWLIB_EXPORT Setting : public QObject
{

public:
    ~Setting();
    static Setting *instance();
    QStringList readableFormats();
    QStringList writableFormats();
    QStringList readableFormatNameFilters();
    QStringList writableFormatNameFilters();
    /**
         * @brief defaultFileDialogPath 文件对话框的默认路径
         */
    QString defaultFileDialogPath() const;

    /**
     * @brief defaultFileDialogNameFilter 文件对话框的默认选中过滤信息
     */
    QString defaultFileDialogNameFilter() const;

    /**
     * @brief setDefaultFileDialogNameFilter 设置文件对话框的默认选中过滤信息
     */
    void setDefaultFileDialogNameFilter(const QString &nameFilter);

    /**
     * @brief setDefaultFileDialogPath 设置文件对话框的默认路径
     */
    void  setDefaultFileDialogPath(const QString &defaultPath);

    /**
     * @brief defaultFileDialogPath 导出对话框的默认路径
    */
    QPair<int,QString> defaultExportDialogPath() const;

    /**
     * @brief setDefaultFileDialogPath 设置导出对话框的默认路径
     */
    void  setDefaultExportDialogPath(const QPair<int,QString> &defaultPath);

    /**
     * @brief defaultExportDialogFilteFormat 导出对话框的默认选中格式信息
     */
    int defaultExportDialogFilteFormat() const;

    /**
     * @brief setDefaultFileDialogNameFilter 设置导出对话框的默认选中格式信息
     */
    void setDefaultExportDialogFilterFormat(const int &nameFilter);

    QString SavePathChange(int index);

    void readSettings();
    void saveSettings();

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key);
private:
    Setting();
private:

    QStringList supReadFormats;
    QStringList supReadFormatsFilter;
    QStringList supWriteFormatFilters;
    QStringList supWriteFormats;

    QString _defaultFileDialogPath;
    QString _defaultFileDialogNameFilter;

    //记录导出格式和路径
    QPair<int,QString> _defaultExportDialogPath = QPair<int,QString>(Pictures,"");
    int _defaultExportDialogFilterFormats = PNG;

    QSettings *_setting = nullptr;
};

#endif // SETTING_H
