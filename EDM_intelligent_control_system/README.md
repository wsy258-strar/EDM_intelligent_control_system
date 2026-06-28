# 电火花加工参数智能调节系统

## 📖 项目简介

基于 **Qt C++** 开发的电火花加工参数智能配置工具，集成**科大讯飞语音识别** + **豆包 AI 大模型**，支持**语音控制调参**、**文本指令 AI 推荐参数**、**参数本地保存**，零门槛实现电火花加工参数自动化配置。

------

## 🚀 快速运行（核心步骤）

### 1. 环境准备（必须）

1. **开发工具**：Qt 
2. **编译器**：MSVC 2017/2019（32/64 位）
3. **依赖 SDK**：科大讯飞 MSC 语音听写 SDK
4. **网络**：可访问公网（AI 接口、语音识别需要）
5. **驱动**：Microsoft Access ODBC 驱动（参数数据库存储）

### 2. 项目配置（关键！）

#### （1）配置科大讯飞语音识别

1. 登录讯飞[开放平台](https://console.xfyun.cn/app/myapp)，创建**语音听写**应用，获取 `APPID`
2. 打开项目中语音配置文件`new_record_stt.cpp`，替换为你的 `APPID`，具体位置：`const char* login_params = "appid = 8257763a, work_dir = ."; `
3. 导入头文件并将讯飞 SDK 的库文件（`msc.lib`/`msc.dll`）放入项目执行目录，具体可参考：[语音听写 Windows SDK 文档](https://www.xfyun.cn/doc/asr/voicedictation/Windows-SDK.html#_1%E3%80%81%E7%AE%80%E4%BB%8B)

#### （2）配置豆包 AI 接口

1. [登录豆包开放平台](https://console.volcengine.com/ark/region:ark+cn-beijing/overview?briefPage=0&briefType=introduce&type=new)，点击模型广场，选择Doubao-Seed-1.8，点击API接入，获取 **API Key / API 地址**

2. 在 `doubaoapi.h`中修改配置：

   ```
   #define OPENAI_API_KEY    "你的豆包API密钥"
   #define OPENAI_BASE_URL   "https://ark.cn-beijing.volces.com/api/v3/chat/completions"
   ```

#### （3）资源文件准备

将 `userwords.json`、`userwords.txt`（语音关键词配置）复制到**编译生成的 exe 同级目录**。

#### （4）路径代码修改

在`new_record_stt.cpp`中：

1. `fp = fopen("C:\\Users\\32284\\source\\repos\\new_record_stt\\new_record_stt\\userwords.txt", "rb");`改为本机的地址
2. `QString filePath = QString("C:\\Users\\32284\\source\\repos\\new_record_stt\\new_record_stt\\DataBase\\electric.accdb");`改为本地的数据库地址

------

## 📁 核心文件说明

| 文件名                           | 作用                                      |
| -------------------------------- | ----------------------------------------- |
| `ChatScrollWidget.h/.cpp`        | 对话界面组件（展示用户指令 + AI 回答）    |
| `doubao.h/.cpp` / `doubaoapi.h`  | 豆包 AI 接口封装、参数解析                |
| `ecommand.h/.cpp`                | 电火花加工参数实体类（20 + 核心参数管理） |
| `new_record_stt.h/.cpp`          | 主业务逻辑（调参、语音、AI 调度中心）     |
| `speech_recognizer.c`/`winrec.c` | 讯飞语音识别、录音采集底层逻辑            |
| `userwords.json/txt`             | 语音关键词配置（语音→参数映射）           |
| `main.cpp`                       | 程序入口                                  |

# 环境

### 1. 安装vcpkg包管理器，项目中缺少哪些包就可以便捷安装

#### 安装vcpkg

打开 **VS2022 开发者命令提示符 (64 位)** 执行：

```cpp
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
```

#### 安装nlohmann

- `vcpkg install nlohmann-json:x64-windows`
- 集成到 VS2022（让 VS 自动识别库）:`vcpkg integrate install`

#### 后面如果有提示缺少非QT的库时 都可以使用vcpkg安装一下

### 2. 安装openssl1.1.1

- 点击安装`openssl/Win64OpenSSL-1_1_1a.msi`

- 安装过程中，一定要选：

  **`Copy OpenSSL DLLs to: The OpenSSL binaries (/bin) directory`**

  不要装到系统目录！

- 把 2 个 dll 复制到你的程序运行目录

  - 打开你的安装目录：`你的目录\OpenSSL-Win64\bin`

  - 复制这 **2 个文件**：

    - `libcrypto-1_1-x64.dll`

    - `libssl-1_1-x64.dll`

  - 粘贴到你项目的 编译输出目录：

    ```
    你的项目文件夹\x64\Debug\
    ```

    ```
    你的项目文件夹\x64\Release\
    ```

  - 重启程序

### 3. VS 安装Force UTF-8插件，防止出现中文乱码(m没有乱码可以先不安装)

### 4. 添加sql到环境中

右键项目 → 「卸载项目」

右键项目 → 「编辑 `new_record_stt.vcxproj`」

找到 `<QtModules>` 节点（在 `<PropertyGroup Label="Qt">` 下），在末尾加 `;sql`：

```
<QtModules>core;gui;widgets;sql</QtModules>
```

保存文件，右键项目 → 「重新加载项目」，再重新生成解决方案

## 🎯 快速使用

1. 语音调参

   点击「录音按钮」→ 说出指令（如：调高放电脉冲时间、调低休止时间）→ 自动识别并修改参数

2. AI 推荐参数

   输入加工需求（如：不锈钢精加工）→ 点击「确认」→ AI 自动生成最优参数并填充界面

3. 保存参数

   填写加工信息 → 点击「保存」→ 支持导出 CSV / 存入 Access 数据库

4. 查看对话

   实时展示语音指令、AI 推荐结果，支持表格渲染

------

## ❌ 常见问题

### 1. 语音识别无反应

- 检查讯飞 `APPID` 是否正确
- 确认麦克风可用、讯飞 SDK 库文件已放入执行目录
- 检查网络是否正常

### 2. AI 接口调用失败

- 核对豆包 `API Key`、`API地址` 是否正确
- 检查网络能否访问公网
- 确认 API 权限已开通

### 3. 编译报错

- 切换为 **MSVC 编译器**（MinGW 不兼容讯飞 SDK）
- 检查项目模块：开启 `Qt Widgets`、`Qt Network`、`Qt Sql`

### 4. 参数保存失败

- 安装 Access ODBC 驱动
- 检查文件 / 数据库路径权限   