/**
 * @file    EDM_intelligent_control_system.cpp
 * @brief   主窗口业务逻辑实现
 *
 * 本文件实现了电火花加工参数智能调节系统的主窗口核心业务逻辑，包括：
 * - 讯飞语音识别（麦克风录音 → 文字转写）的完整流程
 * - 豆包大模型 AI 对话接口的调用与参数解析
 * - 20 个 EDM 加工参数的管理、刷新与在线优化
 * - 参数持久化（CSV 文件导出 / Access 数据库存储）
 *
 * 语音识别部分基于讯飞 MSC SDK，支持实时麦克风输入和离线音频文件输入。
 * AI 对话部分基于豆包（字节跳动火山引擎）的 OpenAI 兼容接口。
 */

#include "EDM_intelligent_control_system.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <process.h>

#include "msp_cmn.h"
#include "msp_errors.h"
#include "./include/speech_recognizer.h"
#include "doubaoapi.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#pragma execution_character_set("utf-8")

#ifdef _WIN64
#pragma comment(lib,"../libs/msc_x64.lib")
#else
#pragma comment(lib, "../libs/msc.lib")
#endif

/* ======================================================================== */
/*  宏定义 — 音频帧大小与缓冲区尺寸                                        */
/* ======================================================================== */

#define FRAME_LEN	640     ///< 单帧音频长度（16kHz/16bit 下为 640 字节，约 20ms）
#define	BUFFER_SIZE	4096    ///< 识别结果缓冲区初始大小

using namespace std;

/* ======================================================================== */
/*  全局对象 — 豆包 AI 接口实例                                             */
/* ======================================================================== */

DoubaoAI doubao;

/* ======================================================================== */
/*  语音识别 — 用户词表上传                                                */
/* ======================================================================== */

/**
 * @brief 上传自定义用户词表到讯飞语音识别服务
 * @return MSP_SUCCESS(0) 表示成功，其他值表示失败
 *
 * 从 userwords.txt 文件中读取用户自定义热词（如 EDM 参数中文名 → 英文缩写映射），
 * 通过 MSPUploadData 接口上传至讯飞云端，提升 EDM 专业术语的识别准确率。
 */
