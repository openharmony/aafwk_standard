# 元能力子系统

## 简介

**元能力子系统**实现对Ability的运行及生命周期进行统一的调度和管理，应用进程能够支撑多个Ability，Ability具有跨应用进程间和同一进程内调用的能力。Ability管理服务统一调度和管理应用中各Ability，并对Ability的生命周期变更进行管理。

![](figures/aafwk.png)

**元能力子系统架构图说明：**

- **Ability Kit**为Ability的运行提供基础的运行环境支撑。Ability是系统调度应用的最小单元，是能够完成一个独立功能的组件，一个应用可以包含一个或多个Ability。Ability分为FA（Feature Ability）和PA（Particle Ability）两种类，其中FA支持Page Ability，PA支持Service Ability和Data Ability。

- **Ability管理服务（AbilityManagerService）**：用于协调各Ability运行关系、及对生命周期进行调度的系统服务。
  - Ability栈管理模块（AbilityStackManager）负责维护各个Ability之间跳转的先后关系。
  - 连接管理模块（AbilityConnectManager）是Ability管理服务对Service类型Ability连接管理的模块。
  - 数据管理模块（DataAbilityManager）是Ability管理服务对Data类型Ability管理的模块。
  - App管理服务调度模块（AppScheduler）提供Ability管理服务对用户程序管理服务进行调度管理的能力。
  - Ability调度模块（AbilityScheduler）提供对Ability进行调度管理的能力。
  - 生命周期调度模块（LifecycleDeal）是Ability管理服务对Ability的生命周期事件进行管理调度的模块。

**Ability生命周期介绍**（Ability Life Cycle）是Ability被调度到INACTIVE、ACTIVE、BACKGROUND等各个状态的统称（主要涉及PageAbility类型和ServiceAbility类型的Ability）。

  - **PageAbility类型的Ability生命周期流转如下图所示**

![PageAbility-Lifecycle](figures/page-ability-lifecycle.png)



  - **ServiceAbility类型的Ability生命周期流转如下图所示**

![ServiceAbility-Lifecycle](figures/service-ability-lifecycle.png)

**Ability生命周期状态说明：**

  - **UNINITIALIZED**：未初始状态，为临时状态，Ability被创建后会由UNINITIALIZED状态进入INITIAL状态。

  - **INITIAL**：初始化状态，也表示停止状态，表示当前Ability未运行，Ability被启动后由INITIAL态进入INACTIVE状态。

  - **INACTIVE**：未激活状态，表示当前窗口已显示但是无焦点状态，由于Window暂未支持焦点的概念，当前状态与ACTIVE一致。

  - **ACTIVE**：前台激活状态，表示当前窗口已显示，并获取焦点，Ability在退到后台之前先由ACTIVE状态进入INACTIVE状态。

  - **BACKGROUND**: 后台状态，表示当前Ability退到后台，Ability在被销毁后由BACKGROUND状态进入INITIAL状态，或者重新被激活后由BACKGROUND状态进入ACTIVE状态。

**PageAbility类型Ability生命周期回调如下图所示：**

![PageAbility-Lifecycel-Callbacks](figures/page-ability-lifecycle-callbacks.png)



**ServiceAbility类型Ability生命周期回调如下图所示:**

![Service-Ability-Lifecycle-Callbacks](figures/service-ability-lifecycle-callbacks.jpg)



## 目录

```
foundation/
└──foundation/aafwk/standard
   ├── frameworks
   │   └── kits
   │       └── ability				# AbilityKit实现的核心代码
   ├── interfaces
   │   └── innerkits
   │       └── want						# Ability之间交互的信息载体的对外接口
   ├── services
   │   ├── abilitymgr					# Ability管理服务框架代码
   │   ├── common							# 日志组件目录
   │   └── test								# 测试目录
   └── tools							    # aa命令代码目录
```

## 使用说明
### 启动Abiltiy
启动新的ability(callback形式)

* startAbility参数描述

