#ifndef VECTORDBENGINE_H
#define VECTORDBENGINE_H

#include "vectordberror.h"
#include <string>
#include <vector>
#include <map>
#include <variant>

namespace db_engine {
namespace vector_db {

struct VectorData{
    std::string id;
    std::vector<double> embedding = {};
    std::map<std::string, std::variant<std::string, int64_t>> metadata;
};

class AbstractDatabaseEngine
{
public:
    virtual ~AbstractDatabaseEngine() {}
    virtual bool isCloud() const {return false;}
    virtual std::string engineName() const = 0;

    virtual VectorDBErrorCode createClient(const std::string& databasePath) = 0;
    virtual VectorDBErrorCode destroyClient() = 0;

    virtual VectorDBErrorCode createCollection(const std::string& collectionName, const std::string& searchAlgorithm) = 0;
    virtual std::vector<std::string> collections() = 0;
    virtual VectorDBErrorCode addData(const std::string& collectionName, const std::vector<VectorData>& data) = 0;
    virtual VectorDBErrorCode updateData(const std::string& collectionName, const std::vector<VectorData>& data) = 0;

    virtual std::vector<VectorData> getData(const std::string& collectionName,
                                    const std::vector<std::string>& ids={}, const std::string& source="") = 0;

    virtual VectorDBErrorCode deleteData(const std::string& collectionName,
                    const std::vector<std::string>& sourceIds={}, const std::vector<std::string>& ids={}) = 0;

    virtual std::vector<std::pair<VectorData, float>> queryData(const std::string& collectionName,
                                                         std::vector<std::vector<double>> embeddings,
                                                         int64_t numResults = 10) = 0;
};

}
}

#endif