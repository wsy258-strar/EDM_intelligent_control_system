/**
 * @file    ChatScrollWidget.cpp
 * @brief   聊天滚动组件实现
 *
 * 实现了 ChatScrollWidget 组件的全部功能：
 * - 聊天消息气泡的创建与布局（提问右对齐，回答左对齐）
 * - Markdown 到 HTML 的转换（支持代码块语法高亮、表格边框样式）
 * - 自适应文本高度计算（离屏渲染 + 高度补偿算法）
 * - 新消息自动滚动到底部
 *
 * CSS 样式通过 QTextEdit 的 setStyleSheet 内联设置，
 * 包含消息气泡、代码块和表格的完整样式定义。
 */

#include "chatscrollwidget.h"

/**
 * @brief 构造函数，构建聊天组件的初始化布局
 * @param parent 父控件指针
 *
 * 布局结构：
 * this（外层）
 *   └── outerLayout
 *         └── m_scrollArea（可滚动区域）
 *               └── m_contentWidget（内容容器）
 *                     └── m_mainLayout（消息列表，垂直排列）
 */
ChatScrollWidget::ChatScrollWidget(QWidget* parent) : QWidget(parent)
{
    /* 外层布局：容纳滚动区域 */
    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    /* 创建滚动区域及内部内容容器 */
    m_scrollArea = new QScrollArea(this);
    m_contentWidget = new QWidget(m_scrollArea);
    m_mainLayout = new QVBoxLayout(m_contentWidget);

    /* 配置滚动区域：允许自适应宽度，隐藏水平滚动条，去除边框 */
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; }");

    /* 内容布局：顶部对齐，消息间距 15px，内边距 20px */
    m_mainLayout->setAlignment(Qt::AlignTop);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    outerLayout->addWidget(m_scrollArea);
    this->setLayout(outerLayout);
}

/**
 * @brief 向聊天区域追加一条消息
 * @param text 消息文本内容
 * @param type 消息类型（Question 或 Answer）
 *
 * 处理逻辑：
 * - Answer 类型：先将 Markdown 文本转换为 HTML，再创建气泡
 * - 添加消息后自动触发滚动到底部
 */
void ChatScrollWidget::addChatContent(const QString& text, ChatType type)
{
    QString showText = text;
    if (type == ChatType::Answer) {
        showText = mdToHtmlByQt(text);
    }

    QWidget* chatWidget = createChatWidget(showText, type);
    m_mainLayout->addWidget(chatWidget);

    /* 自动滚动到最新消息位置 */
    QScrollBar* vScrollBar = m_scrollArea->verticalScrollBar();
    vScrollBar->setValue(vScrollBar->maximum());
}

/**
 * @brief 清空聊天区域中的所有历史消息
 *
 * 遍历 m_mainLayout 中的所有布局项和子控件，逐一安全删除。
 * 使用 deleteLater() 延迟删除子控件以避免事件循环中的悬挂指针问题。
 */
