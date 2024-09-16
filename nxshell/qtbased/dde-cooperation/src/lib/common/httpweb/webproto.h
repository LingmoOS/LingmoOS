// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WEBPROTO_H
#define WEBPROTO_H


#include <stdlib.h>
#include <string.h>

#ifndef PICOJSON_USE_INT64
#define PICOJSON_USE_INT64
#endif
#include "picojson/picojson.h"

struct InfoEntry {
    std::string name; // file or dir name
    int64_t size {0};  // file size, dir size set as 0
    std::vector<InfoEntry> datas; // the array for dir's Entries


    void from_json(const picojson::value &obj)
    {
        name = obj.get("name").to_str();
        size = obj.get("size").get<int64_t>();
        if (obj.get("datas").is<picojson::array>()) {
            const picojson::array &datasArray = obj.get("datas").get<picojson::array>();
            for (const auto &data : datasArray) {
                if (data.is<picojson::object>()) {
                    InfoEntry info;
                    info.from_json(data);
                    datas.push_back(info);
                }
            }
        }
    }

    picojson::value as_json() const
    {
        picojson::object obj;
        obj["name"] = picojson::value(name);
        obj["size"] = picojson::value(size);

        picojson::array datasArray;
        for (const auto &data : datas) {
            datasArray.push_back(picojson::value(data.as_json()));
        }
        obj["datas"] = picojson::value(datasArray);
        return picojson::value(obj);;
    }
};

#endif // WEBPROTO_H
