/**
 * @file    chatscrollwidget.h
 * @brief   聊天滚动组件声明
 *
 * 本文件声明了 ChatScrollWidget 组件类，用于在滚动区域中展示对话气泡。
 * 支持两种类型的消息展示：
 * - 用户提问（ChatType::Question）：右对齐、白底黑字、纯文本渲染
 * - AI 回答（ChatType::Answer）：左对齐、带边框、Markdown 转 HTML 渲染
 *
 * 技术实现：
 * - 基于 QScrollArea + QTextEdit 构建消息气泡
 * - 使用 QTextDocument::setMarkdown() 将 Markdown 转为 HTML
 * - 自适应文本高度（通过离屏渲染测量 QTextDocument 实际尺寸）
 * - 新消息自动滚动到底部
 */

#pragma once
#ifndef CHATSCROLLWIDGET_H
#define CHATSCROLLWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QMargins>
#include <QScrollBar>
#include <QTextDocument>

/**
 * @enum ChatType
 * @brief 对话消息类型枚举
 */
enum class ChatType {
    Question,   ///< 用户提问消息
    Answer      ///< AI 模型回答消息
};

/**
 * @class ChatScrollWidget
 * @brief 聊天滚动显示组件
 *
 * 基于 QScrollArea 实现的对话气泡组件，功能包括：
 * - 动态添加对话内容（支持 Markdown 格式的 AI 回答）
 * - 清空所有历史对话记录
 * - 自动滚动到最新消息
 * - 回答消息的代码块、表格样式美化
 *
 * 消息宽度：提问占组件宽度的 60%，回答占组件宽度的 70%。
 */
class ChatScrollWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父控件指针
     *
     * 初始化滚动区域、内容容器和垂直布局管理器。
     */
    explicit ChatScrollWidget(QWidget* parent = nullptr);

    /**
     * @brief 动态添加一条对话消息到聊天区域
     * @param text 对话文本内容（回答类型时按 Markdown 处理）
     * @param type 对话类型（Question=用户提问 / Answer=AI 回答）
     *
     * 添加后自动滚动到底部以显示最新消息。
     */
    void addChatContent(const QString& text, ChatType type);

    /**
     * @brief 清空聊天区域中的所有历史消息
     *
     * 遍历并删除所有子控件和布局项，完全清空聊天记录。
     */
    void clearAllChat();

private:
    QScrollArea* m_scrollArea;      ///< 滚动区域容器
    QWidget* m_contentWidget;       ///< 滚动区域内的内容控件
    QVBoxLayout* m_mainLayout;      ///< 内容控件的垂直布局管理器

    /**
     * @brief 创建单条对话气泡控件
     * @param text  显示文本内容（回答类型为 HTML）
     * @param type  对话类型
     * @return 构建完成的对话气泡 QWidget
     *
     * 根据对话类型创建不同样式和对齐方式的消息气泡。
     */
    QWidget* createChatWidget(const QString& text, ChatType type);

    /**
     * @brief 计算文本渲染后的实际像素高度
     * @param text     文本内容
     * @param textEdit 参考文本框（用于获取样式和字体配置）
     * @param isHtml   是否为 HTML 格式文本（true=HTML 渲染，false=纯文本渲染）
     * @return 文本渲染后的像素高度（最小 20px）
     *
     * 通过创建临时 QTextEdit 进行离屏渲染，
     * 获取 QTextDocument 的实际尺寸后加上补偿值返回。
     */
    int calculateTextHeight(const QString& text, QTextEdit* textEdit, bool isHtml = false);

    /**
     * @brief 将 Markdown 文本转换为 HTML
     * @param mdText Markdown 格式文本
     * @return 转换后的 HTML 文本
     *
     * 使用 Qt 内置的 QTextDocument::setMarkdown() 进行转换，
     * 采用 GitHub 风格的 Markdown 方言以支持代码块和表格。
     */
    QString mdToHtmlByQt(const QString& mdText);
};

#endif // CHATSCROLLWIDGET_H