| 名称      | 读写属性 | 类型                  | 必填 | 描述                |
| --------- | -------- | --------------------- | ---- | ------------------- |
| parameter | 读写     | StartAbilityParameter | 是   | 表示被启动的Ability |
| callback  | 只读     | AsyncCallback         | 是   | 被指定的回调方法    |

- StartAbilityParameter类型说明

| 名称                | 读写属性 | 类型   | 必填 | 描述                               |
| ------------------- | -------- | ------ | ---- | ---------------------------------- |
| want                | 读写     | want   | 是   | 表示需要包含有关目标启动能力的信息 |
| abilityStartSetting | 只读     | string | 否   | 指示启动能力中使用的特殊启动设置   |

- want类型说明

| 名称        | 读写属性 | 类型                 | 必填 | 描述                       |
| ----------- | -------- | -------------------- | ---- | -------------------------- |
| deviceId    | 读写     | string               | 否   | 设备id                     |
| bundleName  | 读写     | string               | 否   | Bundle名                   |
| abilityName | 读写     | string               | 否   | Ability 名                 |
| uri         | 读写     | string               | 否   | 请求中URI的描述            |
| type        | 读写     | string               | 否   | Want中类型的说明           |
| flags       | 读写     | number               | 否   | Want中标志的选项，必填     |
| action      | 读写     | string               | 否   | Want中对操作的描述         |
| parameters  | 读写     | {[key: string]: any} | 否   | Want中WantParams对象的描述 |
| entities    | 读写     | string               | 否   | 对象中实体的描述           |

* 返回值

  void

* 示例

```javascript
import featureAbility from '@ohos.ability.featureAbility'
function addSlotsCallBack(err) {
	console.info("==========================>startAbility=======================>");
}
featureAbility.startAbility(
        {
        want:
        {
            action: "",
            entities: [""],
            type: "",
            options: {
                // indicates the grant to perform read operations on the URI
                authReadUriPermission: true,
                // indicates the grant to perform write operations on the URI
                authWriteUriPermission: true,
                // support forward want result to origin ability
                abilityForwardResult: true,
                // used for marking the ability start-up is triggered by continuation
                abilityContinuation: true,
                // specifies whether a component does not belong to ohos
                notOhosComponent: true,
                // specifies whether an ability is started
                abilityFormEnabled: true,
                // indicates the grant for possible persisting on the URI.
                authPersistableUriPermission: true,
                // indicates the grant for possible persisting on the URI.
                authPrefixUriPermission: true,
                // support distributed scheduling system start up multiple devices
                abilitySliceMultiDevice: true,
                // indicates that an ability using the service template is started regardless of whether the
                // host application has been started.
                startForegroundAbility: true,
                // install the specified ability if it's not installed.
                installOnDemand: true,
                // return result to origin ability slice
                abilitySliceForwardResult: true,
                // install the specified ability with background mode if it's not installed.
                installWithBackgroundMode: true
            },
            deviceId: "",
            bundleName: "com.example.startability",
            abilityName: "com.example.startability.MainAbility",
            uri: ""
        },
    },
  );
)
```
启动新的ability(Promise形式)
* startAbility参数描述

| 名称      | 读写属性 | 类型                  | 必填 | 描述                |
| --------- | -------- | --------------------- | ---- | ------------------- |
| parameter | 读写     | StartAbilityParameter | 是   | 表示被启动的Ability |

- StartAbilityParameter类型说明

| 名称                | 读写属性 | 类型   | 必填 | 描述                               |
| ------------------- | -------- | ------ | ---- | ---------------------------------- |
| want                | 读写     | want   | 是   | 表示需要包含有关目标启动能力的信息 |
| abilityStartSetting | 只读     | string | 否   | 指示启动能力中使用的特殊启动设置   |

- want类型说明

