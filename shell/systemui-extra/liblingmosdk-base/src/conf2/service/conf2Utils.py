import os
import re
import shutil
import yaml
import sqlite3
import logging
import traceback
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import subprocess

class Converter:
    """通用类，负责读写文件，维护统一视图/用户视图等"""
    _ro_db = '/etc/lingmo-config/user.db'
    _ro_dict = {}
    _user_dict = {}
    def __init__(self, logger:logging.Logger):
        self.logger = logger

    def update_read_only_db(self):
        """更新etc下数据库

        兼容旧版本的数据库，创建新版数据库并迁移数据
        读取/etc下所有配置并合并覆盖
        把数据写到etc的数据库中
        """
        try:
            self._update_table_structure(self._ro_db)
            self._creat_db_file(self._ro_db)
            self._load_stable_dict()
            return self._dict2db(self._ro_dict, self._ro_db)
        except:
            err_msg = traceback.format_exc()
            self.logger.error(f'update_read_only_db {err_msg}')
            return False
        
    def update_user_db(self, path):
        """更新用户数据库
        
        兼容旧版本的数据库，创建新版数据库并迁移数据
        合并/复制etc下数据库数据到用户数据库
        """
        if not os.path.exists(self._ro_db):
            self.logger.info(f'/etc/lingmo-config/user.db not existed')
            return False
        
        existed = self._update_table_structure(path)
        if not existed:
            os.makedirs(os.path.dirname(path), exist_ok=True)
            shutil.copyfile(self._ro_db, path)
            self.update_user_dict(path)
        else:
            data = self._db2dict(self._ro_db)
            if self._user_dict == {}:
                self.update_user_dict(path)
            self._del_discarded_key(self._user_dict, data)
            self._merge_dict(self._user_dict, data)

        #同步gsetting的用户设置到conf2。在应用新装包时，如果存在同名gsetting的id，会生成map.yaml文件
        if os.path.exists(f'{os.path.dirname(path)}/map.yaml'):
            self.logger.info("override gsettings config")
            with open(f'{os.path.dirname(path)}/map.yaml', 'r') as file:
                mmap = yaml.safe_load(file)
            if mmap is not None:
                for app, gsetting_id in mmap.items():
                    for version, version_dict in self._user_dict[app].items():
                        if not isinstance(version_dict, dict):
                            continue
                        self._gsettings_config_get(self._user_dict[app][version], gsetting_id)
            os.remove(f'{os.path.dirname(path)}/map.yaml')

        # 如果是root用户且第一次创建用户数据库， 至此流程结束
        if path.startswith('/root') and not existed:
            return True
        
        # 加载用户目录配置文件，路径在~/.config/lingmo-config/configs/
        # 等同于/etc下的配置文件，但仅在当前用户合并
        if path.startswith('/home'):
            self._load_mutable_dict(os.path.dirname(path))

        self._dict2db(self._user_dict, path)
        return True
    
    def update_user_dict(self, path):
        self._user_dict = self._db2dict(path)
    
    def set(self, path, id, version, key, value):
        try:
            self.logger.info(f'set begin')

            # 修改用户配置文件，写入到数据库
            group_list = id.split('.')

            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            # 开始写入事务
            conn.execute('BEGIN TRANSACTION')

            cursor.execute("SELECT id FROM app WHERE app_name = ?", (group_list[0],))
            app_id = cursor.fetchone()[0]

            cursor.execute("SELECT id FROM version WHERE app_id = ? AND version = ?", (app_id, version))
            version_id = cursor.fetchone()[0]

            # 查找组
            parent_id = 0
            for group_name in group_list[1:]:
                cursor.execute("SELECT id FROM configures WHERE version_id = ? AND parent = ? AND node_name = ? AND node_type = 'schema'", (version_id, parent_id, group_name))
                parent_id = cursor.fetchone()[0]

            cursor.execute("UPDATE configures SET custom_value = ? WHERE version_id = ? AND parent = ? AND node_name = ? AND node_type = 'key'", (value, version_id, parent_id, key))

            conn.commit()
            conn.close()

            # 修改缓存
            tmp = self._user_dict[group_list[0]][version]
            for group in group_list[1:]:
                tmp = tmp[group]
            tmp[key]['_value'] = value

            self.logger.info(f'set end')
            return True
        
        except Exception as e:
            conn.rollback()
            conn.close()
            err_msg = traceback.format_exc()
            self.logger.error(f'set {err_msg}')
            return False
    
    # 重置配置值
    def reset(self, path, id, version, key):
        try:
            self.logger.info(f'reset begin')
            # 修改用户配置文件，写入到数据库
            group_list = id.split('.')

            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            # 开始写入事务
            conn.execute('BEGIN TRANSACTION')

            cursor.execute("SELECT id FROM app WHERE app_name = ?", (group_list[0],))
            app_id = cursor.fetchone()[0]

            cursor.execute("SELECT id FROM version WHERE app_id = ? AND version = ?", (app_id,version))
            version_id = cursor.fetchone()[0]

            # 查找组
            parent_id = 0
            for group_name in group_list[1:]:
                cursor.execute("SELECT id FROM configures WHERE version_id = ? AND parent = ? AND node_name = ? AND node_type = 'schema'", (version_id, parent_id, group_name))
                parent_id = cursor.fetchone()[0]

            cursor.execute("UPDATE configures SET custom_value = NULL WHERE version_id = ? AND parent = ? AND node_name = ? AND node_type = 'key'", (version_id, parent_id, key))

            conn.commit()
            conn.close()

            tmp = {}
            tmp = self._user_dict[group_list[0]][version]
            for group in group_list[1:]:
                tmp = tmp[group]
            if '_value'  in tmp[key]:
                tmp[key].pop('_value')
            
            self.logger.info(f'reset end')
            
            return True
        
        except Exception as e:
            conn.rollback()
            conn.close()
            err_msg = traceback.format_exc()
            self.logger.error(f'set {err_msg}')
            return False

    # 配置数据导出到文件
    def save(self, db_file, dest_path):
        try:
            self.logger.info(f'save begin')

            if not os.path.exists(dest_path):
                self.logger.error(f'{dest_path} not exists')
                return False
            
            data = self._db2dict(db_file)

            if data == {}:
                self.logger.error(f'read sqlite failed')
                return False
            
            #将用户配置写入到复制的只读视图里
            self._value_override_default(data)

            for key in data:
                with open(f'{dest_path}/{key}.yaml', 'w') as yaml_file:
                    yaml_file.write(yaml.safe_dump({key:data[key]}, allow_unicode = True))

            self.logger.info(f'save end')
            return True
        
        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'save {err_msg}')
            return False

    # 用于kconf2-editor保存文件，仅有普通用户权限，root用户没有图形化界面 
    def editor_save(self, apps:list, data, dest_path):
        try:
            self.logger.info(f'editor_save begin')
            
            #将用户配置写入到复制的只读视图里
            self._value_override_default(data)

            for app in apps:
                with open(f'{dest_path}/{app}.yaml', 'w') as yaml_file:
                    yaml_file.write(yaml.safe_dump({app:data[app]}, allow_unicode = True))

            self.logger.info(f'editor_save end')
            return True
        
        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'editor_save {err_msg}')
            return False

    # 创建新id内容继承自旧id。对应gsettings重定向
    def extends_id(self, old_id:str, new_id:str, version:str, target_file):
        try:
            self.logger.info(f'extends_id begin')

            target_view = self._db2dict(target_file)
            
            #获取app的引用
            list = old_id.split('.', -1)
            app = target_view.get(list[0])

            if app is None:
                self.logger.error(f'{old_id} is not exists')
                return False
            
            #获取目标版本的引用，如果为空，获取默认版本
            if version == '':
                old_data = app[app['_default_version']]
            else:
                old_data = app.get(version)

            if old_data is None:
                self.logger.error(f'{version} is not invalid')

            for iter in list[1:]:
                old_data = old_data.get(iter)
                if old_data is None:
                    self.logger.error(f'{old_id} is not exists')
                    return False
                
            tmp = {}
            tmp.update(old_data)

            list = new_id.split('.', -1)
            new_data = target_view.get(list[0])
            if new_data != app:
                self.logger.error(f'{new_id} is invalid')

            if version == '':
                new_data = app[app['_default_version']]
            else:
                new_data = app.get(version)
            
            for iter in list[1:]:
                if iter not in new_data:
                    new_data[iter] = {}
                new_data = new_data[iter]
            
            new_data.update(tmp)
            new_data['_extends'] = old_id
            self.logger.info(f'extends_id end')
            return self._dict2db(target_view, target_file)
        
        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'extends_id {err_msg}')
            return False
    
    # 读取/etc/lingmo-config/conf2.yaml，确定需要读取的目录及生效优先级
    def load_dirs_config(self, dirs:list) -> dict:
        try:
            result = {}
            for dir in dirs:
                list = self._sort_diretory(dir)
                for file in list:
                    # self.logger.info('Read conifg file')
                    data = self._load_one_file(f'{dir}/{file}')
                    # self.logger.info('Read conifg file')

                    if not data or not isinstance(data, dict):
                        continue
                    
                    # self.logger.info('Merger in view')
                    is_group_file = False
                    for value in data.values():
                        if isinstance(value, dict):
                            is_group_file = True if '_default' in next(iter(value.values())) else False
                            break
                    if is_group_file:
                        self._common_override_by_final(result, data)
                    else:
                        self._merge_dict_by_final(result, data)
                    # self.logger.info('Merger in view end')
            return result
        except:
            return {}

    # 读etc下配置文件，合并覆盖为一个字典
    def _load_stable_dict(self):
        self.logger.info('Read stable dirs')

        self._ro_dict.clear()

        with open('/etc/lingmo-config/conf2.yaml', 'r') as file:
            configures = yaml.safe_load(file)

        for dir in configures['dirs']:
            list = self._sort_diretory(f'/etc/lingmo-config/{dir}')
            for file in list:
                # self.logger.info('Read conifg file')
                data = self._load_one_file(f'/etc/lingmo-config/{dir}/{file}')
                # self.logger.info('Read conifg file')

                if not data or not isinstance(data, dict):
                    continue
                
                # self.logger.info('Merger in view')
                is_group_file = False
                for value in data.values():
                    if isinstance(value, dict):
                        is_group_file = True if '_default' in next(iter(value.values())) else False
                        break
                if is_group_file:
                    self._common_override_by_final(self._ro_dict, data)
                else:
                    self._merge_dict_by_final(self._ro_dict, data)
                # self.logger.info('Merger in view end')
        self.logger.info('Read stable dirs end')

    # 读用户配置文件，合并覆盖到用户数据。用户配置文件路径在~/.config/lingmo-config/configs/
    def _load_mutable_dict(self, dir):
        self.logger.info('Read mutable dirs')

        list = self._sort_diretory(f'{dir}/configs')
        for file in list:
            # self.logger.info('Read conifg file')
            data = self._load_one_file(f'{dir}/configs/{file}')
            # self.logger.info('Read conifg file')

            if not data or not isinstance(data, dict):
                continue
            
            # self.logger.info('Merger in view')
            is_group_file = False
            for value in data.values():
                if isinstance(value, dict):
                    is_group_file = True if '_default' in next(iter(value.values())) else False
                    break
            if is_group_file:
                self._common_override_by_stable(self._user_dict, data)
            else:
                self._merge_dict_by_stable(self._user_dict, data)
            # self.logger.info('Merger in view end')
        self.logger.info('Read mutable dirs end')

    # 按规范对文件名进行排序
    def _sort_diretory(self, yaml_dir):
        if not os.path.exists(yaml_dir):
            return []

        # 获取目录内所有文件名
        file_list = os.listdir(yaml_dir)

        # 定义排序函数
        def custom_sort(file_name):
            match = re.match(r"(\d+)?-?(.*)", file_name)
            if match.group(1):
                return (int(match.group(1)), match.group(2))
            else:
                return (0, match.group(2))

        sorted_file_list = sorted(file_list, key=custom_sort)
        return sorted_file_list
        
    # 读取一个文件
    def _load_one_file(self, file:str):
        try:
            chinese_pattern = re.compile(u'[\u4e00-\u9fa5]+')
            if bool(chinese_pattern.search(file)) or file.startswith('.'):
                return {}

            yaml_data = {}
            if file.endswith('.yaml'):
                with open(file,'r') as stream:
                    yaml_data = yaml.safe_load(stream)
                if yaml_data is None:
                    self.logger.error(f'Load {file} failed. Check the text formatting')
                    return {}
            return yaml_data
        except Exception :
            err_msg = traceback.format_exc()
            self.logger.error(f'load {file} fail: {err_msg}')
            return {}

    # 合并两个字典
    def _merge_dict(self, dest:dict , src:dict):
        if src == {}:
            return
        
        if dest == {}:
            dest.update(src)
            return
        
        for key in src.keys():
            if key in dest :
                if isinstance(dest[key], dict) and isinstance(src[key], dict):
                    self._merge_dict(dest[key], src[key])
                else:
                    dest[key]  = src[key]
            else:
                dest[key] = src[key]
        return
    
    # 合并两个字典, 处理final权限
    def _merge_dict_by_final(self, dest:dict , src:dict):
        if src == {}:
            return
        
        if dest == {}:
            dest.update(src)
            return
        if '_permission' in dest:
            if dest['_permission'] == 'final':
                return
        
        for key in src.keys():
            if key in dest :
                if isinstance(dest[key], dict) and isinstance(src[key], dict):
                    self._merge_dict_by_final(dest[key], src[key])
                else:
                    dest[key]  = src[key]
            else:
                dest[key] = src[key]
        return
    
    # 合并两个字典, 处理stable权限
    def _merge_dict_by_stable(self, dest:dict , src:dict):
        if src == {}:
            return
        
        if dest == {}:
            dest.update(src)
            return
        
        if '_permission' in dest:
            if dest['_permission'] == 'final' or dest['_permission'] == 'stable':
                return
        
        for key in src.keys():
            if key in dest :
                if isinstance(dest[key], dict) and isinstance(src[key], dict):
                    self._merge_dict_by_stable(dest[key], src[key])
                else:
                    dest[key]  = src[key]
            else:
                dest[key] = src[key]
        return

    # 全局配置字典覆盖,处理final权限
    def _common_override_by_final(self, dest:dict, src:dict):
        for app in dest:
            for version in dest[app]:
                for group in dest[app][version]:
                    if group in src:
                        self._merge_dict_by_final(dest[app][version][group], src[group])
    
    # 全局配置字典覆盖,处理stable权限
    def _common_override_by_stable(self, dest:dict, src:dict):
        for app in dest:
            for version in dest[app]:
                for group in dest[app][version]:
                    if group in src:
                        self._merge_dict_by_stable(dest[app][version][group], src[group])

    # 字典写入到数据库
    def _dict2db(self, dict:dict, path:str):
        try:
            self.logger.info('write config in database')
            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            conn.execute('BEGIN TRANSACTION')
            cursor.execute("SELECT name FROM sqlite_master WHERE type='table'")
            tables = cursor.fetchall()

            # 遍历所有表名，对每个表执行DELETE语句以删除所有行
            for table in tables:
                cursor.execute(f"DELETE FROM {table[0]}")

            app_id = 0
            version_id = 0
            apps = list(dict.keys())
            for i in range(0, len(apps)):
                app = apps[i]
                app_id = app_id + 1
                default_version = self._calculate_default_version(dict[app])
                cursor.execute("INSERT INTO app (app_name, default_version) VALUES (?,?)",(app, default_version))

                versions = list(dict[app].keys())
                for j in range(0, len(versions)):
                    version = versions[j]
                    if version == '_default_version':
                        continue

                    version_id = version_id + 1
                    cursor.execute("INSERT INTO version (app_id, version, compatible) VALUES (?, ?, ?)",
                                (app_id, version, dict[app][version].get('_compatible')))
                    
                    configures = dict[app][version]
                    self._write_group_in_db(configures, configures, cursor, version_id, 0)
            conn.commit()
            conn.close()
            self.logger.info('write config in database end')
            return True
        
        except:
            err_msg = traceback.format_exc()
            self.logger.error(f'_dict2db {err_msg}')
            conn.rollback()
            conn.commit()
            return False
            
    # 计算默认版本
    def _calculate_default_version(self, data:dict):
        versions = list(data.keys())
        if '_default_version' in versions:
            return data.get('_default_version')
        default_version = ''
        for version in versions:
            if default_version == '':
                default_version = version
                continue
            default_nums = re.findall(r'\d+', default_version)
            nums = re.findall(r'\d+', version)
            for i in range(0, 6):
                if nums[i] > default_nums[i]:
                    default_version = version
                    break
        return default_version
        
    #递归写入组到configures表
    def _write_group_in_db(self, version_data:dict, data: dict, cursor: sqlite3.Cursor, version_id, parent_id):
        groups = []
        for key, value in data.items():
            if isinstance(value, dict):
                if(value.get('_default') is None):
                    groups.append(key)
                else:
                    self._write_key_in_db(version_data, key, value, cursor, version_id, parent_id, data.get('_permission'))

        for group in groups:
            node_name = group

            node_type = 'schema'

            permission = data[group].get('_permission')
            if permission is None:
                permission = 'public'

            description = data[group].get('_description')

            summary = data[group].get('_summary')
            
            extends = data[group].get('_extends')

            cursor.execute('''INSERT INTO configures 
                           (version_id, node_name, node_type, permission, description, summary, parent, extends) 
                           VALUES (?, ?, ?, ?, ?, ?, ?, ?)''',
                           (version_id, node_name, node_type, permission, description, summary, parent_id, extends))
            group_id = cursor.lastrowid
            self._write_group_in_db(version_data, data[group], cursor, version_id, group_id)

    # 写入键的数据到configures表内
    def _write_key_in_db(self, version_data:dict, key:str, value:dict, cursor: sqlite3.Cursor, version_id, parent_id, parent_permission):
        node_name = key

        node_type = 'key'

        permission = value.get('_permission')
        if permission is None:
            if parent_permission is None:
                permission = 'public'
            else:
                permission = parent_permission

        description = value.get('_description')

        summary = value.get('_summary')

        value_type = value.get('_type')

        custom_value = value.get('_value')

        default_value = value.get('_default')
        if isinstance(default_value, bool):
            default_value = 'true' if default_value else 'false'
        
        range = value.get('_range')
        if value_type == 'enum' and isinstance(range, str) and range.startswith('@'):
            enum_range = {}
            element_list = self._recursive_search(version_data, value['_range'][1:])
            for element in element_list:
                num = element['_value']
                if isinstance(num, str):
                    if num.startswith('0x'):
                        num = int(num, 16)
                    else:
                        num = int(num, 10)
                enum_range[element['_nick']] = num
            range = str(enum_range)
        elif value_type == 'bool':
            value_type = 'b'
        elif value_type == 'int':
            value_type = 'i'
        elif value_type == 'int64':
            value_type = 'x'
        elif value_type == 'uint':
            value_type = 'u'
        elif value_type == 'uint64':
            value_type = 't'
        elif value_type == 'double':
            value_type = 'd'
        elif value_type == 'string':
            value_type = 's'
        
        cursor.execute('''INSERT INTO configures 
                       (version_id, node_name, node_type, permission, description, summary, parent, value_type, custom_value, default_value, range) 
                       VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''',
                       (version_id, node_name, node_type, permission, description, summary, parent_id, value_type, custom_value, default_value, range))

    # 查询枚举类型数据的枚举定义
    def _recursive_search(self, dictionary, key):
        if key in dictionary:
            return dictionary[key]
        else:
            for value in dictionary.values():
                if isinstance(value, dict):
                    result = self._recursive_search(value, key)
                    if result is not None:
                        return result
        return None

    # 读数据库数据到字典
    def _db2dict(self, path):
        try:
            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            cursor.execute('SELECT * FROM app')
            apps = cursor.fetchall()

            cursor.execute('SELECT * FROM version')
            versions = cursor.fetchall()

            cursor.execute('SELECT * FROM configures')
            configures = cursor.fetchall()
            conn.close()

            data = {}
            stack = []
            i = j = k = 0
            for i in range(len(apps)):
                app = apps[i][1]
                app_id = i + 1
                default_value = apps[i][2]
                data[app] = {}
                if(default_value):
                    data[app]['_default_version'] = default_value
                for j in range(j, len(versions)):
                    # app_id改变，退出版本循环
                    version_app_id = versions[j][1]
                    if version_app_id != app_id:
                        break
                    version = versions[j][2]
                    data[app][version] = {}
                    if versions[j][3]:
                        data[app][version]['compatible'] = versions[j][3]

                    stack.append((0, data[app][version]))
                    # 如果占用内存过大，可以改为在这里读取对应版本的所有配置
                    # 不再保存k值，每次循环从0开始
                    for k in range(k, len(configures)):
                        # version_id改变，退出循环
                        if configures[k][1] != j + 1:
                            stack.clear()
                            break
                        
                        while configures[k][7] != stack[-1][0]:
                            stack.pop()
                        
                        node_name = configures[k][2]
                        node_type = configures[k][3]
                        permission = configures[k][4]
                        description = configures[k][5]
                        summary = configures[k][6]
                        value_type = configures[k][8]
                        custom_value = configures[k][9]
                        default_value = configures[k][10]
                        value_range = configures[k][11]
                        extends = configures[k][12]

                        if node_type == 'schema':
                            tmp = stack[-1][1]
                            tmp[node_name] = {}
                            stack.append((configures[k][0], tmp[node_name]))
                            if permission is not None:
                                tmp[node_name]['_permission'] = permission
                            if description is not None:
                                tmp[node_name]['_description'] = description
                            if summary is not None:
                                tmp[node_name]['_summary'] = summary
                            if extends is not None:
                                tmp[node_name]['_extends'] = extends
                        elif node_type == 'key':
                            tmp = stack[-1][1]
                            tmp[node_name] = {}
                            if permission is not None:
                                tmp[node_name]['_permission'] = permission
                            if description is not None:
                                tmp[node_name]['_description'] = description
                            if summary is not None:
                                tmp[node_name]['_summary'] = summary
                            if value_type is not None:
                                tmp[node_name]['_type'] = value_type
                            if custom_value is not None:
                                tmp[node_name]['_value'] = custom_value
                            if default_value is not None:
                                tmp[node_name]['_default'] = default_value
                            if value_range is not None:
                                tmp[node_name]['_range'] = value_range
            return data
        except:
            err_msg = traceback.format_exc()
            self.logger.error(f'_db2dict {err_msg}')
            return {}

    # 删除dest中存在，但不存在与src中的键或组
    def _del_discarded_key(self, dest:dict, src:dict):
        keys_to_delete = []
        for key in dest:
            if not isinstance(dest[key], dict):
                continue
            if key not in src:
                if dest[key].get('_extends') is None:
                    keys_to_delete.append(key)
            else:
                self._del_discarded_key(dest[key], src[key])

        for key in keys_to_delete:
            dest.pop(key)

    # 导出时，使用value的值替带default值
    def _value_override_default(self, data:dict):
        for key, value in data.items():
            if isinstance(value, dict):
                self._value_override_default(value)
            else:
                if '_value' in data:
                    data['_default'] = data.pop('_value')
                return

    # 创建数据库以及表
    def _creat_db_file(self, path):
        conn = sqlite3.connect(path)
        cursor = conn.cursor()

        conn.execute('BEGIN TRANSACTION')
        cursor.execute('''CREATE TABLE IF NOT EXISTS app
                    (id INTEGER PRIMARY KEY AUTOINCREMENT,
                    app_name TEXT,
                    default_version TEXT)''')
        cursor.execute('''CREATE TABLE IF NOT EXISTS version
                        (id INTEGER PRIMARY KEY AUTOINCREMENT,
                        app_id INTEGER,
                        version TEXT,
                        compatible TEXT,
                        FOREIGN KEY (app_id) REFERENCES app(id))''')
        cursor.execute('''CREATE TABLE IF NOT EXISTS configures
                        (id INTEGER PRIMARY KEY AUTOINCREMENT,
                        version_id INTEGER,
                        node_name TEXT,
                        node_type TEXT,
                        permission TEXT,
                        description TEXT,
                        summary TEXT,
                        parent INTEGER,
                        value_type TEXT,
                        custom_value TEXT,
                        default_value TEXT,
                        range TEXT,
                        extends TEXT,
                        FOREIGN KEY (version_id) REFERENCES version(id),
                        FOREIGN KEY (parent) REFERENCES configures(id))''')
        conn.commit()
        conn.close()
    
    # 将gsettings的用户配置写入到conf2数据库
    def _gsettings_config_get(self, data, id):
        for key, value in data.items():
            if not isinstance(value, dict):
                continue
            if '_default' in value:
                result = subprocess.run(['gsettings','get', id, key], capture_output=True, text=True)
                custom_value =  result.stdout.strip()
                custom_value = custom_value.split(' ')[-1] if custom_value.startswith('@') else custom_value
                if custom_value:
                    value['_value'] = custom_value
            else:
                self._gsettings_config_get(value, f'{id}.{key}')

    # 更新表结构并完成数据迁移，共两次变更。
    # 一次是修改旧结构为新结构，新旧表结构实现里有
    # 一次是增加extends列
    def _update_table_structure(self, path):
        try:
            if not os.path.exists(path):
                return False
            
            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            cursor.execute('BEGIN TRANSACTION')

            # 检查是否是旧的表结构
            is_old = False
            has_extends = False
            cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='configures';")
            table_exists = cursor.fetchone()
            if table_exists:
                cursor.execute("PRAGMA table_info(configures);")
                columns = cursor.fetchall()
                
                for column in columns:
                    if column[1] == 'group_name':
                        is_old = True
                    if column[1] == 'extends':
                        has_extends = True
            else:
                conn.commit()
                conn.close()
                os.remove(path)
                self.logger.info(f'Data base table is invalid. Remove file {path}')
                return False
            
            if is_old:
                self.logger.info('Data base table is old structure. Update structure')
                # 如果是基础数据库，直接删掉重新生成
                if path  == self._ro_db:
                    os.remove(path)
                else:
                    cursor.execute('''CREATE TABLE IF NOT EXISTS new_configures
                            (id INTEGER PRIMARY KEY AUTOINCREMENT,
                            version_id INTEGER,
                            node_name TEXT,
                            node_type TEXT,
                            permission TEXT,
                            description TEXT,
                            summary TEXT,
                            parent INTEGER,
                            value_type TEXT,
                            custom_value TEXT,
                            default_value TEXT,
                            range TEXT,
                            extends TEXT,
                            FOREIGN KEY (version_id) REFERENCES version(id),
                            FOREIGN KEY (parent) REFERENCES configures(id))''')
                    if has_extends:
                        cursor.execute("INSERT INTO new_configures (version_id, node_name, node_type, permission, description, summary, parent, value_type, custom_value, default_value, range, extends) "
                                    "SELECT "
                                    "version_id,"
                                    "CASE "
                                    "WHEN property IS NOT NULL THEN property "
                                    "WHEN group_name IS NOT NULL THEN group_name "
                                    "END, "
                                    "CASE "
                                    "WHEN property IS NOT NULL THEN 'key' "
                                    "WHEN group_name IS NOT NULL THEN 'schema' "
                                    "END,"
                                    "permission,"
                                    "description,"
                                    "summary,"
                                    "parent,"
                                    "data_type,"
                                    "user_value,"
                                    "default_value,"
                                    "range,"
                                    "CASE "
                                    "WHEN 'extends' IN (SELECT name FROM pragma_table_info('configures')) THEN extends "
                                    "ELSE NULL "
                                    "END "
                                    "FROM configures;")
                    else:
                        cursor.execute("INSERT INTO new_configures (version_id, node_name, node_type, permission, description, summary, parent, value_type, custom_value, default_value, range, extends) "
                                    "SELECT "
                                    "version_id,"
                                    "CASE "
                                    "WHEN property IS NOT NULL THEN property "
                                    "WHEN group_name IS NOT NULL THEN group_name "
                                    "END, "
                                    "CASE "
                                    "WHEN property IS NOT NULL THEN 'key' "
                                    "WHEN group_name IS NOT NULL THEN 'schema' "
                                    "END,"
                                    "permission,"
                                    "description,"
                                    "summary,"
                                    "parent,"
                                    "data_type,"
                                    "user_value,"
                                    "default_value,"
                                    "range,"
                                    "NULL "
                                    "FROM configures;")
                    # 删除原表
                    cursor.execute("DROP TABLE configures;")

                    # 将新表重命名为原表的名称
                    cursor.execute("ALTER TABLE new_configures RENAME TO configures;")

                    # 查看app表的default_version是否存在单词拼写错误
                    cursor.execute("PRAGMA table_info(app);")
                    columns = cursor.fetchall()
                    column_names = [col[1] for col in columns]
                    if 'defualt_version' in column_names:
                        cursor.execute("ALTER TABLE app RENAME COLUMN defualt_version TO default_version;")

            conn.commit()
            conn.close()
            return True
        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'{err_msg}')
            return False

