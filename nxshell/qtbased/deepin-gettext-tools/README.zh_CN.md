# deepin-gettext-tools
处理 desktop 文件翻译的便捷工具

有关如何使用可参考项目 [deepin-feedback](https://github.com/linuxdeepin/deepin-feedback)

## 子命令介绍
有3个子命令: init, desktop2ts, ts2desktop。如果不带任何参数运行本命令则显示帮助信息。

### 子命令 init
```
deepin-desktop-ts-convert init $desktopFile $outputTsDir
```
用于在项目中开始处理一个desktop文件的翻译问题，将翻译源写入`$outputTsDir/desktop.ts`文件，和已有的翻译成果写入各个 `$outputTsDir/desktop_<lang>.ts` 文件中(`<lang>` 是指某种具体的语言代码)。

### 子命令 desktop2ts
```
deepin-desktop-ts-convert desktop2ts $desktopFile $outputTsDir
```
子命令 desktop2ts 与子命令 init 传参是一样的，用于更新翻译源文件，将翻译源写入`$outputTsDir/desktop.ts`文件。应该在 desktop 文件中的可以本地化的字段的值被更新后使用，相关字段有 Name，Comment，GenericName，Keywords。


### 子命令 ts2desktop
```
deepin-desktop-ts-convert ts2desktop $desktopFile $tsDir $outputDesktopFile
```
用于输出带有翻译结果的 desktop 文件，参数 `$desktopFile` 是模板文件，参数 `$tsDir` 是放置相关 ts 文件的文件夹，将集合了各种语言翻译成果的desktop写入`$outputDesktopFile`文件。推荐在构建软件包过程中使用。

## 推荐使用流程
### 开始一个新项目
1. 如果 desktop 文件中已有翻译，使用 init 子命令，否则使用 deskotp2ts 子命令；
2. 将 desktop 文件的后缀改为 `.desktop.in` 作为一个模板，去除里面的翻译；
3. 在 Makefile 里面写一个构建项 ts，当 desktop.in 文件中相关字段的值更新后，执行 make ts；
4. 在 .tx/config 写好资源配置；
5. 在 Makefile 中的 build 项中增加使用 ts2desktop 子命令；
6. 用 ci 的 [sync-transifex](https://ci.deepin.io/view/tools/job/sync-transifex/) 上传翻译源文件；
7. 如果已有一些翻译成果，找翻译管理员，`git clone` 你的项目，然后执行 `tx push -t` 命令上传翻译，tx 命令由软件包 transifex-client 提供；

### 更新翻译成果
1. 用 ci 的 sync-transifex 拉取翻译结果

### 更新模板文件 desktop.in，修改了本地化相关字段
1. 使用 desktop2ts 子命令；
2. 将模板文件 desktop.in 和它的翻译源文件 desktop.ts 在 git 中提交；
3. 用 ci 的 sync-transifex 上传翻译源文件；

## 帮助
任何使用问题都可以通过以下方式寻求帮助：

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [WiKi](https://wiki.deepin.org)
* [官方论坛](https://bbs.deepin.org)
* [开发者中心](https://github.com/linuxdeepin/developer-center/issues) 

## 贡献指南

我们鼓励您报告问题并做出更改

- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## 开源许可证
deepin-gettext-tools 在 [GPL-2.0-or-later](LICENSE) 下发布。
