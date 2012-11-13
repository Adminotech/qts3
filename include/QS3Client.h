
#pragma once

#include "QS3API.h"
#include "QS3Fwd.h"
#include "QS3Defines.h"

#include <QObject>
#include <QString>
#include <QUrl>

class QTS3SHARED_EXPORT QS3Client : public QObject
{
Q_OBJECT

public:
    QS3Client(const QS3Config &config, QObject *parent = 0);
    ~QS3Client();

public slots:
    QS3GetObjectResponse *get(const QString &key);
    QS3ListObjectsResponse *listObjects(const QString &key, const QString &prefix = QString(), const QString &delimiter = QString(), uint maxObjects = 1000);

private slots:
    void onReply(QNetworkReply *reply);

private:
    void listObjectsContinue(QS3ListObjectsResponse *response);

    void prepareRequest(QNetworkRequest *request, QString httpVerb = "GET");
    void addOrReplaceQuery(QUrl *url, const QString &key, const QString &value);
    QUrl generateUrl(QString key, const Q3SQueryParams &queryParams = Q3SQueryParams());

    QS3Config config_;
    QNetworkAccessManager *network_;
    QHash<QNetworkReply*, QS3Response*> requests_;
};

