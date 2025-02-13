/*
* Copyright (C) 2020 ~ 2022 Deepin Technology Co., Ltd.
*
* Author: WangZhengYang<wangzhengyang@uniontech.com>
*
* Maintainer: WangZhengYang<wangzhengyang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <atomic>
#include <QImage>
#include <QString>

class Details;

class PaddleOCRApp
{
public:
    enum Languages {
        UNKNOWN = -1, //未知语言
        CHI_SIM,      //简体中文
        CHI_TRA,      //繁体中文
        ENG           //英文
    };

    static PaddleOCRApp *instance();

    bool isRunning() const
    {
        return m_isRunning;
    }

    QString getRecogitionResult(const QImage &image);

    void setLanguages(Languages data);

    PaddleOCRApp::Languages getSystemLang();

private:
    PaddleOCRApp();
    ~PaddleOCRApp();


    std::vector<std::string> loadDict(const QString &dictPath);

    Details *ocrDetails;

    std::atomic_bool m_isRunning;
};
