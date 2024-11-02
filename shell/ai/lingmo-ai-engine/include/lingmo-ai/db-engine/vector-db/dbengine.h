/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef VD_DB_ENGINE_H
#define VD_DB_ENGINE_H

#include <string>
#include <vector>
#include <memory>

#include "vectordbengine.h"

namespace db_engine {
namespace vector_db {
class AbstractDatabaseEnginePluginFactory {
public:
    virtual ~AbstractDatabaseEnginePluginFactory() {}
    virtual bool isCloud() = 0;
    virtual std::string engineName() const = 0;

    virtual std::unique_ptr<AbstractDatabaseEngine> createAbstractDatabaseEngine() = 0;
};

}
}

#define VECTOR_DATABASE_ENGINE_FACTORY(ClassName)                                                           \
extern "C" {                                                                                                \
    ::db_engine::vector_db::AbstractDatabaseEnginePluginFactory *ai_vector_database_engine_factory_instance() {\
        static ClassName factory;                                                                           \
        return &factory;                                                                                    \
}                                                                                                           \
}

#endif