
/*
* 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。
*/

#include "new_record_stt.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>
#include <process.h>
#include<iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>
#include <mutex>

#include "msp_cmn.h"
#include "msp_errors.h"
#include "./include/speech_recognizer.h"
#include "doubaoapi.h"
#include <QMessageBox>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>



#ifdef _WIN64
#pragma comment(lib,"../libs/msc_x64.lib")
#else
#pragma comment(lib, "../libs/msc.lib")
#endif

#define FRAME_LEN	640 
#define	BUFFER_SIZE	4096

using namespace std;
using json = nlohmann::json;

DoubaoAI doubao;


enum {
	EVT_START = 0,
	EVT_STOP,
	EVT_QUIT,
	EVT_TOTAL
};
static HANDLE events[EVT_TOTAL] = { NULL,NULL,NULL };

static COORD begin_pos = { 0, 0 };
static COORD last_pos = { 0, 0 };

 //字符编码格式转换
string GbkToUtf8(const char* src_str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}

static void show_result(char* string, char is_over)
{
	COORD orig, current;
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE w = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(w, &info);
	current = info.dwCursorPosition;

	if (current.X == last_pos.X && current.Y == last_pos.Y) {
		SetConsoleCursorPosition(w, begin_pos);
	}
	else {
		/* changed by other routines, use the new pos as start */
		begin_pos = current;
	}
	if (is_over)
		SetConsoleTextAttribute(w, FOREGROUND_GREEN);
	qDebug() << QString("Result: [ %s ]\n")<<string;
	if (is_over)
		SetConsoleTextAttribute(w, info.wAttributes);

	GetConsoleScreenBufferInfo(w, &info);
	last_pos = info.dwCursorPosition;
}
//用户提示
//static void show_key_hints(void)
//{
//	printf("\n\
//----------------------------\n\
//Press r to start speaking\n\
//Press s to end your speaking\n\
//Press q to quit\n\
//----------------------------\n");
//}

/* 上传用户词表 */
static int upload_userwords()
{
	char* userwords = NULL;
	size_t			len = 0;
	size_t			read_len = 0;
	FILE* fp = NULL;
	int				ret = -1;

	fp = fopen("C:\\Users\\32284\\source\\repos\\new_record_stt\\new_record_stt\\userwords.txt", "rb");
	if (NULL == fp)
	{
		printf("\nopen [userwords.txt] failed! \n");
		goto upload_exit;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp); //获取文件大小
	fseek(fp, 0, SEEK_SET);

	userwords = (char*)malloc(len + 1);
	if (NULL == userwords)
	{
		printf("\nout of memory! \n");
		goto upload_exit;
	}

	read_len = fread((void*)userwords, 1, len, fp); //读取用户词表内容
	if (read_len != len)
	{
		printf("\nread [userwords.txt] failed!\n");
		goto upload_exit;
	}
	userwords[len] = '\0';

	MSPUploadData("userwords", userwords, len, "sub = uup, dtt = userword", &ret); //上传用户词表
	if (MSP_SUCCESS != ret)
	{
		printf("\nMSPUploadData failed ! errorCode: %d \n", ret);
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

static char* g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;

//展示读取结果
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
				printf("mem alloc failed\n");
				//return;
			}
		}
		strncat(g_result, result, size);

		//return string(g_result);
		//return QString::fromStdString(g_result);
		//show_result(g_result, is_last);
	}
}

//开始监听
void on_speech_begin()
{
	if (g_result)
	{
		free(g_result);
	}
	g_result = (char*)malloc(BUFFER_SIZE);
	g_buffersize = BUFFER_SIZE;
	memset(g_result, 0, g_buffersize);

	qDebug() << ("Start Listening...\n");

}

//监听结束
void on_speech_end(int reason)
{
	if (reason == END_REASON_VAD_DETECT)
		qDebug() << ("\nSpeaking done \n");
	else
		qDebug() << ("\nRecognizer error %d\n", reason);
}