class FileWatche:
    """文件监听类,监听basci目录
    负责自动reload
    负责新配置文件是否存在gsetting配置数据需要迁移
    """
    class MyHandler(FileSystemEventHandler):
        def __init__(self, server):
            self._server = server

        def on_modified(self, event):
            if event.is_directory:
                return
            self._server.reload()

        def on_created(self, event):
            if event.is_directory or not os.path.dirname(event.src_path) == '/etc/lingmo-config/basic':
                return
            
            gsettings_id = os.path.basename(event.src_path).strip(".yaml")
            gsettings_schemas = subprocess.run(['gsettings','list-schemas'], capture_output=True, text=True)
            if gsettings_id in gsettings_schemas.stdout:
                mmap = {}
                with open(event.src_path, 'r') as file:
                    data = yaml.safe_load(file)

                for app, version in data.items():
                    if not isinstance(version, dict):
                        continue
                    mmap[app] = gsettings_id

                home_directories = [os.path.expanduser('~' + username) for username in os.listdir('/home')]
                home_directories.append('/root')
                for home_dir in home_directories:
                    file_path = os.path.join(home_dir, '.config/lingmo-config/map.yaml')
                    if not os.path.exists(os.path.dirname(file_path)):
                        continue
                    # file_path = '/home/lingmo/test.yaml'
                    if os.path.exists(file_path):
                        with open(file_path, 'r') as file:
                            tmp = yaml.safe_load(file)
                            if tmp is not None:
                                mmap.update(tmp)

                        os.remove(file_path)

                    with open(file_path, 'w') as file:
                        file.write(yaml.safe_dump(mmap))

                    os.chmod(file_path, 0o666)
            
    def __init__(self, path, server) -> None:
        self._server = server
        self._observer = Observer()
        self._event_handler = FileWatche.MyHandler(server)

        self._observer.schedule(self._event_handler, path, recursive=False)
        self._observer.start()

    def stopWatche(self):
        if self._observer.isAlive():
            self._observer.stop()
            self._observer.join()
