import os


# 获取当前文件所在的目录路径
current_file_directory_path = os.path.dirname(os.path.abspath(__file__))

# 构造 QML 模块路径
qml_module_path = os.path.join(current_file_directory_path, "qml")

# 获取当前的 QML_IMPORT_PATH 环境变量值
current_path = os.environ.get('QML_IMPORT_PATH', '')

# 检查 qml_module_path 是否已经存在于 QML_IMPORT_PATH 中
if current_path:
    # 将当前路径分割为列表
    paths = current_path.split(os.pathsep)
    # 如果 qml_module_path 不在当前路径列表中，则追加
    if qml_module_path not in paths:
        updated_path = current_path + os.pathsep + qml_module_path
    else:
        # 如果已经存在，则不修改
        updated_path = current_path
else:
    # 如果 QML_IMPORT_PATH 为空，则直接设置为 qml_module_path
    updated_path = qml_module_path

# 更新环境变量
os.environ['QML_IMPORT_PATH'] = updated_path