#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_new_record_stt.h"
#include <QThread>
#include <QMutex>
#include "ecommand.h"
#include "chatscrollwidget.h"



class MicThread : public QThread {
    Q_OBJECT
public:
    MicThread(const char* session_begin_params, QObject* parent = nullptr);
    void stop();

protected:
    void run() override;

private:
    const char* m_session_begin_params;
    volatile bool m_stopped;
};


class new_record_stt : public QMainWindow
{
    Q_OBJECT

public:
    explicit new_record_stt(QWidget *parent = Q_NULLPTR);
    //void getReasult();

    ~new_record_stt();
signals:
    void get_text_signal(); // 定义信号
public slots:
    void onRadioButtonClicked();
    void onRecordingFinished();
    void onSaveCSV_clicked();
    QString onGetText(); // 新增槽函数声明
    //QString onGetText2(); // 新增槽函数声明
    void doubaoAnswer();
    void changeArguments();
    void adjustParameters(const QString& findname, int & judge);
    bool applyParameters(); //应用参数
    bool saveToFile(const QString& filePath); //将组合的参数保存为excel文件
    bool execute_Parameters();


private:
    Ui::new_record_sttClass ui;
    bool isRecording; // 标记是否正在录音
    MicThread* m_micThread; // 线程对象
    QString inputText; //输入文本
    QString m_findname; //参数匹配关键字
    int m_judge; //高低匹配关键字
    ON* on = nullptr;
    OFF* off = nullptr;
    IP* ip = nullptr;
    PL* pl = nullptr;
    V* v = nullptr;
    HP* hp = nullptr;
    PP* pp = nullptr;
    AL* al = nullptr;
    OC* oc = nullptr;
    LD* ld = nullptr;
    MU* mu = nullptr;
    GAP* gap = nullptr;
    UP* up = nullptr;
    DN* dn = nullptr;
    CA* ca = nullptr;
    S* s = nullptr;
    LN* ln = nullptr;
    STEP* step = nullptr;
    L* l = nullptr;
    MyLP* mylp = nullptr;

};


