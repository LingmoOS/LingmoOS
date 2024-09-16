// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontsmanager.h"
#include "../api/utils.h"
#include "../common/commondefine.h"


#include <openssl/evp.h>
#include <cstring>
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#include <QDebug>
#include <QDir>
#include <QtGlobal>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

FontsManager::FontsManager()
    :virtualFonts{"monospace", "mono", "sans-serif", "sans", "serif"}
    ,filePath(utils::GetUserConfigDir()+"/fontconfig"+"/conf.d"+"/99-deepin.conf")
    ,familyBlacklist{"Symbol","webdings","MT Extra",
                     "Wingdings","Wingdings 2","Wingdings 3"}
{
    if(QGSettings::isSchemaInstalled(XSETTINGSSCHEMA))
    {
        xSetting = QSharedPointer<QGSettings>(new QGSettings(XSETTINGSSCHEMA));
    }
    initFamily();
}

void FontsManager::initFamily()
{
    fileName = utils::GetUserHomeDir()+"/.cache/deepin/dde-daemon/fonts/family_table";
    if(!isFcCacheUpdate())
    { 
        bool bSuccess = loadCacheFromFile(fileName);
        if(bSuccess)
        {
            return;
        }
    }

    fcInfosToFamilyTable();

    saveToFile();
}

bool FontsManager::isFontFamily(QString value)
{
    if(virtualFonts.indexOf(value) != -1)
    {
        return true;
    }

    value = md5(value);

    if(familyMap.count(value)!=0){
        return true;
    }

    return false;
}

bool FontsManager::isFontSizeValid(double size)
{
    if(size >=7.0 && size <=22.0)
    {
        return true;
    }
    return false;
}

bool FontsManager::setFamily(QString standard, QString monospace, double size)
{
    if(virtualFonts.indexOf(standard) != -1)
    {
        standard = fcFontMatch(standard);
    }

    if(virtualFonts.indexOf(monospace) != -1)
    {
        monospace = fcFontMatch(monospace);
    }


    QString standardMd5 = md5(standard);
    QString monospaceMd5 = md5(monospace);
    qDebug() << "setFamily md5, standard=" << standardMd5 << ", monospace=" << monospaceMd5;

    if(familyMap.count(standardMd5) != 1)
    {
        qWarning() << QString("Invalid standard id %1").arg(standardMd5);
        return false;
    }
    QSharedPointer<Family> standInfo = familyMap[standardMd5];

    if(familyMap.count(monospaceMd5) != 1)
    {
        qWarning() << QString("Invalid monospace id %1").arg(monospaceMd5);
        return false;
    }
    QSharedPointer<Family> monoInfo = familyMap[monospaceMd5];

    QString content = configContent(standInfo->id, monoInfo->id);

    QDir dir(filePath.left(filePath.lastIndexOf("/")));
    if(!dir.exists())
    {
        qDebug() << "will create Folder" << dir.path();
        dir.mkpath(dir.path());
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "file:" << file.fileName() <<" open error";
        return false;
    }
    file.write(content.toLatin1(),content.length());
    file.close();

    if(qFuzzyCompare(size,-1)){
        size = getFontSize();
    }

    QString value = QString::asprintf("%s %.2lf", standard.toLatin1().data(), size);

    if(!xSetting)
    {
        qWarning() << "xSetting is not exist";
        return false;
    }
    if(xSetting->get(GSKEYFONTNAME).toString() != value)
    {
        xSetting->set(GSKEYFONTNAME,value);
    }

    return true;
}

bool FontsManager::reset()
{
    if (!QFile::remove(filePath)) {
        qWarning() << "failed to remove file " << filePath;
        return false;
    }

    xSetting->reset(GSKEYFONTNAME);
    return true;
}

QStringList FontsManager::listMonospace()
{
    QStringList retList;
    for(auto item :familyMap.toStdMap())
    {
        if(item.second->monospace)
        {
            retList.push_back(item.second->id);
        }
    }

    return retList;
}

QStringList FontsManager::listStandard()
{
    QStringList retList;
    for(auto item :familyMap.toStdMap())
    {
        if(item.second->monospace || !item.second->show)
        {
            continue;
        }
        retList.push_back(item.second->id);
    }
    retList.sort();

    return retList;
}

QSharedPointer<FontsManager::Family> FontsManager::getFamily(QString id)
{
    id = md5(id);

    if(familyMap.count(id)==1)
    {
        return familyMap[id];
    }

    return nullptr;
}

