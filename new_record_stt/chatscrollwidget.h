#pragma once
#ifndef CHATSCROLLWIDGET_H
#define CHATSCROLLWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QMargins>
#include <QScrollBar>
// 新增：MD解析所需头文件
#include <QRegularExpression>
#include <QStringList>

// 对话类型枚举：问题/回答
enum class ChatType {
    Question,  // 用户问题
    Answer     // 模型回答
};

class ChatScrollWidget : public QWidget
{
    Q_OBJECT
public:
    // 构造函数，parent为父控件
    explicit ChatScrollWidget(QWidget* parent = nullptr);

    /**
     * @brief 动态添加对话内容
     * @param text 对话文字内容
     * @param type 对话类型：问题/回答
     */
    void addChatContent(const QString& text, ChatType type);

    /**
     * @brief 清空所有历史对话
     */
    void clearAllChat();

private:
    // 滚动窗口核心控件
    QScrollArea* m_scrollArea;    // 外层滚动区域
    QWidget* m_contentWidget; // 滚动区域的主内容容器
    QVBoxLayout* m_mainLayout;    // 主内容容器的垂直布局（所有问答Widget都加在这里）

    /**
     * @brief 创建单个问答Widget（核心：自适应高度+样式区分）
     * @param text 文字内容
     * @param type 对话类型
     * @return 自适应高度的问答Widget
     */
    QWidget* createChatWidget(const QString& text, ChatType type);

    /**
     * @brief 计算文本在QTextEdit中渲染的真实高度（自适应核心）
     * @param text 文字内容
     * @param textEdit 用于计算的QTextEdit（复用样式）
     * @return 文本真实渲染高度（像素）
     */
     // 新增isHtml参数，适配HTML高度计算
    int calculateTextHeight(const QString& text, QTextEdit* textEdit, bool isHtml = false);

    // 新增：MD格式转HTML核心函数声明（仅新增，无修改）
    QString mdToHtml(const QString& mdText); // 主解析函数
    QString parseMdCodeBlock(const QString& mdText); // 解析代码块``` ```
    QString parseMdTable(const QString& mdText); // 解析表格| | |
};

#endif // CHATSCROLLWIDGET_H