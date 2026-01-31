#include "chatscrollwidget.h"

ChatScrollWidget::ChatScrollWidget(QWidget* parent) : QWidget(parent)
{
    // 1. 创建外层布局（当前Widget的主布局）
    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0); // 去除外层边距，让滚动窗口占满父控件
    outerLayout->setSpacing(0);

    // 2. 初始化滚动区域核心控件
    m_scrollArea = new QScrollArea(this);
    m_contentWidget = new QWidget(m_scrollArea);
    m_mainLayout = new QVBoxLayout(m_contentWidget);

    // 3. 配置滚动区域（关键：让主容器随内容扩展，滚动条正常工作）
    m_scrollArea->setWidget(m_contentWidget);          // 设置滚动区域的核心Widget
    m_scrollArea->setWidgetResizable(true);            // 必须开启！主容器随内容自动调整大小
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏水平滚动条（只垂直滚动）
    m_scrollArea->setStyleSheet("QScrollArea { border: none; }"); // 去除滚动区域边框（可选，美化）

    // 4. 配置主内容布局（问答Widget的容器布局）
    m_mainLayout->setAlignment(Qt::AlignTop); // 内容从上到下排列
    m_mainLayout->setSpacing(15);             // 问答Widget之间的间距（可自定义）
    m_mainLayout->setContentsMargins(20, 20, 20, 20); // 主容器的内边距（可自定义）

    // 5. 将滚动区域添加到外层布局
    outerLayout->addWidget(m_scrollArea);

    // 设置当前Widget的布局
    this->setLayout(outerLayout);
}

void ChatScrollWidget::addChatContent(const QString& text, ChatType type)
{
    // ========== 修改点1：回答文本先做MD转HTML解析，问题保持原文本 ==========
    QString showText = text;
    if (type == ChatType::Answer) {
        showText = mdToHtml(text); // 回答自动解析MD格式
    }
    // 1. 创建问答Widget并添加到主布局（替换为解析后的文本）
    QWidget* chatWidget = createChatWidget(showText, type);
    m_mainLayout->addWidget(chatWidget);

    // 2. 自动滚动到底部（关键：展示最新内容）
    QScrollBar* vScrollBar = m_scrollArea->verticalScrollBar();
    vScrollBar->setValue(vScrollBar->maximum());
}

