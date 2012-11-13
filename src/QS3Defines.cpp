
#include "QS3Defines.h"

QS3Config::QS3Config(const QString &accessKey_, const QString &secredKey_, const QString &bucket_, const QString &host_) :
    accessKey(accessKey_),
    secredKey(secredKey_),
    host(host_),
    bucket(bucket_)
{
    if (bucket.startsWith("/"))
        bucket = bucket.right(bucket.length()-1);
}

QS3Config::~QS3Config()
{
}

QS3Config::QS3Config(const QS3Config &other)
{
    accessKey = other.accessKey;
    secredKey = other.secredKey;
    host = other.host;
    bucket = other.bucket;
}

QS3Object::QS3Object() :
    size(0),
    isDir(false)
{
}

QS3Object::~QS3Object()
{
}

QS3Object::QS3Object(const QS3Object &other)
{
    key = other.key;
    lastModified = other.lastModified;
    eTag = other.eTag;
    size = other.size;
    isDir = other.isDir;
}

QString QS3Object::toString() const
{
    return QString("key=%1 size=%2").arg(key).arg(size);
}

QS3Response::QS3Response(const QUrl &url_, QS3::RequestType type_) :
    url(url_),
    type(type_)
{
}

QS3Response::~QS3Response()
{
}

void QS3Response::emitFailed()
{
    emit failed(error);
}

QS3GetObjectResponse::QS3GetObjectResponse(const QUrl &url) :
    QS3Response(url, QS3::GetObject)
{
}

void QS3GetObjectResponse::emitFinished()
{
    emit finished(this);
}

QS3ListObjectsResponse::QS3ListObjectsResponse(const QUrl &url) :
    QS3Response(url, QS3::ListObjects),
    isTruncated(false)
{
}

void QS3ListObjectsResponse::emitFinished()
{
    emit finished(this);
}
