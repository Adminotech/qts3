
#include "QS3Defines.h"
#include <QDebug>

// QS3Config

QS3Config::QS3Config(const QString &accessKey_, const QString &secredKey_, const QString &bucket_, const S3EndPoint endpoint_) :
    accessKey(accessKey_),
    secredKey(secredKey_),
    bucket(bucket_),
    endpoint(endpoint_),
    host("s3.amazonaws.com")
{
    if (endpoint == US_WEST_1)
        host = "s3-us-west-1.amazonaws.com";
    else if (endpoint == US_WEST_2)
        host = "s3-us-west-2.amazonaws.com";
    else if (endpoint == SA_EAST_1)
        host = "s3.sa-east-1.amazonaws.com";
    else if (endpoint == EU_WEST_1)
        host = "s3-eu-west-1.amazonaws.com";
    else if (endpoint == AP_SOUTHEAST_1)
        host = "s3-ap-southeast-1.amazonaws.com";
    else if (endpoint == AP_SOUTHEAST_2)
        host = "s3-ap-southeast-2.amazonaws.com";
    else if (endpoint == AP_NORTHEAST_1)
        host = "s3-ap-northeast-1.amazonaws.com";

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
    endpoint = other.endpoint;
}

// QS3FileMetaData

QS3FileMetadata::QS3FileMetadata(QString contentType_, const QString &contentEncoding_) :
    contentType(contentType_),
    contentEncoding(contentEncoding_)
{
}

QS3FileMetadata::QS3FileMetadata(const QS3FileMetadata &other)
{
    contentType = other.contentType;
    contentEncoding = other.contentEncoding;
}

QS3FileMetadata::~QS3FileMetadata()
{
}

// QS3Object

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

// QS3AclPermissions

QS3AclPermissions::QS3AclPermissions() :
    fullControl(false),
    read(false),
    write(false),
    readACP(false),
    writeACP(false)
{
}

QS3AclPermissions::QS3AclPermissions(const QS3AclPermissions &other)
{
    username = other.username;
    id = other.id;
    fullControl = other.fullControl;
    read = other.read;
    write = other.write;
    readACP = other.readACP;
    writeACP = other.writeACP;
}

QS3AclPermissions::~QS3AclPermissions()
{
}

QString QS3AclPermissions::toString() const
{
    return QString("username=%1 fullControl=%2 read=%3 write=%4 readACP=%5 writeACP=%6 id[0:7]=%7").arg(username).arg(fullControl).arg(read).arg(write).arg(readACP).arg(writeACP).arg(id.left(7));
}

// QS3Acl

QS3Acl::QS3Acl()
{
    authenticatedUsers.username = "AuthenticatedUsers";
    authenticatedUsers.id = "http://acs.amazonaws.com/groups/global/AuthenticatedUsers";
    allUsers.username = "AllUsers";
    allUsers.id = "http://acs.amazonaws.com/groups/global/AllUsers";
}

QS3Acl::QS3Acl(const QS3Acl &other)
{
    key = other.key;
    ownerName = other.ownerName;
    ownerId = other.ownerId;
    authenticatedUsers = other.authenticatedUsers;
    allUsers = other.allUsers;
    userPermissions = other.userPermissions;
}

QS3Acl::~QS3Acl()
{
}

QS3AclPermissions *QS3Acl::getPermissionById(const QString &id)
{
    if (ownerUser.id == id)
        return &ownerUser;
    if (authenticatedUsers.id == id)
        return &authenticatedUsers;
    if (allUsers.id == id)
        return &allUsers;
    for(int i=0; i<userPermissions.size(); ++i)
    {
        QS3AclPermissions &userPermission = userPermissions[i];
        if (userPermission.id == id)
            return &userPermission;
    }
    return 0;
}
QS3AclPermissions *QS3Acl::getPermissionByUsername(const QString &username)
{
    if (ownerUser.username == username)
        return &ownerUser;
    if (authenticatedUsers.username == username)
        return &authenticatedUsers;
    if (allUsers.username == username)
        return &allUsers;
    for(int i=0; i<userPermissions.size(); ++i)
    {
        QS3AclPermissions &userPermission = userPermissions[i];
        if (userPermission.username == username)
            return &userPermission;
    }
    return 0;
}

QString QS3Acl::toString() const
{
    return QString("key=%1 ownerName=%2 ownerId[0:7]=%3").arg(key).arg(ownerName).arg(ownerId.left(7));
}

// QS3Error

bool QS3Error::isEmpty() const
{
    return (error.isEmpty() && code.isEmpty() && message.isEmpty());
}

QString QS3Error::toString() const
{
    // Internal or Qt network error but no S3 error.
    if (!error.isEmpty() && code.isEmpty() && message.isEmpty())
        return error;
    // S3 error present, only print it.
    else if (code.isEmpty() && message.isEmpty())
        return QString("Code: %1 Message: %2").arg(code).arg(message);
    // Print everything we have.
    return QString("%1 Code: %2 Message: %3").arg(error).arg(code).arg(message);
}

// QS3Response

QS3Response::QS3Response(const QString &key_, const QUrl &url_, QS3::RequestType type_) :
    succeeded(false),
    httpStatusCode(0),
    key(key_),
    url(url_),
    type(type_)
{
}

QS3Response::~QS3Response()
{
}

// QS3ListObjectsResponse

QS3ListObjectsResponse::QS3ListObjectsResponse(const QString &key, const QUrl &url, const QString &prefix_) :
    QS3Response(key, url, QS3::ListObjects),
    isTruncated(false),
    prefix(prefix_)
{
}

void QS3ListObjectsResponse::emitFinished()
{
    emit finished(this);
}

// QS3DeleteObjectResponse

QS3RemoveObjectResponse::QS3RemoveObjectResponse(const QString &key, const QUrl &url) :
    QS3Response(key, url, QS3::RemoveObject)
{
}

void QS3RemoveObjectResponse::emitFinished()
{
    emit finished(this);
}

// QS3CopyObjectResponse

QS3CopyObjectResponse::QS3CopyObjectResponse(const QString &key, const QUrl &url) :
    QS3Response(key, url, QS3::CopyObject)
{
}

void QS3CopyObjectResponse::emitFinished()
{
    emit finished(this);
}

// QS3GetObjectResponse

QS3GetObjectResponse::QS3GetObjectResponse(const QString &key, const QUrl &url) :
    QS3Response(key, url, QS3::GetObject)
{
}

void QS3GetObjectResponse::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(this, bytesReceived, bytesTotal);
}

void QS3GetObjectResponse::emitFinished()
{
    emit finished(this);
}

// QS3PutObjectResponse

QS3PutObjectResponse::QS3PutObjectResponse(const QString &key, const QUrl &url) :
    QS3Response(key, url, QS3::PutObject)
{
}

void QS3PutObjectResponse::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    emit uploadProgress(this, bytesSent, bytesTotal);
}

void QS3PutObjectResponse::emitFinished()
{
    emit finished(this);
}

// QS3GetAclResponse

QS3GetAclResponse::QS3GetAclResponse(const QString &key, const QUrl &url) :
    QS3Response(key, url, QS3::GetAcl)
{
}

void QS3GetAclResponse::emitFinished()
{
    emit finished(this);
}

// QS3SetAclResponse

QS3SetAclResponse::QS3SetAclResponse(const QString &key, const QUrl &url) :
    QS3Response(key, url, QS3::SetAcl)
{
}

void QS3SetAclResponse::emitFinished()
{
    emit finished(this);
}