/* 从文件读取音频 */
static void demo_file(const char* audio_file, const char* session_begin_params)
{
	unsigned int	total_len = 0;
	int				errcode = 0;
	FILE* f_pcm = NULL;
	char* p_pcm = NULL;
	unsigned long	pcm_count = 0;
	unsigned long	pcm_size = 0;
	unsigned long	read_size = 0;
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
		printf("\nopen [%s] failed! \n", audio_file);
		goto iat_exit;
	}

	fseek(f_pcm, 0, SEEK_END);
	pcm_size = ftell(f_pcm); //获取音频文件大小 
	fseek(f_pcm, 0, SEEK_SET);

	p_pcm = (char*)malloc(pcm_size);
	if (NULL == p_pcm)
	{
		printf("\nout of memory! \n");
		goto iat_exit;
	}

	read_size = fread((void*)p_pcm, 1, pcm_size, f_pcm); //读取音频文件内容
	if (read_size != pcm_size)
	{
		printf("\nread [%s] error!\n", audio_file);
		goto iat_exit;
	}

	errcode = sr_init(&iat, session_begin_params, SR_USER, 0, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed : %d\n", errcode);
		goto iat_exit;
	}

	errcode = sr_start_listening(&iat);
	if (errcode) {
		printf("\nsr_start_listening failed! error code:%d\n", errcode);
		goto iat_exit;
	}

	while (1)
	{
		unsigned int len = 10 * FRAME_LEN; // 每次写入200ms音频(16k，16bit)：1帧音频20ms，10帧=200ms。16k采样率的16位音频，一帧的大小为640Byte
		int ret = 0;

		if (pcm_size < 2 * len)
			len = pcm_size;
		if (len <= 0)
			break;

		printf(">");
		ret = sr_write_audio_data(&iat, &p_pcm[pcm_count], len);

		if (0 != ret)
		{
			printf("\nwrite audio data failed! error code:%d\n", ret);
			goto iat_exit;
		}

		pcm_count += (long)len;
		pcm_size -= (long)len;
	}

	errcode = sr_stop_listening(&iat);
	if (errcode) {
		printf("\nsr_stop_listening failed! error code:%d \n", errcode);
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

/* 从麦克风读取音频 */
static void demo_mic(const char* session_begin_params, volatile bool* stopped)
{
	int errcode;
	int i = 0;
	//HANDLE helper_thread = NULL;
	qDebug() << QString("micCcccccccccccccccccccccccccccccccccccccccc\n");
	struct speech_rec iat;
	DWORD waitres;
	char isquit = 0;
	
	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end
	};

	errcode = sr_init(&iat, session_begin_params, SR_MIC, DEFAULT_INPUT_DEVID, &recnotifier);
	if (errcode) {
		qDebug() << "speech recognizer init failed";
		return;
	}

	errcode = sr_start_listening(&iat);
	if (errcode) {
		qDebug() << "start listen failed" << errcode;
		sr_uninit(&iat);
		return;
	}

	// Run until stopped
	while (!*stopped) {
		QThread::msleep(100); // Prevent tight loop, check stop condition periodically
	}

	errcode = sr_stop_listening(&iat);
	if (errcode) {
		qDebug() << "stop listening failed" << errcode;
	}

	sr_uninit(&iat);

	qDebug() << "Reasult:" << QString::fromLocal8Bit(g_result);
	


//exit:
//	//if (helper_thread != NULL) {
//	//	WaitForSingleObject(helper_thread, INFINITE);
//	//	CloseHandle(helper_thread);
//	//}
//
//	for (i = 0; i < EVT_TOTAL; ++i) {
//		if (events[i])
//			CloseHandle(events[i]);
//	}
//
//	sr_uninit(&iat);
}
// MicThread Implementation
MicThread::MicThread(const char* session_begin_params, QObject* parent)
	: QThread(parent), m_session_begin_params(session_begin_params), m_stopped(false)
{
}

