/**
 * @file    main.cpp
 * @brief   应用程序入口文件
 *
 * 本文件包含程序的 main() 入口函数，负责以下初始化工作：
 * - 设置本地字符编码为 GBK，确保中文界面文本在 Qt 控件中正常显示
 * - 创建 QApplication 应用程序实例
 * - 创建并显示 EDMIntelligentControlSystem 主窗口
 * - 启动 Qt 事件循环
 *
 * @author wang
 * @date   2026
 */

#include "EDM_intelligent_control_system.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>
#include <QThread>

/**
 * @brief  应用程序主入口
 * @param  argc  命令行参数个数
 * @param  argv  命令行参数数组
 * @return 应用程序退出码（由 QApplication::exec() 返回）
 *
 * 执行流程：
 * 1. QTextCodec::setCodecForLocale("GBK") — 设置本地 GBK 编码，确保中文字符正确显示
 * 2. QApplication a(argc, argv)         — 创建 Qt 应用程序实例
 * 3. EDMIntelligentControlSystem w; w.show()         — 创建并显示主窗口
 * 4. return a.exec()                    — 进入 Qt 事件循环，阻塞直到程序退出
 *
 * @note 编码设置必须在 QApplication 创建之前完成，否则某些控件的默认字体可能乱码。
 */
int main(int argc, char *argv[])
{
    /* 设置本地编码为 GBK，保证中文文本在 Qt 控件中正确渲染 */
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));

    QApplication a(argc, argv);
    EDMIntelligentControlSystem w;
    w.show();

    return a.exec();
}
