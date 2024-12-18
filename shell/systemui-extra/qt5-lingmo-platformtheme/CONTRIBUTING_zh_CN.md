# 贡献指南

## 写给使用者

LINGMO的使用者都是此项目的用户，因为它影响了所有LINGMO上的Qt应用，我们能从一个标准qt应用的按钮圆角弧度，菜单透明度，动画以及毛玻璃特效等方面看到它对控件样式带来的影响。

我们希望用户能够指出我们在应用和主题风格设计上的不统一处，以及视觉上的不足与问题，这些都有助于我们对项目进行有针对性的提升。

当然，LINGMO主题框架不可能接管所有应用的控件风格与绘制，一个应用可能希望保持自己的主题，或者针对现有主题的不足进行改进。对于用户来说，可能很难分辨一个控件是否是属于LINGMO主题框架内的标准控件，而只能够发现风格上的不同。

无论如何，如果你找到了视觉上的问题，你都可以向我们提交issue，可以在相应的应用项目中，也可以在这里。

## 写给开发者

如果你对Qt5 QPA PlatformTheme以及QStyle比较熟悉，那么你应该能够很快的了解这个项目，并且知道如何参与其中。和其它platform theme，如qt5-gtk2-platformtheme、qt5ct、KDE等项目一样，我们希望通过提供平台插件和主题风格插件来构建我们自己的桌面环境主题，这样做将意味着不光是我们自己开发的应用，其它开发这开发的Qt应用也将会在我们的桌面环境中具有LINGMO的风格。

这种做法将从根本上解决Qt开发者不得不花费大量时间去调整自己应用的样式以达到接近LINGMO设计风格的问题。我们希望开发者花费更多时间在业务逻辑和功能实现上，并且还能够保证应用在其它平台上（如KDE）也能够具有风格的统一性。

为了实现这一目标，我需要开发者们的理解和支持。开发者首先需要知道如何以标准的方式进行应用的开发，然后还需要掌握如何在标准之上针对需求进行合理的调优。通过自己体验这样的流程，你将明白它的长远意义。

现在，我几乎将我全部的精力投入到这个项目的开发中去了，然而它的推进仍然十分缓慢。我一个人很难在短时间内解决所有的问题，所以希望有志愿者能够帮助我。我简单的列出了一些问题：

### Platform Theme层面
和其它platform theme一样，我们希望在LINGMO中提供其它platform theme所提供的特性。

- 平台对话框，比如文件选择对话框
- 平台配置，比如字体配置
- 平台扩展，比如全局菜单

当然，Gtk应用的主题风格与Qt应用的风格主题也是一个比较棘手的问题，我们需要考虑怎样合理的统一它们。

### Style层面
一个好看的主题往往是极其复杂的，比如breeze和oxygen。要构建我们自己的主题，我们需要花费大量的时间在UI设计和细节优化上。作为一名开发者，我们需要做的是将设计稿的内容转化成代码，这大致可以分为2个部分：

- 控件的样式绘制与调优，比如按钮菜单的圆角弧度，调色板中的颜色等
- 动画与控件的结合，比如标签页的滑动切换，按钮的悬浮与渐变效果等

你首先得了解QStyle和主题插件的相关流程，它们是怎么绘制一个控件，怎么让控件具有动画效果的。

### 动画框架层
动画框架一般来说基于Qt的Animation框架，它的目标是：

- 让widgets和视图元素具有动画效果，并且让状态的改版更加的平滑和炫酷

需要注意的是动画框架是相对独立的，然而它对主题的影响还是非常之大。

## 积极的交流

如果你对参与项目感兴趣，我也乐意尽力帮助你熟悉这个项目。你可以以提交issue的方式作为开始，也可以通过邮件与我交流。

Yue Lan, <lanyue@kylinos.cn>