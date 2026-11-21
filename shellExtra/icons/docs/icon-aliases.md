# 图标安装完整性

仓库部分 SVG 路径是单行文本别名，而非 SVG 图稿。安装时先合并 src 和 links，再由 tools/restore-aliases.py 转成相对符号链接，最后生成 GTK 图标缓存。主题索引中的 mimes、animations 目录也必须安装。

检查：python3 tools/audit-aliases.py 检查合并后的所有别名；python3 tests/test_aliases.py 验证链式解析、重复执行和越界拒绝。任何断链都应使安装失败，不能生成无效图标缓存。

本轮已审计 16161 条别名，修复循环引用、大小写不一致和错误相对路径，并补充 lingmo-archive-manager。临时安装检查确认计算器、归档管理器、设置与终端图标最终均为 SVG 图稿。尚需在新 ISO 上进行完整首次登录验收。