static int upload_userwords()
{
    char* userwords = NULL;
    size_t len = 0;
    size_t read_len = 0;
    FILE* fp = NULL;
    int ret = -1;

    fp = fopen("C:\\Users\\32284\\source\\repos\\new_record_stt\\EDM_intelligent_control_system\\userwords.txt", "rb");
    if (NULL == fp)
    {
        qWarning("打开用户词表文件 [userwords.txt] 失败");
        goto upload_exit;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    userwords = (char*)malloc(len + 1);
    if (NULL == userwords)
    {
        qCritical("内存不足，无法加载用户词表");
        goto upload_exit;
    }

    read_len = fread((void*)userwords, 1, len, fp);
    if (read_len != len)
    {
        qWarning("读取用户词表文件 [userwords.txt] 失败");
        goto upload_exit;
    }
    userwords[len] = '\0';

    MSPUploadData("userwords", userwords, len, "sub = uup, dtt = userword", &ret);
    if (MSP_SUCCESS != ret)
    {
        qWarning("上传用户词表失败，错误码: %d", ret);
        goto upload_exit;
    }

upload_exit:
    if (NULL != fp)
    {
        fclose(fp);
        fp = NULL;
    }
    if (NULL != userwords)
    {
        free(userwords);
        userwords = NULL;
    }

    return ret;
}

/* ======================================================================== */
/*  语音识别 — 全局结果缓冲与回调函数                                      */
/* ======================================================================== */

/** @brief 语音识别全局结果缓冲区指针 */
static char* g_result = NULL;

/** @brief 结果缓冲区当前分配大小 */
static unsigned int g_buffersize = BUFFER_SIZE;

/**
 * @brief 语音识别结果回调 — 接收中间/最终识别结果
 * @param result  本次识别的文本片段
 * @param is_last 是否为最终结果（非零表示识别结束）
 *
 * 由讯飞 SDK 在识别过程中多次回调。将每次识别出的文本片段
 * 追加到全局缓冲区 g_result 中。当缓冲区不足时自动扩容。
 */
void on_result(const char* result, char is_last)
{
    if (result) {
        size_t left = g_buffersize - 1 - strlen(g_result);
        size_t size = strlen(result);
        if (left < size) {
            g_result = (char*)realloc(g_result, g_buffersize + BUFFER_SIZE);
            if (g_result)
                g_buffersize += BUFFER_SIZE;
            else {
                qCritical("语音识别结果缓冲区内存分配失败");
            }
        }
        strncat(g_result, result, size);
    }
}

/**
 * @brief 语音识别开始回调 — 语音输入开始
 *
 * 由讯飞 SDK 在检测到用户开始说话时调用。
 * 重新初始化结果缓冲区以准备接收新的识别结果。
 */
void on_speech_begin()
{
    if (g_result)
    {
        free(g_result);
    }
    g_result = (char*)malloc(BUFFER_SIZE);
    g_buffersize = BUFFER_SIZE;
    memset(g_result, 0, g_buffersize);

    qDebug() << "语音识别开始监听...";
}

/**
 * @brief 语音识别结束回调 — 语音输入结束
 * @param reason 结束原因码（END_REASON_VAD_DETECT 表示 VAD 静音检测触发结束）
 *
 * 由讯飞 SDK 在检测到用户停止说话时调用。
 */
void on_speech_end(int reason)
{
    if (reason == END_REASON_VAD_DETECT)
        qDebug() << "语音识别完成（VAD 静音检测）";
    else
        qWarning() << "语音识别异常结束，原因码:" << reason;
}

/* ======================================================================== */
/*  语音识别 — 离线音频文件识别                                            */
/* ======================================================================== */

/**
 * @brief 从音频文件读取数据进行语音识别（离线模式）
 * @param audio_file            音频文件路径（PCM 格式，16kHz/16bit/单声道）
 * @param session_begin_params  讯飞会话参数字符串
 *
 * 以 200ms（10 帧）为单位分块读取音频文件，逐块发送至讯飞 SDK 进行识别。
 * 识别结果通过 on_result 等回调函数返回。
 *
 * @note 此函数为离线测试/调试用途，实际运行时优先使用麦克风输入（demo_mic）。
 */
static void demo_file(const char* audio_file, const char* session_begin_params)
{
    int errcode = 0;
    FILE* f_pcm = NULL;
    char* p_pcm = NULL;
    unsigned long pcm_count = 0;
    unsigned long pcm_size = 0;
    unsigned long read_size = 0;
    struct speech_rec iat;
    struct speech_rec_notifier recnotifier = {
        on_result,
        on_speech_begin,
        on_speech_end
    };

    if (NULL == audio_file)
        goto iat_exit;

    f_pcm = fopen(audio_file, "rb");
    if (NULL == f_pcm)
    {
        qWarning("打开音频文件 [%s] 失败", audio_file);
        goto iat_exit;
    }

    fseek(f_pcm, 0, SEEK_END);
    pcm_size = ftell(f_pcm);
    fseek(f_pcm, 0, SEEK_SET);

    p_pcm = (char*)malloc(pcm_size);
    if (NULL == p_pcm)
    {
        qCritical("内存不足，无法加载音频文件");
        goto iat_exit;
    }

    read_size = fread((void*)p_pcm, 1, pcm_size, f_pcm);
    if (read_size != pcm_size)
    {
        qWarning("读取音频文件 [%s] 失败", audio_file);
        goto iat_exit;
    }

    errcode = sr_init(&iat, session_begin_params, SR_USER, 0, &recnotifier);
    if (errcode) {
        qWarning("离线语音识别初始化失败，错误码: %d", errcode);
        goto iat_exit;
    }

    errcode = sr_start_listening(&iat);
    if (errcode) {
        qWarning("离线语音识别开始监听失败，错误码: %d", errcode);
        goto iat_exit;
    }

    while (1)
    {
        unsigned int len = 10 * FRAME_LEN;
        int ret = 0;

        if (pcm_size < 2 * len)
            len = pcm_size;
        if (len <= 0)
            break;

        ret = sr_write_audio_data(&iat, &p_pcm[pcm_count], len);

        if (0 != ret)
        {
            qWarning("写入音频数据失败，错误码: %d", ret);
            goto iat_exit;
        }

        pcm_count += (long)len;
        pcm_size -= (long)len;
    }

    errcode = sr_stop_listening(&iat);
    if (errcode) {
        qWarning("离线语音识别停止监听失败，错误码: %d", errcode);
        goto iat_exit;
    }

iat_exit:
    if (NULL != f_pcm)
    {
        fclose(f_pcm);
        f_pcm = NULL;
    }
    if (NULL != p_pcm)
    {
        free(p_pcm);
        p_pcm = NULL;
    }

    sr_stop_listening(&iat);
    sr_uninit(&iat);
}

/* ======================================================================== */
/*  语音识别 — 麦克风实时录音识别                                          */
/* ======================================================================== */

/**
 * @brief 从麦克风实时采集音频并进行语音识别
 * @param session_begin_params  讯飞会话参数字符串
 * @param stopped               外部停止标志指针（volatile，跨线程可见）
 *
 * 使用 Windows waveIn API 采集麦克风音频，实时送入讯飞 SDK 进行语音识别。
 * 该函数运行在 MicThread 后台线程中，通过 stopped 标志实现安全退出。
 *
 * @note 初始化完成后进入轮询等待循环，每 100ms 检查一次停止标志，
 *       以降低空转对 CPU 的占用。
 */
static void demo_mic(const char* session_begin_params, volatile bool* stopped)
{
    int errcode;
    struct speech_rec iat;

    struct speech_rec_notifier recnotifier = {
        on_result,
        on_speech_begin,
        on_speech_end
    };

    errcode = sr_init(&iat, session_begin_params, SR_MIC, DEFAULT_INPUT_DEVID, &recnotifier);
    if (errcode) {
        qWarning("麦克风语音识别初始化失败，错误码: %d", errcode);
        return;
    }

    errcode = sr_start_listening(&iat);
    if (errcode) {
        qWarning("麦克风语音识别开始监听失败，错误码: %d", errcode);
        sr_uninit(&iat);
        return;
    }

    /* 轮询等待停止信号，每 100ms 检查一次以避免 CPU 空转 */
    while (!*stopped) {
        QThread::msleep(100);
    }

    errcode = sr_stop_listening(&iat);
    if (errcode) {
        qWarning("麦克风语音识别停止监听失败，错误码: %d", errcode);
    }

    sr_uninit(&iat);

    qDebug() << "语音识别结果:" << QString::fromLocal8Bit(g_result);
}

/* ======================================================================== */
/*  MicThread — 麦克风后台线程实现                                         */
/* ======================================================================== */

/**
 * @brief MicThread 构造函数
 * @param session_begin_params  讯飞语音识别会话参数字符串
 * @param parent                父 QObject 指针
 *
 * 初始化线程对象，保存会话参数并设置停止标志为 false。
 */
MicThread::MicThread(const char* session_begin_params, QObject* parent)
    : QThread(parent), m_session_begin_params(session_begin_params), m_stopped(false)
{
}

/**
 * @brief 请求停止麦克风录音线程
 *
 * 设置 m_stopped 标志为 true，demo_mic() 函数将在下一次轮询时检测到并退出。
 * 调用后应使用 QThread::wait() 等待线程完全退出。
 */
void MicThread::stop()
{
    m_stopped = true;
}

/**
 * @brief 线程主函数
 *
 * 在独立线程中执行 demo_mic()，实现麦克风录音和语音识别的后台运行。
 */
void MicThread::run()
{
    demo_mic(m_session_begin_params, &m_stopped);
}

/* ======================================================================== */
/*  讯飞 — 会话参数（全局常量）                                            */
/* ======================================================================== */

/**
 * @brief 讯飞语音识别会话参数
 *
 * sub = iat            — 语音听写服务
 * domain = iat         — 听写领域
 * language = zh_cn     — 中文普通话
 * accent = mandarin    — 普通话口音
 * sample_rate = 16000  — 16kHz 采样率
 * result_type = plain  — 纯文本结果
 * result_encoding = gb2312 — GB2312 编码（兼容中文）
 */
const char* session_begin_params =
    "sub = iat, domain = iat, language = zh_cn, accent = mandarin, "
    "sample_rate = 16000, result_type = plain, result_encoding = gb2312";

/* ======================================================================== */
/*  EDMIntelligentControlSystem — 主窗口实现                                */
/* ======================================================================== */

/**
 * @brief 主窗口构造函数
 * @param parent 父控件指针
 *
 * 完成以下初始化工作：
 * - 调用 ui.setupUi() 加载 Qt Designer 界面
 * - 登录讯飞 MSP 语音识别服务
 * - 创建所有 EDM 加工参数对象并设置默认值
 * - 创建麦克风录音后台线程
 * - 上传用户自定义热词词表
 * - 连接界面信号与业务槽函数
 * - 调用 refreshValues() 将初始参数刷新到界面
 */
EDMIntelligentControlSystem::EDMIntelligentControlSystem(QWidget *parent) :
    QMainWindow(parent),
    m_micThread(nullptr),
    on(new ON(0)),
    off(new OFF(0)),
    ip(new IP(0.0)),
    pl(new PL('+')),
    v(new V(0)),
    hp(new HP(0)),
    pp(new PP("00")),
    al(new AL(33)),
    oc(new OC(0)),
    ld(new LD(0)),
    mu(new MU(0)),
    gap(new GAP(0)),
    up(new UP(0)),
    dn(new DN(0)),
    ca(new CA(0)),
    s(new S(0)),
    ln(new LN(0)),
    step(new STEP(0)),
    l(new L(0)),
    mylp(new MyLP(0))
{
    ui.setupUi(this);
    int ret = MSP_SUCCESS;
    const char* login_params = "appid = 8257763a, work_dir = .";
    isRecording = false;

    /* 登录讯飞 MSP 语音识别服务 */
    ret = MSPLogin(NULL, NULL, login_params);
    if (MSP_SUCCESS != ret) {
        qCritical("讯飞 MSP 登录失败，错误码: %d", ret);
        QMessageBox::critical(this, "错误", "语音识别初始化失败，请检查配置。");
        ui.radioButton->setEnabled(false);
    }
    else {
        m_micThread = new MicThread(session_begin_params, this);
    }

    /* 上传用户自定义热词表，提升 EDM 术语识别准确率 */
    qDebug() << "正在上传用户自定义热词表...";
    ret = upload_userwords();
    if (MSP_SUCCESS != ret) {
        qWarning("用户热词表上传失败，错误码: %d，将以默认词表运行", ret);
    }

    /* 将初始参数值刷新到界面显示 */
    refreshValues();

    /* 连接 UI 信号与业务逻辑槽函数 */
    connect(ui.m_affirm_pushButton, &QPushButton::clicked, this, &EDMIntelligentControlSystem::onGetText);
    connect(ui.radioButton, &QRadioButton::clicked, this, &EDMIntelligentControlSystem::onRadioButtonClicked);
    connect(this, &EDMIntelligentControlSystem::get_text_signal, this, &EDMIntelligentControlSystem::onRecordingFinished);
    connect(ui.m_affirm_pushButton, &QPushButton::clicked, this, &EDMIntelligentControlSystem::doubaoAnswer);
    connect(ui.m_Save_Parameters_Button, &QPushButton::clicked, this, &EDMIntelligentControlSystem::onSaveAccess_clicked);
    connect(ui.m_Online_Optimization_Button, &QPushButton::clicked, this, &EDMIntelligentControlSystem::onlineOptimization);
}

/**
 * @brief 将 EDM 参数对象的值刷新到界面标签
 *
 * 遍历所有 20 个 EDM 加工参数对象，通过各自的 getter 方法获取当前值，
 * 并写入对应的界面 QLabel 控件。其中 IP 参数保留两位小数显示。
 */
void EDMIntelligentControlSystem::refreshValues() {
    ui.m_ON_value->setText(QString::number(on->getON()));
    ui.m_OFF_value->setText(QString::number(off->getOFF()));
    ui.m_PL_value->setText(QString(pl->getPL()));
    ui.m_V_value->setText(QString::number(v->getV()));
    ui.m_HP_value->setText(QString::number(hp->getHP()));
    ui.m_PP_value->setText(QString::fromStdString(pp->getPP()));
    ui.m_AL_value->setText(QString::number(al->getAL()));
    ui.m_OC_value->setText(QString::number(oc->getOC()));
    ui.m_LD_value->setText(QString::number(ld->getLD()));
    ui.m_MU_value->setText(QString::number(mu->getMU()));
    ui.m_GAP_value->setText(QString::number(gap->getGAP()));
    ui.m_UP_value->setText(QString::number(up->getUP()));
    ui.m_DN_value->setText(QString::number(dn->getDN()));
    ui.m_CA_value->setText(QString::number(ca->getCA()));
    ui.m_S_value->setText(QString::number(s->getS()));
    ui.m_LN_value->setText(QString::number(ln->getLN()));
    ui.m_STEP_value->setText(QString::number(step->getSTEP()));
    ui.m_L_value->setText(QString::number(l->getL()));
    ui.m_MyLP_value->setText(QString::number(mylp->getLP()));
    ui.m_IP_value->setText(QString::number(ip->getIP(), 'f', 2));
}

/**
 * @brief 将所有界面参数值清零/恢复默认值
 *
 * 将界面显示的 20 个参数标签重置为默认值（0 或 "+"/"00"），
 * 通常在保存操作完成后调用，便于用户再次输入新的参数。
 *
 * @note 此函数仅修改界面显示，不修改参数对象的值。
 */
void EDMIntelligentControlSystem::clearValues() {
    ui.m_ON_value->setText(QString::number(0));
    ui.m_OFF_value->setText(QString::number(0));
    ui.m_PL_value->setText(QString("+"));
    ui.m_V_value->setText(QString::number(0));
    ui.m_HP_value->setText(QString::number(0));
    ui.m_PP_value->setText(QString("00"));
    ui.m_AL_value->setText(QString::number(0));
    ui.m_OC_value->setText(QString::number(0));
    ui.m_LD_value->setText(QString::number(0));
    ui.m_MU_value->setText(QString::number(0));
    ui.m_GAP_value->setText(QString::number(0));
    ui.m_UP_value->setText(QString::number(0));
    ui.m_DN_value->setText(QString::number(0));
    ui.m_CA_value->setText(QString::number(0));
    ui.m_S_value->setText(QString::number(0));
    ui.m_LN_value->setText(QString::number(0));
    ui.m_STEP_value->setText(QString::number(0));
    ui.m_L_value->setText(QString::number(0));
    ui.m_MyLP_value->setText(QString::number(0));
    ui.m_IP_value->setText(QString::number(0.00));
}

/**
 * @brief 录音结束后的处理：将识别结果显示到输入框
 *
 * 检查录音状态和全局结果缓冲区，将语音识别结果写入输入框。
 * - 若录音已结束且有识别结果：显示识别出的文本
 * - 若录音已结束但无识别结果：显示 "no g_result..." 提示
 * - 若录音仍在进行中：显示 "recording..." 提示
 */
void EDMIntelligentControlSystem::onRecordingFinished() {
    if (!isRecording) {
        if (g_result) {
            ui.m_input_textEdit->setText(QString::fromLocal8Bit(g_result));
        }
        else {
            ui.m_input_textEdit->setText(QString("未获取到识别结果"));
        }
    }
    else {
        ui.m_input_textEdit->setText(QString("录音中..."));
    }
}

/**
 * @brief 麦克风录音按钮点击槽函数
 *
 * 实现录音按钮的切换逻辑：
 * - 未录音状态 → 启动 MicThread 后台线程开始录音
 * - 正在录音状态 → 设置停止标志并等待线程退出
 *
 * 每次状态切换后均调用 onRecordingFinished() 更新界面输入框。
 */
void EDMIntelligentControlSystem::onRadioButtonClicked()
{
    if (!isRecording) {
        if (m_micThread == nullptr) {
            qWarning("无法启动麦克风线程：MSP 初始化失败");
            return;
        }
        isRecording = true;
        qDebug() << "启动麦克风录音线程...";
        m_micThread->start();
    }
    else {
        isRecording = false;
        qDebug() << "停止麦克风录音线程...";
        m_micThread->stop();
        m_micThread->wait();
    }
    onRecordingFinished();
}

/**
 * @brief 保存参数到 Access 数据库按钮点击槽函数
 *
 * 将当前所有 EDM 参数及加工信息保存到 Access 数据库文件。
 * 保存成功后调用 clearValues() 清空界面参数值。
 */
void EDMIntelligentControlSystem::onSaveAccess_clicked() {
    QString filePath = QString("C:\\Users\\32284\\source\\repos\\EDM_intelligent_control_system\\EDM_intelligent_control_system\\DataBase\\electric.accdb");
    if (!filePath.isEmpty()) {
        saveToAccess(filePath);
    }
    clearValues();
}

/**
 * @brief 获取输入框文本并添加到聊天区域（用户提问方）
 * @return 输入框的文本内容
 *
 * 从输入框获取用户输入的文本，以 ChatType::Question 类型添加到聊天显示区域。
 * 由确认按钮点击信号触发。
 */
QString EDMIntelligentControlSystem::onGetText()
{
    QString text = ui.m_input_textEdit->toPlainText();

    if (!text.isEmpty()) {
        ui.chatScrollWidget->addChatContent(text, ChatType::Question);
    }
    return text;
}

/**
 * @brief 向豆包 AI 发送问题并显示回答
 * @return AI 返回的内容文本
 *
 * 工作流程：
 * 1. 从输入框获取用户文本并清空输入框
 * 2. 构造 EDM 参数专家系统提示词（定义 13 个核心参数的含义和取值范围）
 * 3. 调用 doubao.DoubaoAI_request() 发送请求
 * 4. 将 AI 回答以 ChatType::Answer 类型添加到聊天显示区域
 *
 * 系统提示词中详细说明了每个参数的含义、取值范围和单位要求，
 * 引导 AI 以 Markdown 表格形式返回参数推荐结果。
 */
QString EDMIntelligentControlSystem::doubaoAnswer() {
    QString text = ui.m_input_textEdit->toPlainText();
    ui.m_input_textEdit->clear();

    /* EDM 参数专家系统提示词 — 定义 AI 角色、参数含义及输出格式 */
    QString m_customPrompt = "你是电火花参数加工的专家，我需要你根据我的问题，你只能生成唯一的一个最优的参数组合到一个表格中，表格的参数名列只能写大写字母，设定值列只能是值，不需要单位，参数包括( \
        ON:设定1个脉冲的放电时间（设定范围为 0 - 63；100 - 107）; \
        OFF:OFF：设定１个脉冲放电后的休止时间（设定范围 0 - 63）; \
        IP:设定１个脉冲的放电电流的峰值（Peak），与放电时间 ON组合，是决定加工速度、表面粗糙度、电极消耗以及放电加工性能的重要参数。不同电源型号的 IP 最高输入值也不同（最小输入单位为 0.5, IP参数的值为0.5的整数倍）。 \
        PL : 选择电极，工件的放电极性，以主轴一侧（通常情况为电极）为基准输入＋或－, 正极性加工：工件为＋；电极为－；负极性加工：工件为－；电极为＋。; \
        V：用于转换提供 IP 电流的直流电压，01 对应 Cu - ST 加工（90VDC），02 对应 Gr - ST / CuW - W 加工（120VDC）。;\
        HP：用于控制降低电极消耗的 NOW 回路、电压控制回路（低 / 中压）和高压辅助回路（ON / OFF 及电流追加）。设定值规则：两位数值，十位决定电压回路和 NOW 回路状态（如00 = 低压 + NOW OFF、40 = 低压 + NOW ON、10 = 中压 + NOW OFF 等）， 个位（0~7）控制高压辅助电流（0 = 无电流，1~7依次对应0.5A~3.5A，每档递增0.5A）。电压基准：无负荷电压基准值由十位决定，可选90V / 120V / 150V / 280V \
        PP:设定 PIKADEN 脉冲控制，设定值：00 代表高压 + PIKADEN OFF，01 代表仅PIKADEN ON，10 代表仅高压 ON，11 代表高压 + PIKADEN ON \
        AL：异常放电检验标准设定, 设定范围为 0 - 63（基准值为 33）。设定值越大，加工速度越快，耐电弧性也随之降低。\
        MU：用于设定单次脉冲放电后休止时间 OFF 阶段的脉冲幅度放大倍率，其设定值 0~9 分别对应 ×1 到 ×10 的放大倍率。 \
        GAP：用于设定伺服基准电压，其设定值 0~9 分别对应 0V、15V、25V、35V、45V、60V、70V、80V、120V、130V 的加工电压（实际值随加工条件略有差异）。 \
        UP：控制自动抬刀的抬升时间, 设定为0时无跳刀动作；设定为1~9时，配合 DN 参数启用自动抬刀。 \
        DN：控制自动抬刀的下降时间, 与 UP 组成一个抬刀周期，仅当 UP 也设为1~9时生效；粗 / 精加工需按DN = UP + 固定差值的规则设置。\
        CA ：用于设定极间电容器的容量，设定值 0~9 分别对应 0、0.006、0.012、0.025、0.05、0.1、0.2、0.4、0.8、1.4μF 的容量，适用于有消耗条件下的精加工、细孔加工及电极成形加工\
        S：用于设定伺服速度（设定范围 0 - 9），设定值 0 - 9 对应伺服速度从快到慢，通常设 2 或 3，在细孔、电极成形等加工时需调高设定值以避免伺服轴振动。) \
        STEP：用于设定摇动的摇动半径（放电间隔），设定范围为 0~99999，单位为 μm。";

    QString answer = doubao.DoubaoAI_request(text, m_customPrompt);
    ui.chatScrollWidget->addChatContent(answer, ChatType::Answer);
    return answer;
}

/**
 * @brief 在线优化：将 AI 解析得到的参数值应用到对应对象并刷新界面
 * @return 始终返回 true
 *
 * 从 DoubaoAI 的参数解析结果 (getParamResult()) 中读取每个参数的值，
 * 通过参数名匹配找到对应的参数对象并调用 setter 方法更新值，
 * 最后调用 refreshValues() 刷新界面显示。
 *
 * 支持的参数名包括：ON、OFF、IP、PL、V、HP、PP、AL、MU、GAP、
 * UP、DN、CA、S、STEP，总计 14 个可在线优化的参数。
 */
bool EDMIntelligentControlSystem::onlineOptimization() {
    QMap<QString, QString> map = doubao.getParamResult();
    for (const QString& key : map.keys())
    {
        QString value = map.value(key);
        if (key == "ON")
        {
            on->setON(value.toInt());
        }
        else if (key == "OFF")
        {
            off->setOFF(value.toInt());
        }
        else if (key == "IP")
        {
            ip->setIP(value.toDouble());
        }
        else if (key == "PL")
        {
            pl->setPL(value.at(0).toLatin1());
        }
        else if (key == "V")
        {
            v->setV(value.toInt());
        }
        else if (key == "HP")
        {
            hp->setHP(value.toInt());
        }
        else if (key == "PP")
        {
            pp->setPP(value.toStdString());
        }
        else if (key == "AL")
        {
            al->setAL(value.toInt());
        }
        else if (key == "MU")
        {
            mu->setMU(value.toInt());
        }
        else if (key == "GAP")
        {
            gap->setGAP(value.toInt());
        }
        else if (key == "UP")
        {
            up->setUP(value.toInt());
        }
        else if (key == "DN")
        {
            dn->setDN(value.toInt());
        }
        else if (key == "CA")
        {
            ca->setCA(value.toInt());
        }
        else if (key == "S")
        {
            s->setS(value.toInt());
        }
        else if (key == "STEP")
        {
            step->setSTEP(value.toInt());
        }
    }
    refreshValues();
    return true;
}

/**
 * @brief 离线优化（预留接口，尚未实现）
 * @return 始终返回 true
 *
 * 预留给未来离线优化功能的接口，目前为空实现。
 */
bool EDMIntelligentControlSystem::offlineOptimization() {
    return true;
}

/**
 * @brief 综合优化（预留接口，尚未实现）
 * @return 始终返回 true
 *
 * 预留给未来综合优化（同时包含在线和离线策略的优化）功能的接口，目前为空实现。
 */
bool EDMIntelligentControlSystem::comprehensiveOptimization() {
    return true;
}

/**
 * @brief 将当前加工参数导出到 CSV 文件
 * @param filePath 导出 CSV 文件的完整路径
 * @return true 表示导出成功，false 表示导出失败
 *
 * 导出内容包括：
 * - 20 个 EDM 加工参数值
 * - 电极材料、工件材料、投影面积
 * - 加工速度、表面粗糙度、电极消耗比、加工间隙
 *
 * 若文件为新文件（大小为 0），则先写入 CSV 列头行，
 * 否则在文件末尾追加一行参数值。
 */
bool EDMIntelligentControlSystem::saveToFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件：" + filePath);
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    /* 构建参数列表（参数名、参数值） */
    QList<QPair<QString, QString>> paramList;
    paramList << QPair<QString, QString>("ON", QString::number(on->getON()));
    paramList << QPair<QString, QString>("OFF", QString::number(off->getOFF()));
    paramList << QPair<QString, QString>("IP", QString::number(ip->getIP(), 'f', 2));
    paramList << QPair<QString, QString>("PL", QString(pl->getPL()));
    paramList << QPair<QString, QString>("V", QString::number(v->getV()));
    paramList << QPair<QString, QString>("HP", QString::number(hp->getHP()));
    paramList << QPair<QString, QString>("PP", QString::fromStdString(pp->getPP()));
    paramList << QPair<QString, QString>("AL", QString::number(al->getAL()));
    paramList << QPair<QString, QString>("OC", QString::number(oc->getOC()));
    paramList << QPair<QString, QString>("LD", QString::number(ld->getLD()));
    paramList << QPair<QString, QString>("MU", QString::number(mu->getMU()));
    paramList << QPair<QString, QString>("GAP", QString::number(gap->getGAP()));
    paramList << QPair<QString, QString>("UP", QString::number(up->getUP()));
    paramList << QPair<QString, QString>("DN", QString::number(dn->getDN()));
    paramList << QPair<QString, QString>("CA", QString::number(ca->getCA()));
    paramList << QPair<QString, QString>("S", QString::number(s->getS()));
    paramList << QPair<QString, QString>("LN", QString::number(ln->getLN()));
    paramList << QPair<QString, QString>("STEP", QString::number(step->getSTEP()));
    paramList << QPair<QString, QString>("L", QString::number(l->getL()));
    paramList << QPair<QString, QString>("MyLP", QString::number(mylp->getLP()));

    /* 添加界面中的工艺参数 */
    QString electrodeMaterial = ui.m_Electrode_material->currentText();
    paramList << QPair<QString, QString>(QString("电极材料"), electrodeMaterial);
    QString workpieceMaterial = ui.m_Workpiece_Material->currentText();
    paramList << QPair<QString, QString>(QString("工件材料"), workpieceMaterial);
    QString projectedArea = ui.m_Projected_Area->currentText();
    paramList << QPair<QString, QString>(QString("投影面积"), projectedArea);
    paramList << QPair<QString, QString>(QString("加工速度(mm³/min)"), QString::number(ui.m_Processing_Speed->value()));
    paramList << QPair<QString, QString>(QString("表面粗糙度(μRmax)"), QString::number(ui.m_Surface_Roughness->value()));
    paramList << QPair<QString, QString>(QString("电极消耗比(E/W×100%)"), QString::number(ui.m_Electrode_Consumption_Radio->value()));
    paramList << QPair<QString, QString>(QString("\"加工间隙(μm,β)\""), QString::number(ui.m_Machining_Allowance->value()));

    /* 新文件先写入 CSV 列头 */
    bool isNewFile = (file.size() == 0);
    if (isNewFile) {
        QString headerLine;
        for (int i = 0; i < paramList.size(); ++i) {
            headerLine += paramList[i].first;
            if (i != paramList.size() - 1) {
                headerLine += ",";
            }
        }
        out << headerLine << "\n";
    }

    /* 写入参数值行 */
    QString valueLine;
    for (int i = 0; i < paramList.size(); ++i) {
        valueLine += paramList[i].second;
        if (i != paramList.size() - 1) {
            valueLine += ",";
        }
    }
    out << valueLine << "\n";

    file.close();

    QMessageBox::information(this, "成功", "参数已追加保存到：" + filePath);
    return true;
}

