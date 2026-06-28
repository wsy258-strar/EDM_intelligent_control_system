# EDM Intelligent Control System

基于 **Qt C++** 的电火花加工（EDM）参数智能调节系统，集成**科大讯飞语音识别** + **豆包 AI 大模型**，支持语音调参、AI 参数推荐、参数持久化存储。

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-5.15.2-green.svg)](https://www.qt.io/)
[![MSVC](https://img.shields.io/badge/MSVC-2022-purple.svg)](https://visualstudio.microsoft.com/)

---

## 功能特性

| 功能 | 说明 |
|------|------|
| 语音调参 | 点击录音按钮，说出加工指令（如"调高放电脉冲时间"），自动识别并修改参数 |
| AI 参数推荐 | 输入加工需求（如"不锈钢精加工"），AI 自动推荐最优参数组合 |
| 对话展示 | Markdown 渲染的对话气泡，实时展示语音指令和 AI 推荐结果 |
| 参数管理 | 管理 20 个 EDM 核心加工参数，支持手动微调 |
| 数据持久化 | 支持 CSV 文件导出 + Access 数据库（.accdb）存储 |

---

## 环境要求

| 依赖 | 版本 / 说明 |
|------|-------------|
| Visual Studio | 2022（v143 工具集） |
| Qt | 5.15.2（msvc2019_64） |
| 科大讯飞 MSC SDK | 语音听写 Windows SDK |
| OpenSSL | 1.1.1（libssl / libcrypto） |
| Access ODBC 驱动 | 用于 .accdb 数据库读写 |
| vcpkg | 用于安装 nlohmann-json 等依赖库 |

---

## 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/wsy258-strar/new_record_stt.git
cd new_record_stt
```

### 2. 安装依赖

**通过 vcpkg 安装 nlohmann-json：**

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
vcpkg install nlohmann-json:x64-windows
vcpkg integrate install
```

**安装 OpenSSL 1.1.1：**

下载 [Win64OpenSSL-1_1_1a.msi](https://slproweb.com/products/Win32OpenSSL.html) 安装时选择 **`Copy OpenSSL DLLs to: The OpenSSL binaries (/bin) directory`**。

然后将以下 DLL 复制到编译输出目录（`x64/Debug/` 和 `x64/Release/`）：

- `libcrypto-1_1-x64.dll`
- `libssl-1_1-x64.dll`

### 3. 配置 API 密钥

**科大讯飞语音识别**（`EDM_intelligent_control_system.cpp` 第 456 行）：

```cpp
const char* login_params = "appid = 你的APPID, work_dir = .";
```

> 在[讯飞开放平台](https://console.xfyun.cn/app/myapp)创建语音听写应用获取 APPID。

**豆包 AI 接口**（`doubaoapi.h` 第 96-98 行）：

```cpp
QString OPENAI_API_KEY  = "你的豆包API密钥";
QString OPENAI_BASE_URL = "https://ark.cn-beijing.volces.com/api/v3/chat/completions";
QString OPENAI_MODEL    = "doubao-seed-1-8-251228";
```

> 在[火山引擎 Ark 平台](https://console.volcengine.com/ark/region:ark+cn-beijing/overview)开通豆包模型服务获取 API Key。

### 4. 修改本地路径

`EDM_intelligent_control_system.cpp` 中有两处硬编码路径需要改为你本机的实际路径：

**用户词表路径**（第 74 行）：

```cpp
fp = fopen("你的项目目录\\EDM_intelligent_control_system\\userwords.txt", "rb");
```

**数据库路径**（第 607 行）：

```cpp
QString filePath = QString("你的项目目录\\EDM_intelligent_control_system\\DataBase\\electric.accdb");
```

### 5. 编译运行

1. 用 Visual Studio 2022 打开 `EDM_intelligent_control_system.sln`
2. 确保 Qt VS Tools 扩展已安装并配置 Qt 5.15.2（msvc2019_64）
3. 选择 **Debug | x64** 配置
4. 生成解决方案（F7）
5. 运行（F5），工作目录自动设置为 `bin/`

---

## 项目结构

```
EDM_intelligent_control_system/
├── main.cpp                          # 程序入口
├── EDM_intelligent_control_system.h  # 主窗口 + 麦克风线程声明
├── EDM_intelligent_control_system.cpp# 核心业务逻辑（语音 / AI / 参数 / 存储）
├── EDM_intelligent_control_system.ui # 主窗口 UI 布局（Qt Designer）
├── EDM_intelligent_control_system.qrc# Qt 资源文件
├── ui.qrc                            # UI 资源
├── ChatScrollWidget.h/.cpp           # 对话气泡滚动组件（Markdown 渲染）
├── doubao.cpp / doubaoapi.h          # 豆包 AI 接口封装 + 参数解析
├── ecommand.h/.cpp                   # EDM 加工参数实体类（20 个参数）
├── speech_recognizer.c               # 讯飞语音识别核心逻辑
├── winrec.c                          # Windows 麦克风录音采集
├── include/
│   ├── speech_recognizer.h           # 语音识别头文件
│   ├── winrec.h                      # 录音采集头文件
│   ├── msp_cmn.h / msp_errors.h      # 讯飞 MSC SDK 公共头文件
│   └── qise.h / qisr.h / qtts.h     # 讯飞 MSC SDK 各模块头文件
├── DataBase/
│   └── electric.accdb                # Access 数据库模板
├── userwords.txt / userwords.json    # 语音热词配置
├── bin/                              # 运行时目录（DLL / 资源文件）
│   ├── msc_x64.dll                   # 讯飞 MSC 运行时库
│   └── msc/                          # 讯飞 MSC 日志 / 配置
└── libs/
    ├── msc.lib                       # 讯飞 MSC 链接库（32 位）
    └── msc_x64.lib                   # 讯飞 MSC 链接库（64 位）
```

### EDM 参数清单（20 个）

| 参数 | 含义 | 范围 |
|------|------|------|
| ON | 脉冲放电时间 | 0~63, 100~107 |
| OFF | 脉冲休止时间 | 0~63 |
| IP | 放电电流峰值 | 0.5 的整数倍 |
| PL | 放电极性 | + / - |
| V | 直流电压档位 | 01=90VDC, 02=120VDC |
| HP | NOW 回路 / 高压辅助 | 两位数值 |
| PP | PIKADEN 脉冲控制 | 00 / 01 / 10 / 11 |
| AL | 异常放电检验标准 | 0~63 |
| MU | 脉冲幅度放大倍率 | 0~9（×1~×10） |
| GAP | 伺服基准电压档位 | 0~9（0V~130V） |
| UP | 自动抬刀抬升时间 | 0~9 |
| DN | 自动抬刀下降时间 | 0~9 |
| CA | 极间电容器容量 | 0~9（0~1.4μF） |
| S | 伺服速度 | 0~9 |
| STEP | 摇动半径 / 放电间隔 | 0~99999 μm |
| OC / LD / LN / L / LP | 预留参数 | — |

---

## 使用指南

### 语音调参

1. 点击界面**录音按钮**开始录音
2. 说出加工指令（支持中文自然语言，如"把放电脉冲时间调到 50"）
3. 再次点击按钮停止录音，系统自动识别并填入参数

### AI 参数推荐

1. 在输入框输入加工需求描述（如"不锈钢镜面精加工，要求 Ra 0.1"）
2. 点击**确认**按钮
3. AI 自动分析需求并以 Markdown 表格返回推荐参数
4. 点击**在线优化**将 AI 推荐的参数值自动填入界面

### 参数保存

- **CSV 导出**：点击导出按钮，追加写入 CSV 文件
- **Access 数据库**：点击保存按钮，写入 .accdb 数据库（自动编号去重）

---

## 常见问题

### 语音识别无反应

- 确认讯飞 APPID 正确且应用已开通语音听写服务
- 检查麦克风可用、`msc_x64.dll` 已复制到 `bin/` 目录
- 确认网络可访问讯飞云服务

### AI 接口调用失败

- 核对豆包 API Key 和 API 地址是否正确
- 确认火山引擎 Ark 服务已开通、额度未耗尽
- 检查网络能否访问 `ark.cn-beijing.volces.com`

### 编译报错

- 必须使用 **MSVC 编译器**（MinGW 不兼容讯飞 MSC SDK）
- 确认 Qt 模块已启用：`core;network;gui;widgets;sql`
- 确认 vcpkg 已安装 `nlohmann-json:x64-windows` 并集成到 VS

### 参数保存失败

- 确认已安装 Microsoft Access Database Engine（ODBC 驱动）
- 检查 `DataBase/electric.accdb` 文件路径是否正确且可写

### 中文乱码

- 在 Visual Studio 中安装 **Force UTF-8** 扩展
- 或在 `.vcxproj` 中确认已添加编译选项 `/utf-8`
