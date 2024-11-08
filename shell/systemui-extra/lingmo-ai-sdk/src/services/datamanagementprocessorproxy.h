#ifndef DATAMANAGEMENTPROCESSORPROXY_H
#define DATAMANAGEMENTPROCESSORPROXY_H

#include <string>

#include "services/datamanagementprocessorglue.h"

class DataManagementProcessorProxy
{
public:
    DataManagementProcessorProxy(uid_t uid);
    ~DataManagementProcessorProxy();

    void setUserData(void *userData) { userData_ = userData; }

    int similaritySearch(const std::string& searchConditions, std::string& searchResult);
    int addTextFiles(const std::string& fileinfos);
    int addImageFiles(const std::string& fileinfos);
    int deleteFiles(const std::string& fileinfos);
    int updateFilesName(const std::string& fileinfos);
    int updateContentData(const std::string& fileinfos);
    int getAllFileinfos(std::string& result);
    int getFeatureStatus(int& status);

private:
    void init();
    void destroy();

private:
    const std::string objectPath_ = "/org/lingmo/aisdk/datamanagementprocessor";
    GDBusConnection *connection_ = nullptr;
    void *userData_ = nullptr;
    AisdkDataManagementProcessor *delegate_ = nullptr;
    uid_t uid_;
};

#endif // DATAMANAGEMENTPROCESSORPROXY_H