void MicThread::stop()
{
	m_stopped = true;
}

void MicThread::run()
{
	demo_mic(m_session_begin_params, &m_stopped);
}


//匹配参数关键字
QString findNameByWord(const QString& sttText)
{

	std::ifstream file;
	file.open("C:\\Users\\32284\\source\\repos\\new_record_stt\\new_record_stt\\userwords.json");
	if (!file.is_open()) {
		//return QString("文件打开失败: 无法打开文件 userwords.json");
		QMessageBox::warning(nullptr, "正确", "文件打开成功: userwords.json");
	}

	if (!file.is_open()) {
		//return QString("文件打开失败: 无法打开文件 userwords.json");
		QMessageBox::warning(nullptr, "错误", "文件打开失败: 无法打开文件 userwords.json");
	}

	// 读取文件内容
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	// 解析 JSON 数据
	json data;
	try {
		data = json::parse(content);
	}
	catch (json::parse_error& e) {
		//QMessageBox::warning(nullptr, "错误", "JSON 解析错误");
		qDebug() << QString("JSON 解析错误");
		//return QString("JSON 解析错误: ") + QString(e.what());
	}

	// 检查 "userword" 是否存在且是一个数组
	if (!data.contains("userword") || !data["userword"].is_array()) {
		//QMessageBox::warning(nullptr, "错误", "JSON 格式不正确");
		qDebug() << QString("JSON 格式不正确");
		//return QString("JSON 格式不正确");
	}

	QByteArray sttTextUtf = sttText.toUtf8();
	string sttTextUtf8 = sttTextUtf.toStdString();
	// 遍历 "userword" 数组
	for (const auto& item : data["userword"]) {
		if (item.contains("words") && item["words"].is_array() && item.contains("name") && item["name"].is_string()) {
			// 检查 "words" 数组中的每个字符串
			for (const auto& word : item["words"]) {

				if (sttTextUtf8.find(word.get<std::string>()) != string::npos) { //

					return QString::fromStdString(item["name"].get<string>());
				}
			}
		}
	}


	//QMessageBox::warning(nullptr, "错误", "未找到匹配,请您重新输入");
	qDebug() << QString("未找到匹配,请您重新输入");
	return QString("Unknown");


}

//匹配调高或者调低关键字
int judge_word(QString& s)
{
	// 检查字符串中是否包含“高”
	if (s.contains("\u9ad8")) {
		return 1;
	}
	// 检查字符串中是否包含“低”
	else if (s.contains("\u4f4e")) {
		return -1;
	}
	else {
		qDebug() << QString("字符串中无调节参数，请重新输入！");
		return 0;
	}
}

const char* session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = gb2312";

