/**
 * @file    EDM_intelligent_control_system.h
 * @brief   主窗口及麦克风线程声明
 *
 * 本文件声明了 EDM 参数智能调节系统的核心类：
 * - MicThread：后台麦克风录音线程，封装讯飞语音识别完整流程
 * - EDMIntelligentControlSystem：主窗口类，管理 20 个 EDM 参数、语音识别交互、
 *   AI 对话优化、参数存取等全部业务逻辑
 *
 * @author wang
 * @date   2026
 */

#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_EDM_intelligent_control_system.h"
#include <QThread>
#include <QMutex>
#include "ecommand.h"
#include "chatscrollwidget.h"

/**
 * @class MicThread
 * @brief 麦克风录音后台线程
 *
 * 继承 QThread，在独立线程中运行 demo_mic() 函数进行麦克风音频采集
 * 和讯飞语音识别。通过 m_stopped 标志（volatile）实现线程的安全停止。
 *
 * 使用方式：
 * - 调用 start() 启动线程（自动调用 run()）
 * - 调用 stop() 设置停止标志
 * - 调用 wait() 等待线程完全退出
 */
class MicThread : public QThread {
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param session_begin_params 讯飞语音识别会话参数（sub/domain/language 等）
     * @param parent 父 QObject 指针
     */
    explicit MicThread(const char* session_begin_params, QObject* parent = nullptr);

    /**
     * @brief 请求停止录音线程
     *
     * 设置 m_stopped 为 true，demo_mic() 在下一次轮询时检测到并安全退出。
     * 该调用为异步请求，调用后应使用 wait() 等待线程实际结束。
     */
    void stop();

protected:
    /**
     * @brief 线程主函数
     *
     * 在独立线程中调用 demo_mic() 执行麦克风录音和语音识别。
     */
    void run() override;

private:
    const char* m_session_begin_params;     ///< 讯飞会话参数字符串（sub/domain/language 等键值对）
    volatile bool m_stopped;                ///< 线程停止标志（volatile 保证跨线程可见性）
};

/**
 * @class EDMIntelligentControlSystem
 * @brief EDM 参数智能调节系统主窗口
 *
 * 电火花加工参数智能调整系统的核心界面类，继承 QMainWindow，集成以下功能模块：
 * - 讯飞语音识别（麦克风录音 → 文字转写，支持用户热词）
 * - 豆包大模型 AI 对话（参数推荐，基于自然语言描述的智能参数优化）
 * - EDM 参数管理（20 个加工参数的增删改查与界面刷新）
 * - 参数持久化（CSV 文件导出 + Access 数据库存储，支持编号去重）
 *
 * UI 布局分为左右两栏：
 * - 左栏（聊天区）：ChatScrollWidget 对话气泡组件，显示用户提问和 AI 回答
 * - 右栏（参数区）：20 个参数标签 + 工艺信息下拉框/数值框 + 操作按钮
 */
class EDMIntelligentControlSystem : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     *
     * 执行以下初始化流程：
     * 1. 加载 Qt Designer UI 布局
     * 2. 登录讯飞 MSP 语音识别服务
     * 3. 创建 20 个 EDM 参数对象并赋初值
     * 4. 创建麦克风录音后台线程
     * 5. 上传用户自定义热词表
     * 6. 连接 UI 按钮信号与业务槽函数
     * 7. 刷新参数到界面
     *
     * @param parent 父控件指针
     */
    explicit EDMIntelligentControlSystem(QWidget *parent = Q_NULLPTR);

    /**
     * @brief 析构函数
     *
     * 清理资源：
     * - 停止并等待麦克风线程退出
     * - 释放所有 20 个 EDM 参数对象的内存
     * - 注销讯飞 MSP 会话
     * - 所有指针置空防止悬空引用
     */
    ~EDMIntelligentControlSystem();

signals:
    /**
     * @brief 语音识别完成信号
     *
     * 在录音状态变化（开始/结束）后触发，
     * 由 onRecordingFinished() 槽函数处理，将识别结果刷新到输入框。
     */
    void get_text_signal();

public slots:
    /** @brief 麦克风按钮切换（开始录音 ↔ 停止录音） */
    void onRadioButtonClicked();

    /** @brief 录音结束后将语音识别结果显示到输入框 */
    void onRecordingFinished();

    /** @brief 保存当前参数到 Access 数据库 */
    void onSaveAccess_clicked();

    /** @brief 获取输入框文本并以提问方身份添加到聊天区 */
    QString onGetText();

    /** @brief 向豆包 AI 发送问题，以回答方身份将 AI 回复添加到聊天区 */
    QString doubaoAnswer();

    /** @brief 在线优化：将 AI 解析的 EDM 参数值应用到参数对象并刷新界面 */
    bool onlineOptimization();

    /** @brief 离线优化（预留接口，尚未实现） */
    bool offlineOptimization();

    /** @brief 综合优化（预留接口，尚未实现） */
    bool comprehensiveOptimization();

    /** @brief 将当前参数以追加方式导出到 CSV 文件 */
    bool saveToFile(const QString& filePath);

    /** @brief 将当前参数保存到 Access 数据库（参数化查询，防 SQL 注入） */
    bool saveToAccess(const QString& dbFilePath);

    /** @brief 从 20 个参数对象读取值并刷新界面标签显示 */
    void refreshValues();

    /** @brief 将所有界面参数标签重置为默认值/零值 */
    void clearValues();

private:
    Ui::EDMIntelligentControlSystemClass ui;     ///< Qt Designer 自动生成的 UI 对象
    bool isRecording;               ///< 录音状态标志（true=录音中，false=已停止）
    MicThread* m_micThread;         ///< 麦克风录音后台线程指针（nullptr 表示初始化失败）
    QString inputText;              ///< 输入框文本缓存

    /* ========================= EDM 加工参数对象（20 个） ========================= */

    ON* on = nullptr;               ///< 脉冲放电时间（0~63，100~107）
    OFF* off = nullptr;             ///< 脉冲休止时间（0~63）
    IP* ip = nullptr;               ///< 放电电流峰值（0.5 的整数倍）
    PL* pl = nullptr;               ///< 放电极性（'+' 正极性 / '-' 负极性）
    V* v = nullptr;                 ///< 直流电压档位（01=90VDC，02=120VDC）
    HP* hp = nullptr;               ///< NOW 回路 / 高压辅助回路控制（两位数值）
    PP* pp = nullptr;               ///< PIKADEN 脉冲控制（"00"/"01"/"10"/"11"）
    AL* al = nullptr;               ///< 异常放电检验标准（0~63，基准值 33）
    OC* oc = nullptr;               ///< 预留参数 OC
    LD* ld = nullptr;               ///< 预留参数 LD
    MU* mu = nullptr;               ///< 脉冲幅度放大倍率（0~9，对应 ×1~×10）
    GAP* gap = nullptr;             ///< 伺服基准电压档位（0~9，对应 0V~130V）
    UP* up = nullptr;               ///< 自动抬刀抬升时间（0 无跳刀，1~9 启用）
    DN* dn = nullptr;               ///< 自动抬刀下降时间（与 UP 组合为一个抬刀周期）
    CA* ca = nullptr;               ///< 极间电容器容量（0~9，对应 0~1.4μF）
    S* s = nullptr;                 ///< 伺服速度（0~9，从快到慢）
    LN* ln = nullptr;               ///< 预留参数 LN
    STEP* step = nullptr;           ///< 摇动半径/放电间隔（0~99999 μm）
    L* l = nullptr;                 ///< 预留参数 L
    MyLP* mylp = nullptr;           ///< 预留参数 LP
};