/**
 * @brief 将当前加工参数保存到 Microsoft Access 数据库
 * @param dbFilePath Access 数据库文件路径（.accdb 格式）
 * @return true 表示保存成功，false 表示保存失败
 *
 * 保存逻辑：
 * - 若数据表 "加工参数表" 不存在，自动创建包含所有参数列的数据表
 * - 若数据表已存在，检查当前加工编号是否重复（编号唯一约束）
 * - 将 20 个 EDM 参数 + 工艺信息通过参数化 SQL 插入数据表
 *
 * @note 使用 QODBC 驱动连接 Access 数据库，需要系统安装 Microsoft Access ODBC 驱动。
 *       参数化查询可有效防止 SQL 注入。
 */
bool EDMIntelligentControlSystem::saveToAccess(const QString& dbFilePath)
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else {
        db = QSqlDatabase::addDatabase("QODBC");
    }

    /* 构造 ODBC 连接字符串 */
    QString connStr = QString(
        "DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};"
        "DBQ=%1;"
        "UID=;"
        "PWD=;"
    ).arg(dbFilePath);

    db.setDatabaseName(connStr);

    if (!db.open()) {
        QMessageBox::critical(this, "错误", QString("连接数据库失败：\n%1").arg(db.lastError().text()));
        return false;
    }

    QSqlQuery query(db);
    bool tableExists = false;

    QString tableName = "加工参数表";
    QString eNumber = QString("E%1").arg(ui.m_Processing_Number->value());

    QStringList allTables = db.tables();
    if (allTables.contains(tableName, Qt::CaseInsensitive))
    {
        tableExists = true;
    }

    if (!tableExists) {
        /* 数据表不存在，创建包含所有参数列的新表 */
        QString createTableSql = QString(
            "CREATE TABLE %1 ("
            "ID AUTOINCREMENT PRIMARY KEY,"
            "编号 TEXT,"
            "[ON] INT,"
            "[OFF] INT,"
            "IP DOUBLE,"
            "PL TEXT,"
            "V INT,"
            "HP INT,"
            "PP TEXT,"
            "AL INT,"
            "OC INT,"
            "LD INT,"
            "MU INT,"
            "GAP INT,"
            "UP INT,"
            "DN INT,"
            "CA INT,"
            "S INT,"
            "LN TEXT,"
            "STEP INT,"
            "L TEXT,"
            "LP TEXT,"
            "电极材料 TEXT,"
            "工件材料 TEXT,"
            "投影面积 INT,"
            "加工速度 DOUBLE,"
            "表面粗糙度 INT,"
            "电极消耗比 DOUBLE,"
            "加工间隙 INT"
            ");"
        ).arg(tableName);

        if (!query.exec(createTableSql)) {
            QMessageBox::critical(this, "建表失败", query.lastError().text());
            db.close();
            return false;
        }
    }
    else {
        /* 数据表已存在，检查编号是否重复 */
        QString checkSql = QString("SELECT COUNT(*) FROM %1 WHERE 编号 = ?").arg(tableName);
        query.prepare(checkSql);
        query.addBindValue(eNumber);
        query.exec();

        if (query.next() && query.value(0).toInt() > 0) {
            QMessageBox::warning(this, "提示", QString("编号 [%1] 已存在，请重新修改编号！").arg(eNumber));
            db.close();
            return false;
        }
    }

    /* 获取各参数当前值 */
    int ON = on->getON();
    int OFF = off->getOFF();
    double IP = ip->getIP();
    QString PL = QChar(pl->getPL());
    int V = v->getV();
    int HP = hp->getHP();
    QString PP = QString::fromStdString(pp->getPP());
    int AL = al->getAL();
    int OC = oc->getOC();
    int LD = ld->getLD();
    int MU = mu->getMU();
    int GAP = gap->getGAP();
    int UP = up->getUP();
    int DN = dn->getDN();
    int CA = ca->getCA();
    int S = s->getS();
    QString LN = QString::number(ln->getLN());
    int STEP = step->getSTEP();
    QString L = QString::number(l->getL());
    QString LP = QString::number(mylp->getLP());

    QString electrodeMaterial = ui.m_Electrode_material->currentText();
    QString workpieceMaterial = ui.m_Workpiece_Material->currentText();
    QString projectedArea = ui.m_Projected_Area->currentText();
    double processingSpeed = ui.m_Processing_Speed->value();
    int surfaceRoughness = ui.m_Surface_Roughness->value();
    double electrodeConsumptionRatio = ui.m_Electrode_Consumption_Radio->value();
    int machiningGap = ui.m_Machining_Allowance->value();

    /* 执行参数化 INSERT 语句 */
    QString insertSql = QString(
        "INSERT INTO %1 ("
        "编号,[ON],[OFF],IP,PL,V,HP,PP,AL,OC,LD,MU,GAP,UP,DN,CA,S,LN,STEP,L,LP,"
        "电极材料,工件材料,投影面积,加工速度,表面粗糙度,电极消耗比,加工间隙"
        ") VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"
    ).arg(tableName);

    query.prepare(insertSql);
    query.addBindValue(eNumber);
    query.addBindValue(ON);
    query.addBindValue(OFF);
    query.addBindValue(IP);
    query.addBindValue(PL);
    query.addBindValue(V);
    query.addBindValue(HP);
    query.addBindValue(PP);
    query.addBindValue(AL);
    query.addBindValue(OC);
    query.addBindValue(LD);
    query.addBindValue(MU);
    query.addBindValue(GAP);
    query.addBindValue(UP);
    query.addBindValue(DN);
    query.addBindValue(CA);
    query.addBindValue(S);
    query.addBindValue(LN);
    query.addBindValue(STEP);
    query.addBindValue(L);
    query.addBindValue(LP);
    query.addBindValue(electrodeMaterial);
    query.addBindValue(workpieceMaterial);
    query.addBindValue(projectedArea);
    query.addBindValue(processingSpeed);
    query.addBindValue(surfaceRoughness);
    query.addBindValue(electrodeConsumptionRatio);
    query.addBindValue(machiningGap);

    if (!query.exec()) {
        QMessageBox::critical(this, "保存失败", query.lastError().text());
        db.close();
        return false;
    }

    db.close();

    if (!tableExists) {
        QMessageBox::information(this, "成功", "表不存在，已新建表并保存数据！");
    }
    else {
        QMessageBox::information(this, "成功", "编号有效，数据已追加到表中！");
    }

    return true;
}

