/**
 * @file    doubaoapi.h
 * @brief   豆包（Doubao）AI 接口声明
 *
 * 本文件声明了 DoubaoAI 类，用于调用字节跳动火山引擎豆包大模型
 * 的 OpenAI 兼容 Chat Completions API 接口，实现电火花加工参数的智能推荐。
 *
 * 工作流程：
 * 1. 接收用户输入的问题和 EDM 参数专家系统提示词
 * 2. 构造 OpenAI 兼容的 JSON 请求体（system + user 消息）
 * 3. 通过 HTTPS POST 发送至火山引擎 Ark API
 * 4. 解析响应 JSON 获取 AI 回答文本
 * 5. 使用正则表达式从回答中提取参数名→参数值的映射
 *
 * 使用的模型：doubao-seed-1-8（豆包 Seed 1.8 版本）
 *
 * @note API 采用 OpenAI Chat Completions 格式，可使用任何兼容该格式的端点。
 */

#ifndef DOUBAOAPI_H
#define DOUBAOAPI_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QEventLoop>
#include <QMap>

/**
 * @class DoubaoAI
 * @brief 豆包大模型 API 封装类
 *
 * 通过 HTTP POST 请求调用火山引擎 Ark API 的 Chat Completions 接口，
 * 支持同步方式发送用户问题及系统提示词，获取 AI 生成的 EDM 参数推荐，
 * 并从返回的 Markdown 表格中解析参数值。
 *
 * 使用 QEventLoop 实现同步等待，适用于 Qt 主线程中需要等待 API 响应的场景。
 *
 * @warning API 密钥为硬编码方式，生产环境中应使用安全存储方案。
 */
class DoubaoAI : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父 QObject 指针
     *
     * 初始化参数结果缓存（清空 m_paramResult）。
     */
    explicit DoubaoAI(QObject* parent = nullptr);

    /**
     * @brief 向豆包 API 发送请求并获取 AI 回答
     * @param question     用户输入的问题文本
     * @param customPrompt 系统提示词（定义 AI 角色、EDM 参数含义和输出格式）
     * @return AI 返回的文本内容（请求失败时返回空字符串）
     *
     * 内部完整流程：
     * 1. 构造 OpenAI 兼容的 JSON 请求体
     * 2. 设置认证头（Bearer Token）
     * 3. 配置 SSL 参数（跳过证书验证，适用于 API 网关环境）
     * 4. 通过 QEventLoop 同步等待网络响应
     * 5. 解析响应 JSON 获取 AI 回答文本
     * 6. 自动调用 parseParams() 从回答中提取 EDM 参数表格
     */
    QString DoubaoAI_request(QString& question, const QString& customPrompt);

    /**
     * @brief 从 AI 返回的表格文本中解析 EDM 参数
     * @param content AI 返回的原始文本内容（通常为 Markdown 表格格式）
     * @return 参数名 → 参数值的 QMap 映射表（20 个参数，未匹配到的值为空字符串）
     *
     * 正则匹配规则：查找 "参数名 分隔符 数值" 模式的行，
     * 支持以下分隔符：英文冒号(:)、中文冒号(：)、等号(=)、竖线(|)。
     *
     * 解析结果按 PARAM_LIST 的顺序遍历，保证输出顺序一致。
     */
    QMap<QString, QString> parseParams(const QString& content);

    /**
     * @brief 获取最近一次请求解析得到的参数结果
     * @return 参数名 → 参数值的映射表
     *
     * 一般在 DoubaoAI_request() 调用后使用，
     * 由 onlineOptimization() 消费此结果来更新 EDM 参数值。
     */
    QMap<QString, QString> getParamResult() const;

private:
    QString OPENAI_BASE_URL = "https://ark.cn-beijing.volces.com/api/v3/chat/completions";   ///< 火山引擎 Ark API 基础地址
    QString OPENAI_API_KEY = "2390cff3-ed6f-43a2-b1c7-52d42785e880";                         ///< API 访问密钥（生产环境应考虑外置）
    QString OPENAI_MODEL   = "doubao-seed-1-8-251228";                                       ///< 豆包大模型版本标识
    QMap<QString, QString> m_paramResult;                                                     ///< 最近一次参数解析结果缓存
};

#endif // DOUBAOAPI_H