double FontsManager::getFontSize()
{
    if(!xSetting)
    {
        return 0;
    }
    QString value = xSetting->get(GSKEYFONTNAME).toString();

    QStringList tempList = value.split(" ");
    if(tempList.isEmpty())
    {
        return 0;
    }

    return  tempList.last().toDouble();
}

bool FontsManager::checkFontConfVersion()
{
    QString homeDir = utils::GetUserHomeDir();
    homeDir += homeDir+ "/.config/fontconfig/conf.d/deepin_conf.version";

    QFile file(homeDir);
    if(!file.open(QIODevice::ReadWrite))
    {
        return false;
    }

    if(file.readAll() == "1.4"){
        return true;
    }

    file.close();

    QFile writeFile(homeDir);
    if(!writeFile.open(QFile::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    writeFile.write("1.4");
    writeFile.close();
    return true;
}

QVector<QSharedPointer<FontsManager::Family>> FontsManager::getFamilies(QStringList ids)
{
    QVector<QSharedPointer<FontsManager::Family>> familyVec;
    for(auto id : ids)
    {
        id = md5(id);
        if(familyMap.count(id) == 1)
        {
            familyVec.push_back(familyMap[id]);
        }
    }

    return familyVec;
}

QString FontsManager::md5(QString src)
{
    EVP_MD_CTX *mdctx;
    unsigned char *md5_digest;
    unsigned int md5_digest_len = static_cast<unsigned int>(EVP_MD_size(EVP_md5()));

    // MD5_Init
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_md5(), nullptr);

    // MD5_Update
    EVP_DigestUpdate(mdctx, src.toLatin1().data(), static_cast<unsigned long>(src.size()));

    // MD5_Final
    md5_digest = static_cast<unsigned char*>(OPENSSL_malloc(md5_digest_len));
    EVP_DigestFinal_ex(mdctx, md5_digest, &md5_digest_len);
    EVP_MD_CTX_free(mdctx);

    return getStringFromUnsignedChar(md5_digest,md5_digest_len);

}

QString FontsManager::getStringFromUnsignedChar(unsigned char *str, unsigned int length)
{
    QString s;
    QString result = "";

    // Print String in Reverse order....
    for ( unsigned int i = 0; i<length; i++)
    {
        s = QString("%1").arg(str[i],0,16);

        if(s == "0"){
            s="00";
        }
        result.append(s);
     }
   return result;
}

QString FontsManager::fcFontMatch(QString family)
{
    family = fontMatch(family);
    if(family.isEmpty())
    {
        return family;
    }

    QStringList tmpList = family.split(":");
    if(tmpList.size() != 2)
    {
        return nullptr;
    }

    QStringList nameList = tmpList[1].split("\"");
    if(nameList.size()<2){
        return nullptr;
    }

    QString key = md5(nameList[1]);
    if(familyMap.count(key)==1)
    {
        return familyMap[key]->id;
    }

    for(auto item : familyMap.toStdMap())
    {
        if(item.second->name == nameList[1])
        {
            return item.second->id;
        }
    }

    return nameList[1];
}

QString FontsManager::fontMatch(QString family)
{
    FcPattern* pat =FcNameParse(reinterpret_cast<FcChar8*>(family.toLatin1().data()));
    if(pat == nullptr)
    {
        return nullptr;
    }

    FcConfigSubstitute(nullptr, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcResult result;
    FcPattern* match = FcFontMatch(nullptr, pat, &result);
    FcPatternDestroy(pat);
    if (!match) {
      return nullptr;
    }

    FcFontSet* fs = FcFontSetCreate();
    if (!fs) {
      FcPatternDestroy(match);
      return nullptr;
    }

    FcFontSetAdd(fs, match);
    FcPattern* font = FcPatternFilter(fs->fonts[0], nullptr);
    FcChar8* ret = FcPatternFormat(font, reinterpret_cast<const FcChar8*>("%{=fcmatch}\n"));


    FcPatternDestroy(font);
    FcFontSetDestroy(fs);
    FcPatternDestroy(match);
    //FcFini(); // SIGABRT: FcCacheFini 'assert fcCacheChains[i] == NULL failed'

    if (!ret) {
      return nullptr;
    }

    return getStringFromUnsignedChar(ret,static_cast<unsigned int>(strlen(reinterpret_cast<char*>(ret))));
}

QString FontsManager::configContent(QString standard, QString monospace)
{
    QString retString = QString::asprintf(
    R"(<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "fonts.dtd">
<fontconfig>
    <match target="pattern">
        <test qual="any" name="family">
            <string>sans-serif</string>
        </test>
        <edit name="family" mode="prepend" binding="strong">
            <string>%s</string>
        </edit>
    </match>

    <match target="pattern">
        <test qual="any" name="family">
            <string>monospace</string>
        </test>
        <edit name="family" mode="prepend" binding="strong">
            <string>%s</string>
        </edit>
    </match>

    <match target="font">
        <edit name="rgba"><const>rgb</const></edit>
    </match>
 </fontconfig>)",
    standard.toLatin1().data(),
    monospace.toLatin1().data());
    return retString;
}

bool FontsManager::isFcCacheUpdate()
{
    static bool first = false;
    if(!first)
    {
        FcInit();
        first = true;
        return true;
    }

    return !FcConfigUptoDate(nullptr) && FcInitReinitialize();
}

FontsManager::FcInfo* FontsManager::listFontInfo (int *num)
{
    *num = -1;
    FcPattern *pat = FcPatternCreate();
    if (!pat)
    {
         fprintf(stderr, "Create FcPattern Failed\n");
         return nullptr;
    }

    FcObjectSet *os = FcObjectSetBuild(
         FC_FAMILY,
         FC_FAMILYLANG,
         /* FC_FULLNAME, */
         /* FC_FULLNAMELANG, */
         FC_STYLE,
         /* FC_FILE, */
         FC_LANG,
         FC_SPACING,
         FC_CHARSET,
         NULL);
    if (!os) {
         fprintf(stderr, "Build FcObjectSet Failed\n");
         FcPatternDestroy(pat);
         return nullptr;
    }

    FcFontSet *fs = FcFontList(nullptr, pat, os);
    FcObjectSetDestroy(os);
    FcPatternDestroy(pat);
    if (!fs) {
         fprintf(stderr, "List Font Failed\n");
         return nullptr;
    }

    int i;
    int cnt = 0;
    FcInfo *list = nullptr;
    for (i = 0; i < fs->nfont; i++) {
         char *charset = reinterpret_cast<char*>(FcPatternFormat(fs->fonts[i], reinterpret_cast<FcChar8*>(const_cast<char*>("%{charset}"))));
         if (charset == nullptr || strlen(charset) == 0) {
             free(charset);
             continue;
         }
         free(charset);

         if (appendFontinfo(&list, fs->fonts[i], cnt) == -1) {
              continue;
         }
         cnt++;
    }
    FcFontSetDestroy(fs);

    *num = cnt;

    return list;
}

int FontsManager::appendFontinfo(FcInfo** list, FcPattern* pat, int idx)
{
    FcInfo* tmp = static_cast<FcInfo*>(realloc(*list, static_cast<unsigned long>((idx+1))*sizeof(FcInfo)));
    if (!tmp) {
      fprintf(stderr, "Alloc memory at append %d font info failed\n", idx+1);
      return -1;
    }

    *list = tmp;

    tmp[idx].family = reinterpret_cast<char*>(FcPatternFormat(pat, reinterpret_cast<FcChar8*>(const_cast<char*>("%{family}"))));
    tmp[idx].familylang = reinterpret_cast<char*>(FcPatternFormat(pat, reinterpret_cast<FcChar8*>(const_cast<char*>("%{familylang}"))));
    tmp[idx].style = reinterpret_cast<char*>(FcPatternFormat(pat, reinterpret_cast<FcChar8*>(const_cast<char*>("%{style}"))));
    tmp[idx].lang = reinterpret_cast<char*>(FcPatternFormat(pat, reinterpret_cast<FcChar8*>(const_cast<char*>("%{lang}"))));
    tmp[idx].spacing = reinterpret_cast<char*>(FcPatternFormat(pat, reinterpret_cast<FcChar8*>(const_cast<char*>("%{spacing}"))));

    return 0;
}

void FontsManager::freeFontInfoList(FcInfo *list, int num)
{
    if (!list) {
        return;
    }

    int i;
    for (i = 0; i < num; i++)
    {
        free(list[i].family);
        free(list[i].familylang);
        free(list[i].style);
        free(list[i].lang);
        free(list[i].spacing);
    }

    free(list);
}

QSharedPointer<FontsManager::Family> FontsManager::fcInfoToFamily(FcInfo* info)
{
    QString familyName(info->family);
    if(familyName.isEmpty())
    {
        return nullptr;
    }

    QStringList familyNames = familyName.split(DEFAULTNAMEDELIM);
    QStringList familyLangs = QString(info->familylang).split(DEFAULTNAMEDELIM);

    QString family = familyName;
    int index = familyLangs.indexOf(DEFAULTLANG);
    // only check blacklist for en, if the font only have other lang, then skip
    if(index != -1) {
        if(index < familyNames.size())
        {
            family = familyNames[index];
        }

        if(familyBlacklist.contains(family) || family.isEmpty())
        {
            return nullptr;
        }
    }

    QStringList langs = QString(info->lang).split(DEFAULTLANGDELIM);
    QString curLang = getCurLang();

    QSharedPointer<FontsManager::Family> familyObj(new FontsManager::Family);

    familyObj->id =family;
    index = familyLangs.indexOf(curLang);
    if(index != -1 && index<familyNames.size())
    {
        familyObj->name = familyNames[index];
    }
    familyObj->styles = QString(info->style).split(DEFAULTNAMEDELIM);
    familyObj->monospace = QString(info->spacing) == SPACETYPEMONO || familyName.toLower().contains("mono");
    familyObj->show = langs.contains(curLang);

    return familyObj;
}


QString FontsManager::getCurLang()
{
    char* locale = getenv("LANGUAGE");
    if(strlen(locale)==0)
    {
        locale = getenv("LANG");
    }

    QString lang = getLangFromLocale(locale);
    if(lang.isEmpty())
    {
        return DEFAULTLANG;
    }

    return lang;
}

QString FontsManager::getLangFromLocale(QString locale)
{
    if(locale.isEmpty())
    {
        return "";
    }

    locale = locale.toLower();
    if(locale.contains("."))
    {
        locale = locale.split(".")[0];
    }

    if(locale.contains(":"))
    {
        locale = locale.split(":")[0];
    }

    if(locale == "zh_hk")
    {
        locale = "zh-tw";
    }else if (locale == "zh_cn"||locale == "zh_tw"||locale == "zh_sg"||
              locale == "ku_tr"||locale == "ku_tr"||locale == "pap_an"||locale == "pap_aw") {
        locale = locale.replace("_","-");
    }else {
        locale = locale.split("_")[0];
    }

    return locale;
}

void FontsManager::fcInfosToFamilyTable()
{
    int num;
    FontsManager::FcInfo* infos = listFontInfo(&num);
    if(num<1)
    {
        return;
    }

    for(int i=0;i<num;i++)
    {
        QSharedPointer<FontsManager::Family> family =  fcInfoToFamily(&infos[i]);
        if(!family)
        {
            continue;
        }

        QString key = md5(family->id);
        if(familyMap.count(key) == 0)
        {
            familyMap[key] = family;
        }
    }
}

bool FontsManager::loadCacheFromFile(QString fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"open "<<fileName<<"fail";
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if(error.error != QJsonParseError::NoError)
    {
        return false;
    }

    QJsonObject obj = doc.object();
    for(auto key : obj.keys())
    {
        QSharedPointer<Family> family(new Family);

        QJsonObject familyObj = obj[key].toObject();
        for(auto familyKey : familyObj.keys())
        {
            if(familyKey == "id")
            {
                family->id = familyObj[familyKey].toString();

            }else if (familyKey == "name") {
                family->name = familyObj[familyKey].toString();

            }else if (familyKey == "styles") {
                for(auto iter:familyObj[familyKey].toArray())
                {
                    family->styles.push_back(iter.toString());
                }

            }else if (familyKey == "monospace") {
                family->monospace = familyObj[familyKey].toBool();

            }else if (familyKey == "show") {
                family->show = familyObj[familyKey].toBool();
            }
        }

        familyMap[key] = family;
    }
    return true;
}

bool FontsManager::saveToFile()
{
    QJsonDocument doc;
    QJsonObject obj;

    for(auto iter : familyMap.toStdMap())
    {
        QJsonObject familyObj;
        familyObj["id"] = iter.second->id;
        familyObj["name"] = iter.second->name;

        QJsonArray styleArr;
        for(auto style : iter.second->styles)
        {
            styleArr.push_back(style);
        }
        familyObj["styles"] = styleArr;

        familyObj["monospace"] = iter.second->monospace;
        familyObj["show"] = iter.second->show;

        obj[iter.first] = familyObj;
    }

    doc.setObject(obj);
    QByteArray text = doc.toJson();

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        return false;
    }

    file.write(text);
    return true;
}
