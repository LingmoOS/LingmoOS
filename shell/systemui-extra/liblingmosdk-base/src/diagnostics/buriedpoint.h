/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: jishengjie <jishengjie@kylinos.cn>
 *
 */

#ifndef BURIEDPOINT_H_
#define BURIEDPOINT_H_

#include <string>
#include <map>

namespace kdk
{

class BuriedPoint
{
public:
    BuriedPoint();
    ~BuriedPoint();

    /**
     * @brief 上传埋点数据
     *
     * @param packageName : 包名
     * @param messageType : 消息类型
     * @param data : 要上传的数据
     *
     * @retval true : 上传成功
     * @retval false : 上传失败
     */
    bool uploadMessage(std::string packageName , std::string messageType , std::map<std::string , std::string> data);

private:
    enum returnState {
        OK = 0,                                        /* 存储成功 */
        InvalidArgumentFormat = 1,                     /* 参数格式错误 */
        InvalidTid = 2,                                /* tid异常 , 但消息存储成功 */
        InvalidUploadedMessageSha256 = 3,              /* shan256异常 */
        InvalidUploadedMessageSha256Decryption = 4,    /* sha256解密异常 */
        InvalidCreateTimeStamp = 5                     /* 时间字段异常 */
    };

    bool checkDir(void);
    std::string getUploadData(std::map<std::string , std::string> &data);
    std::string getCurrentTime(void);
    bool callDbus(const std::string &pkgInfo , const std::string &uploadData , const std::string &uploadMessageSha256);
    std::string readTid(void);
    bool writeTid(std::string tid);
};

}

#endif
