/**
 * @file    doubao.cpp
 * @brief   豆包（Doubao）AI 接口实现
 *
 * 实现了 DoubaoAI 类的全部方法，包括：
 * - 通过 QNetworkAccessManager 发送 OpenAI 兼容格式的 HTTPS 请求
 * - 使用 QRegularExpression 从 AI 返回的 Markdown 表格中提取 EDM 参数值
 * - 通过 QEventLoop 实现同步等待网络响应
 *
 * 参数解析支持 20 个 EDM 参数（见 PARAM_LIST），
 * 可从多种分隔符格式的表格行中提取数值。
 */

#include "doubaoapi.h"
#include <QRegularExpression>
#include <QMap>

/**
 * @brief 构造函数，初始化参数结果缓存
 * @param parent 父 QObject 指针
 */
DoubaoAI::DoubaoAI(QObject* parent) : QObject(parent)
{
    m_paramResult.clear();
}

/**
 * @brief EDM 参数名列表（20 个），用于正则匹配遍历和解析结果维护
 *
 * 此列表定义了所有 EDM 参数的名称，索引顺序与 ecommand.h 中的声明顺序一致。
 * 正则解析时按此顺序遍历匹配，确保解析结果中参数顺序的确定性。
 */
static const QStringList PARAM_LIST = {
    "ON", "OFF", "IP", "PL", "V", "HP", "PP", "AL",
    "OC", "LD", "MU", "GAP", "UP", "DN", "CA", "S",
    "LN", "STEP", "L", "LP"
};

/**
 * @brief 从 AI 返回的文本内容中解析 EDM 参数键值对
 * @param content AI 返回的原始文本内容（通常包含 Markdown 表格）
 * @return 参数名 → 参数值的 QMap，未匹配到的参数值为空字符串 ""
 *
 * 解析算法：
 * - 遍历 PARAM_LIST 中的每个参数名
 * - 对每个参数名构造正则：参数名后跟分隔符（[:：=|]）和数值
 * - 从 content 中执行不区分大小写的正则匹配
 * - 匹配成功：提取捕获组 2（数值部分）
 * - 匹配失败：对应参数值设为空字符串
 *
 * 支持的输入表格格式示例：
 * ```
 * | ON | 30 |
 * | OFF: 20
 * IP：15.5
 * HP=42
 * ```
 */
QMap<QString, QString> DoubaoAI::parseParams(const QString& content) {
    QMap<QString, QString> paramMap;

    foreach(const QString & param, PARAM_LIST) {
        /* 正则模式：参数名出现在行首、空格后或竖线后，后跟分隔符和数值 */
        QString pattern = QString(R"((?:^|\s|[|])(%1)\s*[:：=|]\s*([^\s|,;]+))").arg(param);
        QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = regex.match(content);

        if (match.hasMatch()) {
            paramMap[param] = match.captured(2);
        }
        else {
            paramMap[param] = "";
        }
    }
    return paramMap;
}

/**
 * @brief 向豆包大模型 API 发送请求并获取回答
 * @param question     用户输入的问题文本（如"不锈钢精加工参数"）
 * @param customPrompt 系统提示词（定义 AI 的 EDM 专家角色和输出格式约束）
 * @return AI 返回的文本内容，若请求失败则返回空字符串
 *
 * 详细流程：
 * 1. 构造 QNetworkRequest 并设置 URL 和认证头
 * 2. 构建 messages 数组（system 消息 + user 消息）
 * 3. 组装完整的请求 JSON（model + messages）
 * 4. 配置 SSL 为 AnyProtocol + VerifyNone（适用于 API 网关/反向代理环境）
 * 5. 发送 POST 请求，通过 QEventLoop 同步等待 finished 信号
 * 6. 检查响应状态，解析 JSON 中 choices[0].message.content
 * 7. 调用 parseParams() 从 AI 回答中提取 EDM 参数键值对
 * 8. 将解析结果输出到日志（qDebug）以供追踪
 *
 * @note 使用 QEventLoop 实现同步等待，调用此函数会阻塞当前线程直到网络请求完成。
 *       若需避免 UI 卡顿，应将此调用放在工作线程中执行。
 */
QString DoubaoAI::DoubaoAI_request(QString& question, const QString& customPrompt)
{
    QNetworkRequest request;
    QNetworkAccessManager manager;
    request.setUrl(QUrl(OPENAI_BASE_URL));

    /* 设置 Bearer Token 认证头 */
    QString token = "Bearer " + OPENAI_API_KEY;
    request.setRawHeader("Authorization", token.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    /* 构造 Chat Completions 请求的消息数组 */
    QJsonArray messagesarray;

    /* 系统提示词：定义 AI 的 EDM 专家角色和输出规范 */
    if (!customPrompt.isEmpty()) {
        QJsonObject systemMsg;
        systemMsg["role"] = "system";
        systemMsg["content"] = customPrompt;
        messagesarray.append(systemMsg);
    }

    /* 用户问题：具体的加工参数请求 */
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = question;
    messagesarray.append(userMsg);

    /* 组装完整请求体 */
    QJsonObject data;
    data["model"] = OPENAI_MODEL;
    data["messages"] = messagesarray;

    QJsonDocument doc(data);
    QByteArray postData = doc.toJson();

    /* 配置 SSL：允许任意协议版本，跳过证书验证（适用于 API 网关环境） */
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::AnyProtocol);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(config);

    /* 发送 POST 请求并通过事件循环同步等待响应 */
    QNetworkReply* reply = manager.post(request, postData);
    QEventLoop waitserver;
    connect(reply, &QNetworkReply::finished, &waitserver, &QEventLoop::quit);
    waitserver.exec();

    QString aiReplyContent;
    m_paramResult.clear();

    /* 处理 HTTP 响应 */
    if (reply != nullptr && reply->error() == QNetworkReply::NoError) {
        QByteArray reply_data = reply->readAll();
        QJsonObject obj = QJsonDocument::fromJson(reply_data).object();
        QJsonArray choicesarray = obj.value("choices").toArray();

        if (!choicesarray.isEmpty()) {
            QJsonObject choiceobj = choicesarray[0].toObject();
            if (choiceobj.contains("message") && choiceobj["message"].isObject()) {
                QJsonObject messageobj = choiceobj["message"].toObject();
                if (messageobj.contains("content") && messageobj["content"].isString()) {
                    aiReplyContent = messageobj["content"].toString();
                    m_paramResult = parseParams(aiReplyContent);

                    /* 输出解析结果用于日志追踪 */
                    qDebug() << "===== AI 表格参数解析结果 =====";
                    foreach(const QString & param, PARAM_LIST) {
                        if (!m_paramResult[param].isEmpty()) {
                            qDebug() << param << "=" << m_paramResult[param];
                        }
                    }
                }
            }
        }
    }
    else {
        qWarning() << "豆包 API 请求失败：" << (reply ? reply->errorString() : "回复为空");
    }

    if (reply) {
        reply->deleteLater();
    }

    return aiReplyContent;
}

/**
 * @brief 获取最近一次请求解析得到的参数结果
 * @return 参数名 → 参数值的 QMap 映射表
 *
 * 返回的是最后一次 DoubaoAI_request() 调用中 parseParams() 的解析结果。
 * 调用者（onlineOptimization）通过此方法获取解析后的参数键值对，
 * 然后应用到对应的 EDM 参数对象。
 */
QMap<QString, QString> DoubaoAI::getParamResult() const
{
    return m_paramResult;
}
