/*
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef LOGUTILS_H
#define LOGUTILS_H
#include <QtMessageHandler>

class LogUtils
{
public:
    static void initLogFile(const QString &fileName);
    static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static void checkLogFile();
    static bool checkFileSize(const QString &fileName);
    static void clearFile(const QString &fileName);
    static quint64 m_startUpTime;
    static int m_logFileId;
    static QString m_logFileName;
    static QString m_currentLogFile;
};

#endif // LOGUTILS_H