void ChatScrollWidget::clearAllChat()
{
    // 遍历布局，删除所有子Widget，清空历史对话
    while (QLayoutItem* item = m_mainLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

QWidget* ChatScrollWidget::createChatWidget(const QString& text, ChatType type)
{
    // 1. 创建当前问答的根Widget（承载文本控件）
    QWidget* rootWidget = new QWidget();
    QHBoxLayout* rootLayout = new QHBoxLayout(rootWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 2. 创建文本展示控件（QTextEdit，支持自动换行+高度自适应）
    QTextEdit* textEdit = new QTextEdit(); // ========== 修改点2：先空创建，后续分模式设置文本 ==========
    textEdit->setReadOnly(true); // 只读，禁止编辑
    textEdit->setLineWrapMode(QTextEdit::WidgetWidth); // 按控件宽度自动换行（核心）
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏文本内部滚动条
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // 保留你新增的任意位置换行

    // 3. 区分问题/回答的样式表（核心：视觉区分，可自定义）
    if (type == ChatType::Question) {
        // 问题样式：右对齐、浅灰色背景、文字黑色、最大宽度60%（防止文字过长）
        rootLayout->setAlignment(Qt::AlignRight); // 整个Widget右对齐
        textEdit->setPlainText(text); // ========== 问题：纯文本模式 ==========
        textEdit->setStyleSheet(R"(
            QTextEdit {
                background-color: #ECECEC;
                color: #000000;
                border: none;
                border-radius: 10px;
                padding: 5px 5px;
                font-size: 12px;
            }
        )");
        textEdit->setMaximumWidth(this->width() * 0.6); // 最大宽度为滚动窗口的60%
        // 计算高度：纯文本模式
        int textRealHeight = calculateTextHeight(text, textEdit, false);
        textEdit->setFixedHeight(textRealHeight);
    }
    else {
        // 回答样式：左对齐、白色背景、文字深灰色、最大宽度70% + 新增MD可视化样式
        rootLayout->setAlignment(Qt::AlignLeft); // 整个Widget左对齐
        textEdit->setHtml(text); // ========== 修改点3：回答：HTML富文本模式（核心） ==========
        // 样式表新增：代码块、表格的可视化样式，保留你原有所有属性
        textEdit->setStyleSheet(R"(
            QTextEdit {
                background-color: #FFFFFF;
                color: #333333;
                border: 1px solid #E0E0E0;
                border-radius: 10px;
                padding: 5px 5px;
                font-size: 12px;
                line-height: 1.4; /* 新增：行高，提升阅读体验 */
            }
            /* 代码块样式：灰色背景+等宽字体+内边距 */
            QTextEdit pre {
                background-color: #f5f5f5;
                border-radius: 6px;
                padding: 6px;
                margin: 4px 0;
                font-family: Consolas, Monaco, monospace;
                font-size: 11px;
                border: 1px solid #e0e0e0;
            }
            /* 表格样式：合并边框+表头浅灰+内边距 */
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
        textEdit->setMaximumWidth(this->width() * 0.7); // 最大宽度为滚动窗口的70%
        // 计算高度：HTML模式
        int textRealHeight = calculateTextHeight(text, textEdit, true);
        textEdit->setFixedHeight(textRealHeight);
    }

    // 5. 将文本控件添加到根布局
    rootLayout->addWidget(textEdit);
    rootWidget->setLayout(rootLayout);

    return rootWidget;
}

int ChatScrollWidget::calculateTextHeight(const QString& text, QTextEdit* textEdit, bool isHtml)
{
    // 1. 创建临时QTextEdit，完全复制实际控件的所有关键属性（保证换行效果一致）
    QTextEdit tempEdit;
    tempEdit.setStyleSheet(textEdit->styleSheet()); // 复制样式（padding/颜色/字体大小）
    tempEdit.setFont(textEdit->font());             // 复制字体（行高/字号，影响换行和高度）
    tempEdit.setLineWrapMode(textEdit->lineWrapMode()); // 同步换行模式（WidgetWidth）
    tempEdit.setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); // 同步任意位置换行
    tempEdit.setMaximumWidth(textEdit->maximumWidth()); // 同步最大宽度（核心！决定换行数）

    // ========== 修改点4：根据模式设置文本（纯文本/HTML） ==========
    if (isHtml) {
        tempEdit.setHtml(text); // 回答：HTML模式
    }
    else {
        tempEdit.setPlainText(text); // 问题：纯文本模式
    }

    // 2. 关键步骤：强制文档重排，让高度适配换行后的实际内容（缺一不可）
    tempEdit.document()->adjustSize();

    // 3. 获取换行后文档的真实高度（仅文字内容，不含padding）
    qreal docRealHeight = tempEdit.document()->size().height();

    // 4. 手动设置：与你样式表中的上下padding一致（保留你原有值：5px）
    int paddingTop = 5;    // 样式表padding的「上」值（px）
    int paddingBottom = 5; // 样式表padding的「下」值（px）

    // 5. 总高度：完全保留你原有计算逻辑！仅HTML模式加少量冗余（适配表格/代码块）
    int totalHeight = 0;
    if (isHtml) {
        totalHeight = qRound(docRealHeight) + paddingTop + paddingBottom + 5; // HTML加5px冗余
    }
    else {
        totalHeight = qRound(docRealHeight - 1) + 3; // 完全保留你原有纯文本计算逻辑
    }

    // 兜底：最小高度（避免空文本/超短文本时控件过窄，适配你的样式）
    return qMax(totalHeight, 20);
}

// ===================== 新增：MD格式转HTML核心解析函数（无侵入，仅新增） =====================
// 主解析函数：依次解析代码块、表格，替换换行为<br>
QString ChatScrollWidget::mdToHtml(const QString& mdText)
{
    QString html = mdText;
    html = parseMdTable(html);     // 再解析表格
    html = parseMdCodeBlock(html); // 先解析代码块
    html = html.replace("\n", "<br>"); // 普通换行转HTML换行
    return html;
}

// 解析MD代码块：```xxx``` → <pre><code>xxx</code></pre>
QString ChatScrollWidget::parseMdCodeBlock(const QString& mdText)
{
    // 正则匹配```开头、```结尾的代码块（兼容任意语言标记）
    QRegularExpression reg(R"(```(.*?)\n([\s\S]*?)\n```)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(mdText);
    QString result = mdText;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString codeContent = match.captured(2).trimmed().toHtmlEscaped(); // 转义HTML特殊字符，避免渲染异常
        QString htmlCode = QString("<pre><code>%1</code></pre>").arg(codeContent);
        result.replace(match.captured(0), htmlCode);
    }
    return result;
}

// 解析MD表格：| 表头 | → <table><th>表头</th></table>
QString ChatScrollWidget::parseMdTable(const QString& mdText)
{
    // 创建非const副本，避免修改原始const对象
    QString mdTextCopy = mdText;
    // 预处理：去除多余空行，统一换行符
    mdTextCopy = mdTextCopy.replace("\r\n", "\n").replace("\r", "\n");
    QStringList lines = mdTextCopy.split("\n", Qt::SkipEmptyParts);
    QStringList tableLines;
    bool inTable = false;

    // 遍历所有行，识别表格块（容错性更强）
    for (const QString& line : lines) {
        QString trimLine = line.trimmed();
        // 识别表格行：包含至少一个|，且不是代码块/标题行
        if (trimLine.contains("|") && !trimLine.startsWith("```") && !trimLine.startsWith("#")) {
            tableLines.append(line);
            inTable = true;
        }
        else if (inTable) {
            // 遇到非表格行，结束表格识别
            break;
        }
    }

    if (tableLines.size() < 2) return mdTextCopy; // 至少需要2行才可能是表格

    // 构建HTML表格
    QString htmlTable = "<table>";
    bool isHeader = true;

    for (const QString& line : tableLines) {
        QString trimLine = line.trimmed();
        // 处理无首尾|的情况：自动补全
        if (!trimLine.startsWith("|")) trimLine = "|" + trimLine;
        if (!trimLine.endsWith("|")) trimLine = trimLine + "|";

        QString lineContent = trimLine.mid(1, trimLine.size() - 2); // 去除首尾|
        QStringList cells = lineContent.split("|");

        // 跳过分隔行（包含---的行）
        if (trimLine.contains("---")) {
            continue;
        }

        htmlTable += "<tr>";
        for (const QString& cell : cells) {
            QString cellContent = cell.trimmed().toHtmlEscaped();
            if (isHeader) {
                htmlTable += QString("<th>%1</th>").arg(cellContent);
            }
            else {
                htmlTable += QString("<td>%1</td>").arg(cellContent);
            }
        }
        htmlTable += "</tr>";
        isHeader = false; // 第一行之后都是内容行
    }

    htmlTable += "</table>";

    // 替换原表格块为HTML表格
    mdTextCopy.replace(tableLines.join("\n"), htmlTable);
    return mdTextCopy;
}