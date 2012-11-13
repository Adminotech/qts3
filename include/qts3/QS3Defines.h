
#pragma once

#include "QS3API.h"

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QHash>

typedef QHash<QString, QString> Q3SQueryParams;
typedef QPair<QString, QString> QS3QueryPair;
typedef QList<QS3QueryPair > QS3QueryPairList;

namespace QS3
{
    enum RequestType
    {
        ListObjects = 0,
        GetObject,
        PutObject,
        GetAcl,
        SetAcl
    };
    
    enum CannedAcl
    {
        NoCannedAcl = 0,
        Private,
        PublicRead,
        PublicReadWrite,
        AuthenticatedRead,
        BucketOwnerRead,
        BucketOwnerFullControl
    };
}

// QS3Config

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

// QS3FileMetaData

class QTS3SHARED_EXPORT QS3FileMetadata
{
public:
    QString contentType;
    QString contentEncoding;
    
    QS3FileMetadata(const QString contentType_ = "binary/octet-stream", const QString &contentEncoding_ = "application/octet-stream");
    QS3FileMetadata(const QS3FileMetadata &other);
    ~QS3FileMetadata();
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

// QS3AclPermissions

class QTS3SHARED_EXPORT QS3AclPermissions
{
public:
    QString username;
    QString id;
    bool fullControl;
    bool read;
    bool write;
    bool readACP;
    bool writeACP;
    
    QS3AclPermissions();
    QS3AclPermissions(const QS3AclPermissions &other);
    ~QS3AclPermissions();
    
    QString toString() const;
};

// QS3Acl

class QTS3SHARED_EXPORT QS3Acl
{
public:
    QString key;
    QString ownerName;
    QString ownerId;
   
    QS3AclPermissions ownerUser;
    QS3AclPermissions authenticatedUsers;
    QS3AclPermissions allUsers;
    QList<QS3AclPermissions> userPermissions;
    
    QS3AclPermissions *getPermissionById(const QString &id);
    QS3AclPermissions *getPermissionByUsername(const QString &username);
    
    QS3Acl();
    QS3Acl(const QS3Acl &other);
    ~QS3Acl();

    QString toString() const;
};

// QSS3Response

class QTS3SHARED_EXPORT QS3Response : public QObject
{
Q_OBJECT

friend class QS3Client;

public:
    QS3Response(const QUrl &url_, QS3::RequestType type_);
    virtual ~QS3Response();

    QS3::RequestType type;
    QUrl url;

protected:
     virtual void emitFinished() = 0;
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

// QS3PutObjectResponse

class QTS3SHARED_EXPORT QS3PutObjectResponse : public QS3Response
{
Q_OBJECT

public:
    QS3PutObjectResponse(const QUrl &url);
    bool succeeded;
    
signals:
    void finished(QS3PutObjectResponse *response);

protected:
    void emitFinished();
};

// QSS3ListObjectsResponse

class QTS3SHARED_EXPORT QS3ListObjectsResponse : public QS3Response
{
Q_OBJECT

public:
    QS3ListObjectsResponse(const QUrl &url);
    bool isTruncated;
    QS3ObjectList objects;

signals:
    void finished(QS3ListObjectsResponse *response);

protected:
    void emitFinished();
};

// QS3AclResponse

class QTS3SHARED_EXPORT QS3GetAclResponse : public QS3Response
{
Q_OBJECT

public:
    QS3GetAclResponse(const QUrl &url);
    QS3Acl acl;

signals:
    void finished(QS3GetAclResponse *response);

protected:
    void emitFinished();
};

// QS3SetAclResponse

class QTS3SHARED_EXPORT QS3SetAclResponse : public QS3Response
{
Q_OBJECT

public:
    QS3SetAclResponse(const QUrl &url);
    bool succeeded;

signals:
    void finished(QS3SetAclResponse *response);

protected:
    void emitFinished();
};
