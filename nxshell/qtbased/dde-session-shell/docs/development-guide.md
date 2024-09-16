# dde-session-shell Development guide

- [概述](#概述)
- [登录插件](#登录插件)
  - [本地账户](#本地账户)
  - [域管账户](#域管账户)
  - [远程账户](#远程账户)

## 概述

插件开发依赖 dde-session-shell-dev 开发包。

模块化接口头文件路径: `/usr/include/dde-session-shell`，接口说明参考各接口类中的注释

- base_module_interface.h 定义了接口类必须实现且共用的方法，是所有接口类的基类。
- login_module_interface.h 定义了登录模块必须实现的方法，继承接口基类。
- tray_module_interface.h 文件定义了右下角区域模块必须实现的方法，继承接口基类。

## 登录插件

```cpp
// login_module_interface.h
/**
 * @brief 认证插件需要传回的数据
 */
struct AuthCallbackData{
    int result;          // 认证结果
    std::string account; // 账户
    std::string token;   // 令牌
    std::string message; // 提示消息
    std::string json;    // 预留数据
};

/**
 * @brief 回调函数
 * AuthData: 需要传回的数据
 * void *: ContainerPtr
 */
using AuthCallbackFun = void (*)(const AuthCallbackData *, void *);

/**
 * @brief 插件回调相关
 */
struct AuthCallback {
    void *app_data;                // 插件无需关注
    AuthCallbackFun callbackFun;   // 回调函数
};

class LoginModuleInterface : public BaseModuleInterface
{
public:
    /**
     * @brief 模块的类型
     * @return ModuleType
     */
    ModuleType type() const override { return LoginType; }

    /**
     * @brief 认证完成后，需要调用回调函数 CallbackFun
     */
    virtual void setAuthFinishedCallback(AuthCallback *) = 0;
};
```

登录插件涉及多种登录方式，不局限于本地账户登录，可能包含多种远程账户登录，总的来说，分为三种情况：

### 本地账户

本地账户由于账户信息存储在本地，走常规流程，不做过多说明。

### 域管账户

域管账户登录时，可能属于第一次登录，本地没有此账户信息，在常规流程中，增加了创建账户信息的操作，此流程由域管完成。当账户由域管验证合法后，调用系统接口，创建本地用户信息，后续由 lightdm 接管进入系统。

### 远程账户

远程账户不能凭空登录机器，可以采用 windows 类似的方案。

- 新建账户，在重装系统第一次开机时，允许通过远程账户登录，登录成功后，自动创建一个本地账户与之绑定；
- 已有账户，先创建一个本地账户登录，后续在控制中心操作与远程账户绑定。

总的来说，远程账户必须与一个本地账户绑定，且同一个机器绑定信息唯一。这样我们就可以将用户确定，为保证后续既可以输入本地账户密码登录也可以输入远程账户密码或扫码登录等远程登录方式，远端服务器需要在账户绑定初期生成一个唯一识别码，此识别码作为回调函数中的`token`使用。

远程账户可以登录多台机器，绑定多台机器中的本地账户，为了解决远程账户与本地账户对应的问题，可以采用远程记录数据或本地记录数据的方式。

- 远程记录本地账户信息，此方式需要远程账户记录绑定的本地账户信息，在不做限制的情况下，数据量可能会很多，后续每次登录，都需要遍历这些数据，导致登录耗时过长。
- 本地机器记录远程账户信息，本地账户增加字段用于记录自己绑定的远程账户信息，字段内容为远程账户在服务器中创建时生成的唯一 id 和绑定本地账户时生成的唯一识别码。未绑定远程账户的本地账户此字段未空。此唯一识别码可由非对称加密的公钥组成，远程账户提前生成一对公私钥，绑定本地账户时，将公钥给本地账户保存，远程账户登录时，将认证结果通过回调函数发送给登录程序。`AuthCallbackData`中的`account`传递远端账户的 id，`token`传递公钥，`result`传递认证结果，`message`传递认证提示消息，可用于界面显示。登录程序在收到这些数据后，在本地账户中找到匹配`account`和`token`的账户，登录对应账户的桌面环境。

为增加安全性，可在系统中增加公私钥过期状态的接口，当账户登录成功，由控制中心与远端发起请求更新各自的密钥，但此处又涉及到远端可能需要保存多份密钥数据的问题，与上述记录本地账户数据类似，当数据量变大时，会导致登录耗时过长。。。
