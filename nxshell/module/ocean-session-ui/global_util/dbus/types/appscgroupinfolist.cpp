#include "appscgroupinfolist.h"

void registerAppsCGroupInfoMetaType()
{
    qRegisterMetaType<AppsCGroupInfo>("AppsCGroupInfo");
    qDBusRegisterMetaType<AppsCGroupInfo>();
}

void registerAppsCGroupInfoListMetaType()
{
    registerAppsCGroupInfoMetaType();

    qRegisterMetaType<AppsCGroupInfoList>("AppsCGroupInfoList");
    qDBusRegisterMetaType<AppsCGroupInfoList>();
}
