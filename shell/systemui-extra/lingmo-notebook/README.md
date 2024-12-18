# lingmo-notebook

灵墨便签是lingmo桌面环境的一个组件，提供便捷的文本记录和灵活的界面展示,
具有使用方便，界面简洁等优点。欢迎各位下载并试用。

## 目前实现的功能
* 便签列表
  * 实时按照修改时间倒序排序
  * 显示每条便签的修改时间和部分文本内容
  * 新建：列表条目增加并打开一个便签页
  * 搜索：匹配列表中所有便签的文本内容进行搜索
  * 删除：删除当前列表选中条目，删除后自动选中列表中上一条便签，若删除时，对应条目的便签为打开状态，则同时关闭此便签页；若无列表中无条目选中，则删除无效
  * 支持双击列表/图标条目，打开或重新激活置顶便签并获取输入焦点
* 便签页
  * 支持文本修改自动保存
  * 支持用户自定义便签头颜色并保存数据库
  * 文本修改后，此便签页对应便签列表中条目自动置顶排序
  * 便签头颜色修改后，此便签页对应便签列表中条目自动更新同步
  * 删除此便签：删除此便签，并删除此便签页对应便签列表中对应条目
  * 打开便签本：任意便签可重新唤起便签列表
  * 新建：在任一便签页新建会创建新便签页，同步到便签本
  * 关闭：关闭当前便签页，若当前便签页文本内容为空，则删除此便签，并删除此便签页对应便签本列表中条目
  * 支持加粗、斜体、下划线、删除线、无序列表、有序列表
  * 支持修改字体大小，字体颜色
