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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef SEARCHDIR_H
#define SEARCHDIR_H
#include <QStringList>
class SearchDir
{
public:
    enum ErrorInfo{
        Successful = 0,
        Duplicated,
        UnderBlackList,
        RepeatMount1,
        RepeatMount2,
        NotExists
    };
    SearchDir() = default;
    SearchDir(const QString& path, bool generateBlackList = true);
    bool operator == (const SearchDir& rhs) const;
    ErrorInfo error();
    QString errorString();
    QString getPath() const;
    void setBlackList(const QStringList& blackList);
    QStringList getBlackList() const;
    /**
     * @brief 重新生成黑名单
     */
    void generateBlackList();
    static QStringList blackListOfDir(const QString &dirPath);
private:
    void handleBlackListGenerate();
    static QStringList handleSubVolumes4SingleDir(const QString& searchDir);
    QString m_path;
    QStringList m_blackList;
    ErrorInfo m_error = ErrorInfo::Successful;
};

#endif // SEARCHDIR_H
