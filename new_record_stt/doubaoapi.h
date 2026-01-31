//Í·ÎÄ¼þ
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

class DoubaoAI : public QObject
{
    Q_OBJECT
public:
    DoubaoAI();
    QString DoubaoAI_request(QString& question);
private:
    QString OPENAI_BASE_URL = "https://ark.cn-beijing.volces.com/api/v3/chat/completions";
    QString OPENAI_API_KEY = "2390cff3-ed6f-43a2-b1c7-52d42785e880";
    QString OPENAI_MODEL = "doubao-seed-1-8-251228";
};

#endif // DOUBAOAPI_H
#pragma once
