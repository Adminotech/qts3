
#include "QS3Defines.h"

// QS3Config

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

// QS3Response

QS3Response::QS3Response(const QUrl &url_, QS3::RequestType type_) :
    url(url_),
    type(type_),
    succeeded(false)
{
}

QS3Response::~QS3Response()
{
}

// QS3ListObjectsResponse

QS3ListObjectsResponse::QS3ListObjectsResponse(const QUrl &url) :
    QS3Response(url, QS3::ListObjects),
    isTruncated(false)
{
}

void QS3ListObjectsResponse::emitFinished()
{
    emit finished(this);
}

// QS3DeleteObjectResponse

QS3RemoveObjectResponse::QS3RemoveObjectResponse(const QUrl &url) :
    QS3Response(url, QS3::RemoveObject)
{
}

void QS3RemoveObjectResponse::emitFinished()
{
    emit finished(this);
}

// QS3CopyObjectResponse

QS3CopyObjectResponse::QS3CopyObjectResponse(const QUrl &url) :
    QS3Response(url, QS3::CopyObject)
{
}

void QS3CopyObjectResponse::emitFinished()
{
    emit finished(this);
}

// QS3GetObjectResponse

QS3GetObjectResponse::QS3GetObjectResponse(const QUrl &url) :
    QS3Response(url, QS3::GetObject)
{
}

void QS3GetObjectResponse::emitFinished()
{
    emit finished(this);
}

// QS3PutObjectResponse

QS3PutObjectResponse::QS3PutObjectResponse(const QUrl &url) :
    QS3Response(url, QS3::PutObject)
{

}

void QS3PutObjectResponse::emitFinished()
{
    emit finished(this);
}

// QS3GetAclResponse

QS3GetAclResponse::QS3GetAclResponse(const QUrl &url) :
    QS3Response(url, QS3::GetAcl)
{
}

void QS3GetAclResponse::emitFinished()
{
    emit finished(this);
}

// QS3SetAclResponse

QS3SetAclResponse::QS3SetAclResponse(const QUrl &url) :
    QS3Response(url, QS3::SetAcl)
{
}

void QS3SetAclResponse::emitFinished()
{
    emit finished(this);
}