| 名称        | 读写属性 | 类型                 | 必填 | 描述                       |
| ----------- | -------- | -------------------- | ---- | -------------------------- |
| deviceId    | 读写     | string               | 否   | 设备id                     |
| bundleName  | 读写     | string               | 否   | Bundle名                   |
| abilityName | 读写     | string               | 否   | Ability 名字               |
| uri         | 读写     | string               | 否   | 请求中URI的描述            |
| type        | 读写     | string               | 否   | Want中类型的说明           |
| flags       | 读写     | number               | 否   | Want中标志的选项，必填     |
| action      | 读写     | string               | 否   | Want中对操作的描述         |
| parameters  | 读写     | {[key: string]: any} | 否   | Want中WantParams对象的描述 |
| entities    | 读写     | string               | 否   | 对象中实体的描述           |

* 返回值

  void

* 示例

```javascript
import featureAbility from '@ohos.ability.featureAbility'
featureAbility.startAbility(
    {
        want:
        {
            action: "action.system.home",
            entities: ["entity.system.home"],
            type: "MIMETYPE",
            options: {
                // indicates the grant to perform read operations on the URI
                authReadUriPermission: true,
                // indicates the grant to perform write operations on the URI
                authWriteUriPermission: true,
                // support forward want result to origin ability
                abilityForwardResult: true,
                // used for marking the ability start-up is triggered by continuation
                abilityContinuation: true,
                // specifies whether a component does not belong to ohos
                notOhosComponent: true,
                // specifies whether an ability is started
                abilityFormEnabled: true,
                // indicates the grant for possible persisting on the URI.
                authPersistableUriPermission: true,
                // indicates the grant for possible persisting on the URI.
                authPrefixUriPermission: true,
                // support distributed scheduling system start up multiple devices
                abilitySliceMultiDevice: true,
                // indicates that an ability using the service template is started regardless of whether the
                // host application has been started.
                startForegroundAbility: true,
                // install the specified ability if it's not installed.
                installOnDemand: true,
                // return result to origin ability slice
                abilitySliceForwardResult: true,
                // install the specified ability with background mode if it's not installed.
                installWithBackgroundMode: true
            },
            deviceId: "",
            bundleName: "com.example.startability",
            abilityName: "com.example.startability.MainAbility",
            uri: ""
        },
    }
   ).then((void) => {
	console.info("==========================>startAbility=======================>");
});
```

### 停止Ability

设置page ability返回给被调用方的结果代码和数据，并销毁此page ability(callback形式)

* 参数描述

| 名称     | 读写属性 | 类型                 | 必填 | 描述             |
| -------- | -------- | -------------------- | ---- | ---------------- |
| callback | 只读     | AsyncCallback\<void> | 是   | 被指定的回调方法 |

* 返回值

  void

* 示例

```javascript
import featureAbility from '@ohos.ability.featureability';
function terminateSelfCallBack(err) {
	console.info("==========================>terminateSelfCallBack=======================>");
}
featureAbility.terminateSelf()
```

设置page ability返回给被调用方的结果代码和数据，并销毁此page ability(Promise形式)

* 参数描述

  Null

* 返回值

* void

* 示例

```javascript
import featureAbility from '@ohos.ability.featureability';
featureAbility.terminateSelf().then((void) => {		    console.info("==========================>terminateSelfCallBack=======================>");
});
```


## **aa命令**

**aa help**

| 命令    | 描述               |
| ------- | ------------------ |
| aa help | 显示aa命令帮助信息 |

**aa start**

| 命令                                                           | 描述                      |
| -------------------------------------------------------------- | ------------------------ |
| aa start [-d <device>] -a <ability-name> -b <bundle-name> [-D] | 启动ability，设备ID 可空  |

```
示例：
aa start -d 12345 -a com.ohos.app.MainAbility -b com.ohos.app -D
```

**aa dump**

| 命令       | 描述                  |
| ---------- | --------------------- |
| aa dump -a | 打印栈中的Ability信息 |

**aa force-stop**

| 命令                                                           | 描述                      |
| -------------------------------------------------------------- | ------------------------ |
| aa force-stop <bundle-name>                                    | 强制停止application       |

```
示例：
aa force-stop com.ohos.app
```

## 相关仓
元能力子系统

appexecfwk_standard

**aafwk_standard**

