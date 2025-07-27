/*
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGSETTINGS_ADDONS_H
#define KGPGSETTINGS_ADDONS_H

public:
   static
   QString defaultKey()
   {
     if (self()->mDefaultKey.isEmpty())
     {
        self()->mDefaultKey = KgpgInterface::getGpgSetting(QLatin1String( "default-key" ), gpgConfigPath());
     }
     return self()->mDefaultKey;
   }

   static
   void setDefaultKey(const QString &_defaultKey)
   {
     self()->mDefaultKey = _defaultKey;
     KgpgInterface::setGpgSetting(QLatin1String( "default-key" ), _defaultKey, gpgConfigPath());
   }

private:
   QString mDefaultKey;

#endif //KGPGSETTINGS_ADDONS_H
