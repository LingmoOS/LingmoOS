/***************************************************************************
 *   This file was modified by Uniontech Technology Co., Ltd.              *
 *   Copyright (C) 2011~2011 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <QDebug>
#include <fcitx-config/hotkey.h>
#include <assert.h>

#include "dummyconfig.h"

namespace Fcitx {

DummyConfig::DummyConfig()
{

}

DummyConfig::~DummyConfig()
{
    FcitxConfigFree(&m_config);
    foreach (void *value, m_dummyValue) {
        free(value);
    }
}

void DummyConfig::setConfigDesc(FcitxConfigFileDesc *cfdesc)
{
    m_cfdesc = cfdesc;
    m_config.configFile = nullptr;
    /* malloc necessary value */
    HASH_FOREACH(cgdesc, m_cfdesc->groupsDesc, FcitxConfigGroupDesc) {
        HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
            QString name("%1/%2");
            name = name.arg(cgdesc->groupName, codesc->optionName);
            if (m_dummyValue.contains(name))
                continue;
            void *value = nullptr;
            switch (codesc->type) {
#define OPTION_TYPE_CASE(NAME, TYPE) \
case T_##NAME: \
    value = fcitx_utils_new(TYPE); \
    break;
                OPTION_TYPE_CASE(Integer, int);
                OPTION_TYPE_CASE(Boolean, boolean);
                OPTION_TYPE_CASE(Char, char);
                OPTION_TYPE_CASE(Color, FcitxConfigColor);
                OPTION_TYPE_CASE(Enum, int);
                OPTION_TYPE_CASE(File, char *);
                OPTION_TYPE_CASE(Font, char *);
                OPTION_TYPE_CASE(Hotkey, FcitxHotkeys);
                OPTION_TYPE_CASE(String, char *);
                OPTION_TYPE_CASE(I18NString, char *);
            default:
                break;
            }
            if (value)
                m_dummyValue[name] = value;
        }
    }
}

FcitxGenericConfig *DummyConfig::genericConfig()
{
    return &m_config;
}

void DummyConfig::load(FILE *fp)
{
    if (!m_config.configFile) {
        m_config.configFile = FcitxConfigParseConfigFileFp(fp, m_cfdesc);

        HASH_FOREACH(cgdesc, m_cfdesc->groupsDesc, FcitxConfigGroupDesc) {
            HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                QString name("%1/%2");
                name = name.arg(cgdesc->groupName, codesc->optionName);
                if (!m_dummyValue.contains(name))
                    continue;
                // assert(m_dummyValue[name]);
                FcitxConfigBindValue(m_config.configFile, cgdesc->groupName, codesc->optionName, m_dummyValue[name], nullptr, nullptr);
            }
        }
    } else {
        m_config.configFile = FcitxConfigParseIniFp(fp, m_config.configFile);
    }
}

void DummyConfig::bind(char *group, char *option, FcitxSyncFilter filter, void *arg)
{
    if (!m_config.configFile)
        return;
    QString name("%1/%2");
    name = name.arg(group, option);

    if (!m_dummyValue.contains(name))
        return;

    // assert(m_dummyValue[name]);
    FcitxConfigBindValue(m_config.configFile, group, option, m_dummyValue[name], filter, arg);
}

bool DummyConfig::isValid()
{
    return (m_config.configFile != nullptr);
}

void DummyConfig::configSyncToUi()
{
    FcitxConfigBindSync(&m_config);
}




}
