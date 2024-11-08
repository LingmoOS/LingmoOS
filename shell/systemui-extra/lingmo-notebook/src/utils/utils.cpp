/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#include "utils.h"

void Utils::setCLIName(QCommandLineParser &parser) {

    QCommandLineOption monitorRoleOption(QStringList() << "show", QObject::tr("show"));
    parser.addOption(monitorRoleOption);
    //添加帮助信息(-h,--help)，这个方法由QCommandLineParser自动处理
    parser.addHelpOption();
    //添加（-v,--version）方法，显示应用的版本，这个方法由QCommandLineParser自动处理
    parser.addVersionOption();
}
