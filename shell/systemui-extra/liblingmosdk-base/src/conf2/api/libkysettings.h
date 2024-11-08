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
 * Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef KYSETTINGS_H
#define KYSETTINGS_H

#if __cplusplus
extern "C"
{
#endif

    typedef struct  _KSettings KSettings;

    typedef void    (*KCallBack)                    (void);

    /**
     * @brief 创建指定id，指定version的配置句柄
     * 
     * @param id 配置id，从应用名开始到目标组的路径，每个组名以"."分割
     * @param version 配置版本号
     * @return KSettings* 配置组的句柄，每个句柄存储一个组的所有配置数据
     */
    KSettings *     kdk_conf2_new                   (const char         *id,
                                                     const char         *version);

    /**
     * @brief 销毁配置句柄
     * 
     * @param ksettings 配置句柄
     */
    void            kdk_conf2_ksettings_destroy     (KSettings          *ksettings);

    /**
     * @brief 获取句柄的配置id
     * 
     * @param ksettings 配置句柄
     * @return char* 配置id，失败返回NULL。返回值需要释放
     */
    char *          kdk_conf2_get_id                (KSettings          *ksettings);

    /**
     * @brief 获取句柄的配置版本
     * 
     * @param ksettings 配置句柄
     * @return char* 配置版本，失败返回NULL。返回值需要释放
     */
    char *          kdk_conf2_get_version           (KSettings          *ksettings);
    
    /**
     * @brief 获取配置句柄的子组配置句柄
     * 
     * @param ksettings 配置句柄
     * @param name 子组名
     * @return KSettings* 子组句柄 
     */
    KSettings *     kdk_conf2_get_child             (KSettings          *ksettings,
                                                     const char         *name);

    /**
     * @brief 列举配置组的所有子组
     * 
     * @param ksettings 配置句柄
     * @return char** 子组列表，失败返回NULL。返回值需要释放
     */
    char **         kdk_conf2_list_children         (KSettings          *ksettings);

    /**
     * @brief 列举配置组的所有键
     * 
     * @param ksettings 配置句柄
     * @return char** 键名列表，失败返回NULL。返回值需要释放
     */
    char **         kdk_conf2_list_keys             (KSettings          *ksettings);

    /**
     * @brief 获取配置项的取值范围
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char* 取值范围，如果是数值类型返回字符串"min,max",如果是枚举返回字符串"{nick : value}"
     */
    char *          kdk_conf2_get_range             (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 检查要设置的值是否在键的取值范围内
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 要设置的值
     * @return int 可视作bool类型，0表示否，1表示是
     */
    int             kdk_conf2_range_check           (KSettings          *ksettings,
                                                     const char         *key,
                                                     const char         *value);

    /**
     * @brief 设置键的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 要设置的值
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_value             (KSettings          *ksettings,
                                                     const char         *key,
                                                     const char         *value);

    /**
     * @brief 获取键的值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char* 符合glib类型标签的数据字符串，失败返回NULL。返回值需要释放
     */
    char *          kdk_conf2_get_value             (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取键的默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char* 符合glib类型标签的数据字符串，失败返回NULL。返回值需要释放
     */
    char *          kdk_conf2_get_default_value     (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 设置bool类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 键值，bool类型，即0或1
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_boolean           (KSettings          *ksettings,
                                                     const char         *key,
                                                     int                 value);

    /**
     * @brief 获取bool类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return int 0表示FALSE，1表示TRUE
     */
    int             kdk_conf2_get_boolean           (KSettings          *ksettings,
                                                     const char         *key);
    /**
     * @brief 获取bool类型的键默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return int 0表示FALSE，1表示TRUE
     */
    int             kdk_conf2_get_default_boolean   (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 设置double类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名 
     * @param value 键值
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_double            (KSettings          *ksettings,
                                                     const char         *key,
                                                     double              value);

    /**
     * @brief 获取double类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return double 配置值
     */
    double          kdk_conf2_get_double            (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取double类型的键默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return double 配置值
     */
    double          kdk_conf2_get_default_double    (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 设置枚举类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 要设置的值
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_enum              (KSettings          *ksettings,
                                                     const char         *key,
                                                     int                 value);

    /**
     * @brief 获取枚举类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return int 枚举值
     */
    int             kdk_conf2_get_enum              (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取枚举类型的键默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return int 枚举值
     */
    int             kdk_conf2_get_default_enum      (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 设置int类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 要设置的键值
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_int               (KSettings          *ksettings,
                                                     const char         *key,
                                                     int                 value);

    /**
     * @brief 获取int类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return int 键值
     */
    int             kdk_conf2_get_int               (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取int类型的键默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return int 键值
     */
    int             kdk_conf2_get_default_int       (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 设置int64类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 要设置的键值
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_int64             (KSettings          *ksettings,
                                                     const char         *key,
                                                     long               value);

    /**
     * @brief 获取int64类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return long 键值
     */
    long            kdk_conf2_get_int64             (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取int64类型的键默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return long 键值
     */
    long            kdk_conf2_get_default_int64     (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 设置uint类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 要设置的键值
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_uint              (KSettings          *ksettings,
                                                     const char         *key,
                                                     unsigned int        value);

    /**
     * @brief 获取uint类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return unsigned int 键值
     */
    unsigned int    kdk_conf2_get_uint              (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取uint类型的键默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return unsigned int 键值
     */
    unsigned int    kdk_conf2_get_default_uint      (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 设置uint64类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 要设置的键值
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_uint64            (KSettings          *ksettings,
                                                     const char         *key,
                                                     unsigned long       value);

    /**
     * @brief 获取uint64类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return unsigned long 键值
     */
    unsigned long   kdk_conf2_get_uint64            (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取uint64类型的键默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return unsigned long 键值
     */
    unsigned long   kdk_conf2_get_default_uint64    (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 设置string类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 要设置的键值，符合类型标签的字符串
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_string            (KSettings          *ksettings,
                                                     const char         *key,
                                                     const char         *value);

    /**
     * @brief 获取string类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char* 符合类型标签的字符串，失败返回NULL。返回值需要释放
     */
    char *          kdk_conf2_get_string            (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取string类型的键默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char* 符合类型标签的字符串，失败返回NULL。返回值需要释放
     */
    char *          kdk_conf2_get_default_string    (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 设置字符串列表类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @param value 要设置的键值
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_set_strv              (KSettings          *ksettings,
                                                     const char         *key,
                                                     const char * const *value);

    /**
     * @brief 获取字符串列表类型的键值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char** 字符串列表，失败返回NULL。返回值需要释放
     */
    char **         kdk_conf2_get_strv              (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取字符串列表类型的键默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char** 字符串列表，失败返回NULL。返回值需要释放
     */
    char **         kdk_conf2_get_default_strv      (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取键的详细描述
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char* 键的描述，不存在返回NULL。返回值需要释放
     */
    char *          kdk_conf2_get_descrition        (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取键的简要描述
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char* 键的简述，不存在返回NULL。返回值需要释放
     */
    char *          kdk_conf2_get_summary           (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 获取键的类型标签字符串
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return char* 键的类型标枪字符串，返回值需要释放
     */
    char *          kdk_conf2_get_type              (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 将键值复位为默认值
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     */
    void            kdk_conf2_reset                 (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 检查键值是否可写
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return int 可视作bool类型，0表示否，1表示是
     */
    int             kdk_conf2_is_writable           (KSettings          *ksettings,
                                                     const char         *key);

    /**
     * @brief 检测组内是否存在某个键
     * 
     * @param ksettings 配置句柄
     * @param key 键名
     * @return int 可视作bool类型，0表示否，1表示是
     */
    int             kdk_conf2_has_key               (KSettings          *ksettings, 
                                                     const char         *key);

    /**
     * @brief changed信号的回调原型，注册时强转成KCallBack类型
     * 
     */
    typedef void    (*changed)                      (KSettings *ksettings,
                                                     const char *key,
                                                     void *user_data);
    /**
     * @brief 注册回调函数相应信号，信号的回调函数参数各不相同但连接时都需要强转为KCallBack类型
     * 
     * @param ksettings 配置句柄
     * @param signal_name 信号名 信号名"changed"，如果监听指定键，则写作"changed:key"
     * @param handler 回调函数
     * @param user_data 用户数据
     * @return unsigned long 信号连接id
     */
    unsigned long   kdk_conf2_connect_signal        (KSettings          *ksettings,
                                                     const char         *signal_name,
                                                     KCallBack           handler,
                                                     void               *user_data);

    /**
     * @brief 重新加载配置文件
     * 
     */
    void            kdk_conf2_reload                (void);

    /**
     * @brief 列举指定app，指定version的所有id
     * 
     * @param app 应用名
     * @param version 版本号
     * @return char** id列表，失败返回NULL。返回值需要释放
     */
    char **         kdk_conf2_list_schemas          (const char         *app,
                                                     const char         *version);

    /**
     * @brief 将配置数据以app为单位生成配置文件到指定目录
     * 
     * @param path 路径
     * @return int 可视作bool类型，0表示失败，1表示成功
     */
    int             kdk_conf2_save_user_configure   (const char         *path);

    /**
     * @brief 以旧id的配置为模板，创建一个新的id
     * 
     * @param old_id 旧的配置id
     * @param new_id 新的配置id 
     * @param version 旧id的版本号，新id将自动创建在这个版本号下
     * @return KSettings* 新id的配置句柄,失败返回NULL
     */
    KSettings *     kdk_conf2_new_extends_id        (const char         *old_id,
                                                     const char         *new_id,
                                                     const char         *version);

    /**
     * @brief 检查配置id是否存在
     * 
     * @param id 配置id
     * @param version 配置版本
     * @return int 可视作bool类型，0表示否，1表示是
     */
    int             kdk_conf2_is_schema             (const char         *id,
                                                     const char         *version);
    /**
     * @brief 获取组的简要描述
     * 
     * @param ksettings 配置句柄
     * @return char* 组的简述，不存在返回NULL。
     */
    char *          kdk_conf2_get_schema_summary    (KSettings          *ksettings);

    /**
     * @brief 获取组的纤细描述
     * 
     * @param ksettings 配置句柄
     * @return char* 组的描述，不存在返回NULL。
     */
    char *          kdk_conf2_get_schema_desription (KSettings          *ksettings);
#if __cplusplus
}
#endif // extern "C"

#endif // KYSETTINGS_H
