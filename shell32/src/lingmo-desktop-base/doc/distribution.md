# distribution.info 文件内容格式和说明

## 位置和格式

配置文件位于 `/usr/share/deepin/distribution.info` ，此文件中，格式为下述格式，使用基本遵循 `.desktop` [文件格式](https://specifications.freedesktop.org/desktop-entry-spec/latest/) 所描述的解析方式。由于并非完全遵循此规范，故避免混淆起见不使用 `.desktop` 作为扩展名。

此配置文件中所指向的媒体资源文件应当使用绝对路径，这些资源文件建议放置于 `/usr/share/deepin/distribution/` 目录下。

此文件用以提供发行版及其定制后应用需要呈现的公用信息。对于不需要定制的信息，组或者键可以留空（如果允许），应用在此时应当自行处理回落显示方案，例如在没有制造商相关信息时则不显示对应内容。

### 文件解析建议

可以考虑使用任何能够解析 `.desktop` 文件的库。如果可以使用 DTK，则 dtkcore 提供了 `DSysInfo` 以获取部分常用的定制配置信息，对于不常见的定制信息，可以使用 `DDesktopEntry` 帮助解析。

### 作弊表：

*后续考虑在 DTK 提供此类格式的解析支持。*

 - 使用 `#` 开头的行作为注释
 - 使用 `[GroupName]` 格式表示一个组的开始
 - 使用 `Key=Value` 的格式表示键和值
 - 使用 `Key[zh_CN]=Value` 的格式表示本地化的值
 - 字符串中特殊字符需要转义，请尽量规避使用这些字符，转义方式参见上方给定的 `desktop-entry-spec` 
 - 需要提供包含多项值的情况，使用分号 `;` 分割多个值，同样因此，字符串中分号需要转义

## 何种信息应当于此文件中提供

需要在多个不同应用下共用的定制信息，例如发行版名称和 Logo 等，以便不同应用可以按照此约定读取配置文件以供显示此类信息。

## 何种信息不应当于此文件中提供

针对于某个特定应用的定制显示内容，应当由应用自身提供定制方式。

## 约定的组名和键名

下表为约定存在的组名和键名对应关系信息。若希望新增一个组或键以供使用，请先更新下表。

### 组名表

组名 | 含义 | 解释说明
----|----|----
Distribution | 发行版信息 | 表示发行版自身的信息
Distributor | 发行厂商信息 | 表示提供发布此发行版的厂商信息
Manufacturer | 制造商信息 | 对于硬件制造商 OEM 需求，提供制造商的相关信息

### 厂商和发行版自身信息键名表

键名 | 含义 | 备注
----|----|----
Name | 发行者厂商名称 | 可本地化
Logo | 发行者厂商 Logo | 普通尺寸 Logo，适用于系统设置中的关于等位置
LogoLight | 发行者厂商 Logo ，小尺寸版本 | 在 DTK 的关于对话框中，会使用此项显示 Logo
LogoSymbolic | 发行者厂商 Logo ，符号化版本 | 目前没有组件用到，预留使用
LogoTransparent | 发行者厂商 Logo ，半透明版本 | 可以用于水印或其它需要半透明的场景的 Logo
Website | 发行者厂商网站 | 站点主页 URL
WebsiteName | 发行者厂商网站名称 | 呈现给用户的发行者网站显示名称

## 示例写法

```
[Distributor]
Name=Deepin
Name[zh_CN]=深度操作系统
Logo=/usr/share/deepin/distribution/distribution_logo.svg

[Manufacturer]
Name=Sorry
Logo=/usr/share/deepin/distribution/manufacturer_logo.svg
```