new_record_stt::new_record_stt(QWidget *parent) : 
	QMainWindow(parent), 
	m_micThread(nullptr),
	on(new ON(0)),
	off(new OFF(0)),
	ip(new IP(0.0)),
	pl(new PL('+')),  // char类型用空字符'\0'初始化，比0更规范
	v(new V(0)),
	hp(new HP(0)),
	pp(new PP(0)),
	al(new AL(0)),
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
	mylp(new MyLP(0))   // MyLP无显式构造函数，先new再赋值
{
    ui.setupUi(this);
	int			ret = MSP_SUCCESS;
	int			upload_on = 1; //是否上传用户词表
	const char* login_params = "appid = 8257763a, work_dir = ."; // 登录参数，appid与msc库绑定,请勿随意改动
	int aud_src = 1; //从麦克风读取
	m_findname = NULL;
	m_judge = 0;
	inputText = NULL;
	isRecording = false;
	

	qDebug() << "getReasult function entered";

	/*
	* sub:				请求业务类型
	* domain:			领域
	* language:			语言
	* accent:			方言
	* sample_rate:		音频采样率
	* result_type:		识别结果格式
	* result_encoding:	结果编码格式
	*
	*/

	// 初始化events事件标志
	for (int i = 0; i < EVT_TOTAL; ++i) {
		events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}


	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数	
	if (MSP_SUCCESS != ret) {
		qDebug() << "MSPLogin failed, Error code" << ret;
		QMessageBox::critical(this, "错误", "语音识别初始化失败，请检查配置。");
		ui.radioButton->setEnabled(false); // 禁用按钮以防止后续操作
	}
	else {
		m_micThread = new MicThread(session_begin_params, this); // 仅在成功时初始化
	}

	qDebug() << ("\n########################################################################\n");
	qDebug() << QString("## The iFly Auto Transform technology can convert speech into corresponding text in real time.##\n");
	qDebug() << ("########################################################################\n\n");
	//qDebug() << ("演示示例选择:是否上传用户词表？\n0:不使用\n1:使用\n");

	//scanf("%d", &upload_on);
	if (upload_on)
	{
		qDebug() << QString("sending User dictionary ...\n");
		ret = upload_userwords();
		if (MSP_SUCCESS != ret)
			//goto exit;
		qDebug() << QString("sending User dictionary succeed!\n");
	}

	ui.m_ON_value->setText(QString::number(on->getON()));
	ui.m_OFF_value->setText(QString::number(off->getOFF()));
	ui.m_PL_value->setText(QString(pl->getPL()));
	ui.m_V_value->setText(QString::number(v->getV()));
	ui.m_HP_value->setText(QString::number(hp->getHP()));
	ui.m_PP_value->setText(QString::fromStdString(pp->getPPDisplay()));
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

	// ========== double类型参数（可选保留小数位数，示例保留2位） ==========
	ui.m_IP_value->setText(QString::number(ip->getIP(), 'f', 2));


	//初始化events事件标志
	for (int i = 0; i < EVT_TOTAL; ++i) {
		events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	// 连接按钮的 clicked 信号到槽函数
	//connect(ui.m_affirm_pushButton, &QPushButton::clicked, this, &new_record_stt::onGetText);
	//connect(ui.pushButton_2, &QPushButton::clicked, this, &new_record_stt::onGetText2);
	connect(ui.radioButton, &QRadioButton::clicked, this, &new_record_stt::onRadioButtonClicked);
	connect(this, &new_record_stt::get_text_signal, this, &new_record_stt::onRecordingFinished);

	connect(ui.m_affirm_pushButton, &QPushButton::clicked, this, &new_record_stt::changeArguments);
	connect(ui.m_affirm_pushButton, &QPushButton::clicked, this, &new_record_stt::doubaoAnswer);

	//connect(ui.pushButton_2, &QPushButton::clicked, this, &new_record_stt::changeArguments);
	connect(ui.radioButton, &QRadioButton::clicked, this, &new_record_stt::changeArguments);

	connect(ui.m_Save_Parameters_Button, &QPushButton::clicked, this, &new_record_stt::onSaveCSV_clicked);
	//connect(ui.m_Apply_Parameters_Button, &QPushButton::clicked, this, &new_record_stt::applyParameters);
	//connect(ui.m_Execute_Button, &QPushButton::clicked, this, &new_record_stt::execute_Parameters);




	// 在界面显示后延迟一段时间调用 
	QTimer::singleShot(1000, this, &new_record_stt::changeArguments);
	//HANDLE helperThread = start_helper_thread(this);

	//QMessageBox::information(this, "text", QString::fromStdString(g_result));
	qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";


}



void new_record_stt::onRecordingFinished() {
	qDebug() << "isRecording:" << isRecording << "g_result:" << g_result; // 新增打印
	ui.m_input_textEdit->setText(QString("no g_result..."));
	if (!isRecording) {
		// 如果录音已停止，显示识别结果
		if (g_result) {
			ui.m_input_textEdit->setText(QString::fromLocal8Bit(g_result));
		}
		else {
			ui.m_input_textEdit->setText(QString("no g_result..."));
		}
	}
	else {
		// 如果正在录音，显示提示信息
		ui.m_input_textEdit->setText(QString("recording..."));
	}
}


void new_record_stt::onRadioButtonClicked()
{
	if (!isRecording) {
		if (m_micThread == nullptr) {
			qDebug() << "无法启动麦克风线程，因为初始化失败。";
			//QMessageBox::warning(this, "警告", "由于初始化失败，麦克风线程不可用。");
			return;
		}
		isRecording = true;
		qDebug() << "Starting microphone thread...";
		m_micThread->start();
	}
	else {
		isRecording = false;
		qDebug() << "Stopping microphone thread...";
		m_micThread->stop();
		m_micThread->wait(); // Wait for the thread to finish
	}
	onRecordingFinished();

	
}

void new_record_stt::onSaveCSV_clicked() {
	QString filePath = QFileDialog::getSaveFileName(this, "保存参数", "./params.csv", "CSV文件 (*.csv)");
	if (!filePath.isEmpty()) {
		saveToFile(filePath);
	}
}

//获取文本框内容，现已废弃
QString new_record_stt::onGetText()
{
	// 获取 QTextEdit 中的文本
	QString text = ui.m_input_textEdit->toPlainText();

	if (!text.isEmpty()) {
		// ui.chatScrollWidget 是自动生成的指针，直接调用addChatContent
		// 类型为ChatType::Question（用户问题）
		ui.chatScrollWidget->addChatContent(text, ChatType::Question);
	}
	return text;
}

////大模型回复
//QString new_record_stt::onGetText2()
//{
//
//	emit get_text_signal();
//	return ui.m_answer_textEdit->toPlainText();
//	//return text;
//}

void new_record_stt::doubaoAnswer() {
	QString text = ui.m_input_textEdit->toPlainText();
	ui.m_input_textEdit->clear(); //清除输入框内容
	QString answer = doubao.DoubaoAI_request(text);
	ui.chatScrollWidget->addChatContent(answer, ChatType::Answer);
	qDebug() << "Answer:" << answer;
	//ui.m_answer_textEdit->setText(answer);
}


void new_record_stt::changeArguments()
{

	// 判断文字输入框是否有输入
	if (!ui.m_input_textEdit->toPlainText().isEmpty()) {
		// 执行文字输入相关的操作
		inputText = onGetText();
		qDebug() << "Text input:" << inputText;
		// 
		m_findname = findNameByWord(inputText);
		// 寻找匹配的参数
		qDebug() << QString("m_findname:") << m_findname;
		//判断调高还是调低
		m_judge = judge_word(inputText);
		qDebug() << QString("m_judge:") << m_judge;
	}
	else if (!ui.m_input_textEdit->toPlainText().isEmpty()) {
		// 判断语音输入框是否有输入
		//text = onGetText2();
		inputText = ui.m_input_textEdit->toPlainText();

		qDebug() << "Voice input:" << inputText;
		// 
		m_findname = findNameByWord(inputText);
		// 寻找匹配的参数
		qDebug() << QString("m_findname:") << m_findname;
		//判断调高还是调低
		m_judge = judge_word(inputText);
		qDebug() << QString("m_judge:") << m_judge;
	}
	else {
		// 如果都没有输入，可以提示用户
		qDebug() << "No input detected";
	}
	//m_findname = QString("130");
	adjustParameters(m_findname, m_judge);

}

void new_record_stt::adjustParameters(const QString& findname, int & judge) {
	qDebug() << "findname in adjustParameters:" << findname; // 调试输出
	if (findname == QString("130")) {
		on->setON(14);
		off->setOFF(15);
		ip->setIP(4.0);
		v->setV(1);
		mu->setMU(1);
		ui.m_ON->setText(QString::number(on->getON()));
		ui.m_OFF->setText(QString::number(off->getOFF()));
		ui.m_IP->setText(QString::number(ip->getIP(),'d', 1));
		ui.m_V->setText(QString::number(v->getV()));
		ui.m_MU->setText(QString::number(mu->getMU()));

	}
	if (findname == QString("ON")) {
		on->setON(judge);
		ui.m_ON->setText(QString::number(on->getON()));
		qDebug() << QString("Parameter adjustment successful!");

	}
	else if (findname == QString("OFF")) {
		off->setOFF(judge);
		ui.m_OFF->setText(QString::number(off->getOFF()));
		qDebug() << QString("Parameter adjustment successful!");
		qDebug() << QString("The adjusted parameter values are:") << off->getOFF();
	}
	else if (findname == QString("IP")) {
		ip->setIP(judge);
		ui.m_IP->setText(QString::number(ip->getIP()));
		qDebug() << QString("Parameter adjustment successful!");
		qDebug() << QString("The adjusted parameter values are:") << ip->getIP();
	}
	else if (findname == QString("V")) {
		v->setV(judge);
		ui.m_V->setText(QString::number(v->getV()));
		qDebug() << QString("Parameter adjustment successful!");
		qDebug() << QString("The adjusted parameter values are:") << v->getV();
	}
	else if (findname == QString("MU")) {
		mu->setMU(judge);
		ui.m_MU->setText(QString::number(mu->getMU()));
		qDebug() << QString("Parameter adjustment successful!");
		qDebug() << QString("The adjusted parameter values are:") << mu->getMU();
	}
	else {
		qDebug() << QString("Parameters match failed!") ;
	}
}

bool new_record_stt::applyParameters() {
	return true;
}

bool new_record_stt::execute_Parameters() {
	return true;
}

bool new_record_stt::saveToFile(const QString& filePath) {
	// 1. 打开文件（追加模式 + 写入模式 + 文本模式）
// Append：追加（不覆盖）；WriteOnly：写入；Text：文本模式
	QFile file(filePath);
	if (!file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "错误", "无法打开文件：" + filePath);
		return false;
	}

	QTextStream out(&file);
	out.setEncoding(QStringConverter::Utf8);

	// 2. 定义列标签（参数名称）和对应的数值
	QList<QPair<QString, QString>> paramList;
	// ===== 填充所有参数（顺序不变）=====
	paramList << QPair<QString, QString>("ON", QString::number(on->getON()));
	paramList << QPair<QString, QString>("OFF", QString::number(off->getOFF(), 'f', 2));
	paramList << QPair<QString, QString>("IP", QString::number(ip->getIP(), 'f', 2));
	paramList << QPair<QString, QString>("PL", QString(pl->getPL()));  // char类型特殊处理
	paramList << QPair<QString, QString>("V", QString::number(v->getV()));
	paramList << QPair<QString, QString>("HP", QString::number(hp->getHP()));
	paramList << QPair<QString, QString>("PP", QString::number(pp->getPP()));
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



	// 3. 判断是否是首次创建文件（文件为空/新文件），仅首次写列标签
	bool isNewFile = (file.size() == 0); // 文件大小为0 = 首次创建
	if (isNewFile) {
		// 首次保存：写入列标签（第一行）
		QString headerLine;
		for (int i = 0; i < paramList.size(); ++i) {
			headerLine += paramList[i].first;
			if (i != paramList.size() - 1) {
				headerLine += ",";  // CSV列分隔符
			}
		}
		out << headerLine << "\n";  // 换行
	}

	// 4. 写入参数数值（追加行，每次保存新增一行）
	QString valueLine;
	for (int i = 0; i < paramList.size(); ++i) {
		valueLine += paramList[i].second;
		if (i != paramList.size() - 1) {
			valueLine += ",";
		}
	}
	out << valueLine << "\n";

	// 5. 关闭文件
	file.close();

	QMessageBox::information(this, "成功", "参数已追加保存到：" + filePath);
	return true;
}

new_record_stt::~new_record_stt()
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

	// 释放后置空，避免野指针
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

