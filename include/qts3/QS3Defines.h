
#pragma once

#include "QS3API.h"

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QHash>
#include <QPair>

typedef QHash<QString, QString> Q3SQueryParams;
typedef QPair<QString, QString> QS3QueryPair;
typedef QList<QS3QueryPair > QS3QueryPairList;
typedef QPair<QString, QUrl> QS3UrlPair;

namespace QS3
{
    enum RequestType
    {
        ListObjects = 0,
        RemoveObject,
        CopyObject,
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

/// QS3Config

class QTS3SHARED_EXPORT QS3Config
{
public:
    enum S3EndPoint
    {
        S3_DEFAULT,         // s3.amazonaws.com
        US_WEST_1,          // s3-us-west-1.amazonaws.com
        US_WEST_2,          // s3-us-west-2.amazonaws.com
        SA_EAST_1,          // s3.sa-east-1.amazonaws.com
        EU_WEST_1,          // s3-eu-west-1.amazonaws.com
        AP_SOUTHEAST_1,     // s3-ap-southeast-1.amazonaws.com
        AP_SOUTHEAST_2,     // s3-ap-southeast-2.amazonaws.com
        AP_NORTHEAST_1,     // s3-ap-northeast-1.amazonaws.com
    };
    
    QString accessKey;
    QString secredKey;
    QString host;
    QString bucket;
    S3EndPoint endpoint;

    QS3Config(const QString &accessKey_, const QString &secredKey_, const QString &bucket_, S3EndPoint endpoint_);
    QS3Config(const QS3Config &other);
    ~QS3Config();
};

/// QS3FileMetaData

class QTS3SHARED_EXPORT QS3FileMetadata
{
public:
    QString contentType;
    QString contentEncoding;
    
    QS3FileMetadata(const QString contentType_ = "binary/octet-stream", const QString &contentEncoding_ = "application/octet-stream");
    QS3FileMetadata(const QS3FileMetadata &other);
    ~QS3FileMetadata();
};

/// QS3Object

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

/// QS3AclPermissions

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

/// QS3Acl

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

/// QS3Error
/** Collects data from the Amazon S3 error response, see more from
	http://docs.aws.amazon.com/AmazonS3/latest/API/ErrorResponses.html */
class QTS3SHARED_EXPORT QS3Error
{
public:
	/// S3 error spec
	QString code;
	QString message;
	QString requestId;
	QString resource;

	/// Generic internal or Qt network error.
	QString error;

	/// Returns true if there is no error.
	bool isEmpty() const;

	/// Returns formatted printable string for error code and message.
	QString toString() const;
};

/// QS3Response

class QTS3SHARED_EXPORT QS3Response : public QObject
{
Q_OBJECT

friend class QS3Client;

public:
    QS3Response(const QString &key_, const QUrl &url_, QS3::RequestType type_);
    virtual ~QS3Response();

    /// Did request completed succesfully.
    bool succeeded;
    
	/// S3 error object. For network error see QS3Error::networkError.
	QS3Error error;
    
    /// Request Amason S3 object key.
    QString key;
    
    /// Request full url. Includes all query parameters.
    QUrl url;
    
    /// HTTP response status code.
    int httpStatusCode;
    
    /// Type of the request.
    QS3::RequestType type;

protected:
    /// Each inheriting class needs to implement this function
    /// and emit its finished/failed signal.
    virtual void emitFinished() = 0;
};

/// QS3ListObjectsResponse

class QTS3SHARED_EXPORT QS3ListObjectsResponse : public QS3Response
{
Q_OBJECT

public:
    QS3ListObjectsResponse(const QString &key, const QUrl &url, const QString &prefix_);
    
    bool isTruncated;
    QString prefix;
    QS3ObjectList objects;

signals:
    /// Request response finished.
    /** This signal will fire if the request succeeded and
        if it fails. Check succeeded and error members for the status. */
    void finished(QS3ListObjectsResponse *response);

protected:
    void emitFinished();
};

/// QS3RemoveObjectResponse

class QTS3SHARED_EXPORT QS3RemoveObjectResponse : public QS3Response
{
Q_OBJECT

public:
    QS3RemoveObjectResponse(const QString &key, const QUrl &url);

signals:
    /// Request response finished.
    /** This signal will fire if the request succeeded and
        if it fails. Check succeeded and error members for the status. */
    void finished(QS3RemoveObjectResponse *response);

protected:
    void emitFinished();
};

/// QS3CopyObjectResponse

class QTS3SHARED_EXPORT QS3CopyObjectResponse : public QS3Response
{
Q_OBJECT

public:
    QS3CopyObjectResponse(const QString &key, const QUrl &url);

signals:
    /// Request response finished.
    /** This signal will fire if the request succeeded and
        if it fails. Check succeeded and error members for the status. */
    void finished(QS3CopyObjectResponse *response);

protected:
    void emitFinished();
};

/// QS3GetObjectResponse

class QTS3SHARED_EXPORT QS3GetObjectResponse : public QS3Response
{
Q_OBJECT

public:
    QS3GetObjectResponse(const QString &key, const QUrl &url);
    QByteArray data;

signals:
    /// Request response finished.
    /** This signal will fire if the request succeeded and
        if it fails. Check succeeded and error members for the status. */
    void finished(QS3GetObjectResponse *response);

    /// Reports the download progress. 
    /** @note bytesTotal may be -1 except when the upload finishes. */
    void downloadProgress(QS3GetObjectResponse *response, qint64 bytesReceived, qint64 bytesTotal);

private slots:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    
protected:
    void emitFinished();
};

/// QS3PutObjectResponse

class QTS3SHARED_EXPORT QS3PutObjectResponse : public QS3Response
{
Q_OBJECT

public:
    QS3PutObjectResponse(const QString &key, const QUrl &url);
    
signals:
    /// Request response finished.
    /** This signal will fire if the request succeeded and
        if it fails. Check succeeded and error members for the status. */
    void finished(QS3PutObjectResponse *response);
    
    /// Reports the upload progress. 
    /** @note bytesTotal may be -1 except when the upload finishes. */
    void uploadProgress(QS3PutObjectResponse *response, qint64 bytesSent, qint64 bytesTotal);

private slots:
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    
protected:
    void emitFinished();
};

/// QS3AclResponse

class QTS3SHARED_EXPORT QS3GetAclResponse : public QS3Response
{
Q_OBJECT

public:
    QS3GetAclResponse(const QString &key, const QUrl &url);
    QS3Acl acl;

signals:
    /// Request response finished.
    /** This signal will fire if the request succeeded and
        if it fails. Check succeeded and error members for the status. */
    void finished(QS3GetAclResponse *response);

protected:
    void emitFinished();
};

/// QS3SetAclResponse

class QTS3SHARED_EXPORT QS3SetAclResponse : public QS3Response
{
Q_OBJECT

public:
    QS3SetAclResponse(const QString &key, const QUrl &url);

signals:
    /// Request response finished.
    /** This signal will fire if the request succeeded and
        if it fails. Check succeeded and error members for the status. */
    void finished(QS3SetAclResponse *response);

protected:
    void emitFinished();
};