void ChatScrollWidget::clearAllChat()
{
    while (QLayoutItem* item = m_mainLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

/**
 * @brief 创建单条对话气泡控件
 * @param text  显示文本内容（Answer 类型时为 HTML 格式）
 * @param type  对话类型
 * @return 构建完成的对话气泡 QWidget
 *
 * 样式差异说明：
 * - 提问消息（Question）：右对齐，白底黑字，圆角无边框，宽度上限 60%
 * - 回答消息（Answer）：左对齐，白底灰字，圆角灰边框，宽度上限 70%，
 *   额外包含代码块（等宽字体灰底）和表格（边框合并）样式
 */
QWidget* ChatScrollWidget::createChatWidget(const QString& text, ChatType type)
{
    QWidget* rootWidget = new QWidget();
    QHBoxLayout* rootLayout = new QHBoxLayout(rootWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    /* 创建只读文本框，禁用滚动条，启用自动换行 */
    QTextEdit* textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    if (type == ChatType::Question) {
        /* 提问消息：右对齐，纯文本展示，简洁白底风格 */
        rootLayout->setAlignment(Qt::AlignRight);
        textEdit->setPlainText(text);
        textEdit->setStyleSheet(R"(
            QTextEdit {
                background-color: #FFFFFF;
                color: #000000;
                border: none;
                border-radius: 10px;
                padding: 5px 5px;
                font-size: 12px;
            }
        )");
        textEdit->setMaximumWidth(this->width() * 0.6);
        int textRealHeight = calculateTextHeight(text, textEdit, false);
        textEdit->setFixedHeight(textRealHeight);
    }
    else {
        /* 回答消息：左对齐，HTML 渲染，包含代码块和表格的额外样式 */
        rootLayout->setAlignment(Qt::AlignLeft);
        textEdit->setHtml(text);
        textEdit->setStyleSheet(R"(
            QTextEdit {
                background-color: #FFFFFF;
                color: #333333;
                border: 1px solid #E0E0E0;
                border-radius: 10px;
                padding: 5px 5px;
                font-size: 12px;
                line-height: 1.4;
            }
            QTextEdit pre {
                background-color: #f5f5f5;
                border-radius: 6px;
                padding: 6px;
                margin: 4px 0;
                font-family: Consolas, Monaco, monospace;
                font-size: 12px;
                border: 1px solid #e0e0e0;
            }
            QTextEdit table {
                border-collapse: collapse;
                margin: 4px 0;
                width: 100%;
                font-size: 11px;
            }
            QTextEdit th, QTextEdit td {
                border: 1px solid #e0e0e0;
                padding: 3px 6px;
                text-align: left;
            }
            QTextEdit th {
                background-color: #f8f8f8;
            }
        )");
        textEdit->setMaximumWidth(this->width() * 0.7);
        int textRealHeight = calculateTextHeight(text, textEdit, true);
        textEdit->setFixedHeight(textRealHeight);
    }

    rootLayout->addWidget(textEdit);
    rootWidget->setLayout(rootLayout);

    return rootWidget;
}

/**
 * @brief 计算文本渲染后的实际像素高度
 * @param text     文本内容
 * @param textEdit 参考文本框（用于复制其样式、字体和换行配置）
 * @param isHtml   是否为 HTML 格式（true=HTML 渲染，false=纯文本渲染）
 * @return 文本渲染后的像素高度（最小 20px）
 *
 * 算法说明：
 * - 创建临时 QTextEdit 作为离屏渲染目标
 * - 复制参考文本框的样式、字体和最大宽度
 * - 根据 isHtml 分别调用 setHtml() 或 setPlainText()
 * - 调用 QTextDocument::adjustSize() 触发重新布局
 * - 读取 QTextDocument::size().height() 获取渲染高度
 * - HTML 和纯文本使用不同的高度补偿策略以消除多余留白
 */
int ChatScrollWidget::calculateTextHeight(const QString& text, QTextEdit* textEdit, bool isHtml)
{
    /* 创建临时文本框用于离屏渲染测量 */
    QTextEdit tempEdit;
    tempEdit.setStyleSheet(textEdit->styleSheet());
    tempEdit.setFont(textEdit->font());
    tempEdit.setLineWrapMode(textEdit->lineWrapMode());
    tempEdit.setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    tempEdit.setMaximumWidth(textEdit->maximumWidth());

    if (isHtml) {
        tempEdit.setHtml(text);
    }
    else {
        tempEdit.setPlainText(text);
    }

    /* 触发 QTextDocument 重新计算布局尺寸 */
    tempEdit.document()->adjustSize();

    qreal docRealHeight = tempEdit.document()->size().height();

    /* 高度补偿值（经验值，根据实际渲染效果微调） */
    const int paddingTop = 5;
    const int paddingBottom = 5;

    int totalHeight = 0;
    if (isHtml) {
        /* HTML 渲染：补偿上下 padding 和额外边距 */
        totalHeight = qRound(docRealHeight) + paddingTop + paddingBottom + 5;
    }
    else {
        /* 纯文本渲染：微量补偿即可，避免底部出现过多留白 */
        totalHeight = qRound(docRealHeight - 1) + 3;
    }

    return qMax(totalHeight, 20);
}

/**
 * @brief 将 Markdown 文本转换为 HTML
 * @param mdText Markdown 格式的原始文本
 * @return 转换后的 HTML 文本
 *
 * 使用 Qt 5.14+ 内置的 QTextDocument::setMarkdown() 进行转换，
 * 选择 GitHub 方言以获得最优的代码块和表格兼容性。
 *
 * @note 该方法是 Qt 5.14 引入的特性，若在更低版本中使用需自行实现转换。
 */
QString ChatScrollWidget::mdToHtmlByQt(const QString& mdText)
{
    QTextDocument doc;
    doc.setMarkdown(mdText, QTextDocument::MarkdownDialectGitHub);
    return doc.toHtml();
}