/**
 * @brief 主窗口析构函数
 *
 * 执行清理工作：
 * - 停止并等待麦克风线程退出
 * - 释放所有 EDM 参数对象（通过 delete）
 * - 注销讯飞 MSP 会话
 * - 将所有指针置空防止悬空指针
 */
EDMIntelligentControlSystem::~EDMIntelligentControlSystem()
{
    if (m_micThread->isRunning()) {
        m_micThread->stop();
        m_micThread->wait();
    }
    delete m_micThread;
    delete on;
    delete off;
    delete ip;
    delete pl;
    delete v;
    delete hp;
    delete pp;
    delete al;
    delete oc;
    delete ld;
    delete mu;
    delete gap;
    delete up;
    delete dn;
    delete ca;
    delete s;
    delete ln;
    delete step;
    delete l;
    delete mylp;

    m_micThread = nullptr;
    on = nullptr;
    off = nullptr;
    ip = nullptr;
    pl = nullptr;
    v = nullptr;
    hp = nullptr;
    pp = nullptr;
    al = nullptr;
    oc = nullptr;
    ld = nullptr;
    mu = nullptr;
    gap = nullptr;
    up = nullptr;
    dn = nullptr;
    ca = nullptr;
    s = nullptr;
    ln = nullptr;
    step = nullptr;
    l = nullptr;
    mylp = nullptr;

    MSPLogout();
}
