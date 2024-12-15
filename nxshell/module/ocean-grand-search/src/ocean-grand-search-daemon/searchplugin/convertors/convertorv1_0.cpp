// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "convertorv1_0.h"
#include "global/matcheditem.h"
#include "utils/specialtools.h"
#include "utils/searchpluginprotocol.h"

#include <QStringList>

#ifdef CURRENT_CONVERTOR_VERSION
# error "Redeclaration of CURRENT_CONVERTOR_VERSION."
#else
# define CURRENT_CONVERTOR_VERSION "1.0"
#endif

using namespace GrandSearch;

ConvertorV1_0::ConvertorV1_0()
{

}

ConvertorV1_0::~ConvertorV1_0()
{

}

AbstractConvertor *ConvertorV1_0::create()
{
    return new ConvertorV1_0();
}

QString ConvertorV1_0::version() const
{
    return CURRENT_CONVERTOR_VERSION;
}

QHash<QString, ConvertInterface> ConvertorV1_0::interfaces()
{
    QHash<QString, ConvertInterface> funcs;
    funcs.insert(PLUGININTERFACE_TYPE_SEARCH, &search);
    funcs.insert(PLUGININTERFACE_TYPE_RESULT, &result);
    funcs.insert(PLUGININTERFACE_TYPE_STOP, &stop);
    funcs.insert(PLUGININTERFACE_TYPE_ACTION, &action);

    return funcs;
}

int ConvertorV1_0::search(void *in, void *out)
{
    Q_ASSERT(in);
    Q_ASSERT(out);

    QJsonObject *json = static_cast<QJsonObject *>(out);
    QStringList *args = static_cast<QStringList *>(in);

    //只包含三个参数
    if (Q_UNLIKELY(args->size() != 3))
        return -1;

    QString ver = args->at(0);
    QString id = args->at(1);
    QString content = args->at(2);

    //版本不匹配
    if (Q_UNLIKELY(CURRENT_CONVERTOR_VERSION != ver))
        return -1;

    //输入值为空
    if (Q_UNLIKELY(id.isEmpty() || content.isEmpty()))
        return 1;

    QVariantMap values;
    values.insert(PLUGININTERFACE_PROTOCOL_VERSION, ver);
    values.insert(PLUGININTERFACE_PROTOCOL_MISSIONID, id);
    values.insert(PLUGININTERFACE_PROTOCOL_CONTENT, content);

    *json = QJsonObject::fromVariantMap(values);
    return 0;
}

int ConvertorV1_0::result(void *in, void *out)
{
    Q_ASSERT(in);
    Q_ASSERT(out);
    QVariantList *varList = static_cast<QVariantList *>(in);
    if (varList->size() != 2)
        return -1;
    QString plugin = varList->first().toString();

    if (plugin.isEmpty())
        return 1;

    QJsonObject *json = static_cast<QJsonObject *>(varList->at(1).value<void *>());

    auto ver = SpecialTools::getJsonString(json, PLUGININTERFACE_PROTOCOL_VERSION);
    if (CURRENT_CONVERTOR_VERSION != ver)
        return -1;

    auto missionID = SpecialTools::getJsonString(json, PLUGININTERFACE_PROTOCOL_MISSIONID);
    if (missionID.isEmpty())
        return 1;

    static int MaxItemCount = 100;
    int count = 0;
    QJsonArray contents = SpecialTools::getJsonArray(json, PLUGININTERFACE_PROTOCOL_CONTENT);
    MatchedItemMap mContents;
    for (const QJsonValue &groupValue : contents) {

        //!前面组已经有100个项目了，不再继续解析
        if (count >= MaxItemCount)
            break;

        if (groupValue.isObject()) {
            auto group = groupValue.toObject();

            QString groupName = SpecialTools::getJsonString(&group, PLUGININTERFACE_PROTOCOL_GROUP);
            if (groupName.isEmpty())
                continue;

            QJsonArray itemsValue = SpecialTools::getJsonArray(&group, PLUGININTERFACE_PROTOCOL_ITEMS);
            MatchedItems aGroup;

            for (const QJsonValue &itemValue : itemsValue) {
                //!当前组最大支持100个，超过不再解析
                if (count >= MaxItemCount)
                    break;

                if (itemValue.isObject()) {
                    auto item = itemValue.toObject();

                    MatchedItem mItem;
                    mItem.item = SpecialTools::getJsonString(&item, PLUGININTERFACE_PROTOCOL_ITEM);
                    if (mItem.item.isEmpty())
                        continue;

                    mItem.name = SpecialTools::getJsonString(&item, PLUGININTERFACE_PROTOCOL_NAME);
                    if (mItem.name.isEmpty())
                        continue;

                    mItem.type = SpecialTools::getJsonString(&item, PLUGININTERFACE_PROTOCOL_TYPE);
                    if (mItem.type.isEmpty())
                        continue;

                    mItem.icon = SpecialTools::getJsonString(&item, PLUGININTERFACE_PROTOCOL_ICON);

                    mItem.searcher = plugin;

                    aGroup.append(mItem);

                    count++;
                }
            }

            if (aGroup.isEmpty())
                continue;

            auto org = mContents.value(groupName);
            org.append(aGroup);
            mContents.insert(groupName, org);
        }
    }

    QVariantList *results = static_cast<QVariantList *>(out);
    results->append(QVariant::fromValue(missionID));
    results->append(QVariant::fromValue(mContents));

    return 0;
}

int ConvertorV1_0::stop(void *in, void *out)
{
    Q_ASSERT(in);
    Q_ASSERT(out);

    QJsonObject *json = static_cast<QJsonObject *>(out);
    QStringList *args = static_cast<QStringList *>(in);

    //只包含两个参数
    if (Q_UNLIKELY(args->size() != 2))
        return -1;

    QString ver = args->at(0);
    QString id = args->at(1);

    //版本不匹配
    if (Q_UNLIKELY(CURRENT_CONVERTOR_VERSION != ver))
        return -1;

    //输入值为空
    if (Q_UNLIKELY(id.isEmpty()))
        return 1;

    QVariantMap values;
    values.insert(PLUGININTERFACE_PROTOCOL_VERSION, ver);
    values.insert(PLUGININTERFACE_PROTOCOL_MISSIONID, id);

    *json = QJsonObject::fromVariantMap(values);
    return 0;
}

int ConvertorV1_0::action(void *in, void *out)
{
    Q_ASSERT(in);
    Q_ASSERT(out);

    QJsonObject *json = static_cast<QJsonObject *>(out);
    QStringList *args = static_cast<QStringList *>(in);

    //只包含三个参数
    if (Q_UNLIKELY(args->size() != 3))
        return -1;

    QString ver = args->at(0);
    QString action = args->at(1);
    QString item = args->at(2);

    //版本不匹配
    if (Q_UNLIKELY(CURRENT_CONVERTOR_VERSION != ver))
        return -1;

    //输入值为空
    if (Q_UNLIKELY(action.isEmpty() || item.isEmpty()))
        return 1;

    QVariantMap values;
    values.insert(PLUGININTERFACE_PROTOCOL_VERSION, ver);
    values.insert(PLUGININTERFACE_PROTOCOL_ACTION, action);
    values.insert(PLUGININTERFACE_PROTOCOL_ITEM, item);

    *json = QJsonObject::fromVariantMap(values);
    return 0;
}
