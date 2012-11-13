
#pragma once

#include "QS3API.h"

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QUrl>

typedef QHash<QString, QString> Q3SQueryParams;

namespace QS3
{
    enum RequestType
    {
        GetObject = 0,
        ListObjects
    };
}

class QTS3SHARED_EXPORT QS3Config
{
public:
    QString accessKey;
    QString secredKey;
    QString host;
    QString bucket;

    QS3Config(const QString &accessKey_, const QString &secredKey_, const QString &bucket_, const QString &host_ = "s3.amazonaws.com");
    QS3Config(const QS3Config &other);
    ~QS3Config();
};

// QS3Object

class QTS3SHARED_EXPORT QS3Object
{
public:
    QString key;
    QString lastModified;
    QString eTag;

    uint size;
    bool isDir;

    QS3Object();
    QS3Object(const QS3Object &other);
    ~QS3Object();

    QString toString() const;
};
typedef QList<QS3Object> QS3ObjectList;

// QSS3Response

class QTS3SHARED_EXPORT QS3Response : public QObject
{
Q_OBJECT

friend class QS3Client;

public:
    QS3Response(const QUrl &url_, QS3::RequestType type_);
    virtual ~QS3Response();

    QS3::RequestType type;
    QString error;
    QUrl url;

signals:
    void failed(const QString &error);

protected:
     virtual void emitFinished() = 0;
     void emitFailed();
};

// QS3GetObjectResponse

class QTS3SHARED_EXPORT QS3GetObjectResponse : public QS3Response
{
    Q_OBJECT

public:
    QS3GetObjectResponse(const QUrl &url);
    QByteArray data;

signals:
    void finished(QS3GetObjectResponse *response);

protected:
    void emitFinished();
};

// QSS3ListObjectsResponse

class QTS3SHARED_EXPORT QS3ListObjectsResponse : public QS3Response
{
Q_OBJECT

public:
    QS3ListObjectsResponse(const QUrl &url_);
    
    bool isTruncated;
    QS3ObjectList objects;

signals:
    void finished(QS3ListObjectsResponse *response);

protected:
    void emitFinished();
};
