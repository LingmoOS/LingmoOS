/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include <syslog.h>
#include <unistd.h>
#include "lingmo-search-service.h"
#include "log-utils.h"
#include "cgroup-helper.h"
using namespace LingmoUISearch;
int main(int argc, char *argv[])
{
    char *p_home = NULL;
    unsigned int i = 0;
    while(p_home == NULL) {
        ::sleep(1);
        ++i;
        p_home = getenv("HOME");
        if(i % 5 == 0) {
            qWarning() << "I can't find home! I'm done here!!";
            printf("I can't find home! I'm done here!!");
            syslog(LOG_ERR, "I can't find home! I'm done here!!\n");
        }
    }
    p_home = NULL;
    while(!QDir(QDir::homePath()).exists()) {
        qWarning() << "Home is not exits!!";
        printf("Home is not exits!!");
        syslog(LOG_ERR, "Home is not exits!!\n");
        ::sleep(1);
    }

    // Output log to file
    LogUtils::initLogFile("lingmo-search-service");
    qInstallMessageHandler(LogUtils::messageOutput);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    //为lingmo-search-service添加CPU限制，目前限制为单核100%
    if(!CgroupHelper::setServiceCpuLimit()) {
        qDebug()<<"CgroupHelper::setServiceCpuLimit() failed!";
    }
    LingmoUISearchService ukss(argc, argv, "lingmo-search-service");

    return LingmoUISearch::LingmoUISearchService::exec();
}
