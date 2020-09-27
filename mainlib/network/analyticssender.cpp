#include "analyticssender.h"

#include <config/bigringsettings.h>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QUrlQuery>

namespace {
const int SEND_DELAY_SECONDS = 60;
const QString URL = "https://www.google-analytics.com/collect";
}
AnalyticsSender::AnalyticsSender(QObject *parent) :
    QObject(parent), _networkAccessManager(new QNetworkAccessManager(this)), _sendTimer(new QTimer(this)), _clientId(BigRingSettings().clientId())
{
    _sendTimer->setInterval(SEND_DELAY_SECONDS * 1000);
    connect(_sendTimer, &QTimer::timeout, this, &AnalyticsSender::sendAnalyticsUpdate);
    _sendTimer->start();
    QTimer::singleShot(0, this, &AnalyticsSender::sendAnalyticsUpdate);
}

void AnalyticsSender::sendAnalyticsUpdate()
{
    QNetworkRequest request(URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("v", "1");
    params.addQueryItem("tid", "UA-65087849-4");
    params.addQueryItem("cid", _clientId);
    params.addQueryItem("t", "screenview");
    params.addQueryItem("an", "Big Ring Indoor Video Cycling");
    params.addQueryItem("av", QString(APP_VERSION));
    params.addQueryItem("cd", "Main");

    const QByteArray body = params.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply *reply = _networkAccessManager->post(request, body);
    connect(reply, &QNetworkReply::finished, this, &AnalyticsSender::replyReceived);
}

void AnalyticsSender::replyReceived()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "no error. status code" << statusCode;
    } else {
        QString replyBody = QString::fromUtf8(reply->readAll().data());
        qDebug() << replyBody;
    }
    reply->deleteLater();
}
