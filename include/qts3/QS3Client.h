
#pragma once

#include "QS3API.h"
#include "QS3Fwd.h"
#include "QS3Defines.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QUrl>

/** QS3Client provides access to Amazon S3 file storage.
   
    There are two ways of connecting to responses, the model
    resembles that of QNetworkAccessManager.

    1) Connect to each response objects finished signal
       and the error signals in QS3Client. 
    2) Connect to the QS3Client finished signals, one for each type
       of response and the error signals in QS3Client.
*/

class QTS3SHARED_EXPORT QS3Client : public QObject
{
Q_OBJECT

public:
    explicit QS3Client(const QS3Config &config, QObject *parent = 0);
    ~QS3Client();

public slots:
    /// List bucket objects.
    /** @param QString prefix for the request.
        @param QString delimiter for the request.
        @param uint maximum objects to return with single response.
        @return QS3ListObjectsResponse response object. 
        @note Maximum object mean how many objects are fetched per request. 
        If you want to control the total amount of returned objects use 
        prefix and delimiter for filtering. */
    QS3ListObjectsResponse *listObjects(const QString &prefix = "", const QString &delimiter = "", uint maxObjects = 1000);

    /// Remove object with key.
    /** @param QString key aka path in the bucket.
        @return QS3DeleteObjectResponse response object. 
        @note If the target is a folder it needs to be empty for it to be removed.
        @todo Automate the above so each file is removed from the folder automatically. */
    QS3RemoveObjectResponse *remove(const QString &key);

    /// Copy object with keys within the same bucket.
    /** @param QString source key. This object will be copied. Do not include source bucket in the key.
        @param QString target key. This is where the object will be copied to. Do not include target bucket in the key.
        @param QS3::CannedAcl Applied canned ACL to uploaded file. By default QS3::BucketOwnerFullControl is used.
        @return QS3CopyObjectResponse response object. */
    QS3CopyObjectResponse *copy(const QString &sourceKey, const QString &destinationKey, QS3::CannedAcl cannedAcl = QS3::BucketOwnerFullControl);

    /// Copy object with keys from bucket to another.
    /** @param QString source bucket name. The source key must be in this bucket.
        @param QString source key. This object will be copied. Do not include source bucket in the key.
        @param QString target key. This is where the object will be copied to. Do not include target bucket in the key.
        @param QS3::CannedAcl Applied canned ACL to uploaded file. By default QS3::BucketOwnerFullControl is used.
        @return QS3CopyObjectResponse response object. */
    QS3CopyObjectResponse *copy(const QString &sourceBucket, const QString &sourceKey, const QString &destinationKey, QS3::CannedAcl cannedAcl = QS3::BucketOwnerFullControl);

    /// Get object with key.
    /** @param QString key aka path in the bucket.
        @return QS3GetObjectResponse response object. */
    QS3GetObjectResponse *get(const QString &key);
    
    /// Put new object to bucket with key and file.
    /** @param QString key to upload.
        @param QFile file to upload.
        @param QS3FileMetadata File metadata.
        @param QS3::CannedAcl Applied canned ACL to uploaded file. By default QS3::BucketOwnerFullControl is used.
        @return QS3PutObjectResponse response object.
        @note Returned response can be null if invalid input params were given. */
    QS3PutObjectResponse *put(const QString &key, QFile *file, const QS3FileMetadata &metadata, QS3::CannedAcl cannedAcl = QS3::BucketOwnerFullControl);
    
    /// Put new object to bucket with key and file data.
    /** @param QString key to upload.
        @param QByteArray data to upload.
        @param QS3FileMetadata Metadata.
        @param QS3::CannedAcl Applied canned ACL to uploaded file. By default QS3::BucketOwnerFullControl is used.
        @return QS3PutObjectResponse response object.
        @note Returned response can be null if invalid input params were given. */
    QS3PutObjectResponse *put(const QString &key, const QByteArray &data, const QS3FileMetadata &metadata, QS3::CannedAcl cannedAcl = QS3::BucketOwnerFullControl);
    
    /// Create new folder to storage. 
    /** You must be sure this folder does not already exist. If it exists the resulted behaviour is undefined.
        @param QString key. Folder path, must end with "/" or will be rejected. 
        @param QS3::CannedAcl Applied canned ACL to the created folder. By default QS3::BucketOwnerFullControl is used. 
        @return QS3PutObjectResponse response object.
        @note Amazon S3 will automatically create folders if you upload a file that has non-existing folders in the key.
        You don't have to use this to ensure the full folder path exists.
        @note Returned response can be null if invalid input params were given. */
    QS3PutObjectResponse *createFolder(const QString &key, QS3::CannedAcl cannedAcl = QS3::BucketOwnerFullControl);
    
    /// Get object ACL for key.
    /** @param QString key.
        @return QS3GetAclResponse response object. 
        @note Default key value is "/" that is bucket root.*/
    QS3GetAclResponse *getAcl(const QString &key = "/");

    // Set object ACL for key.
    /** @param QString key. Key cannot be "/" that is bucket root.
        @param QS3::CannedAcl canned acl. Beware that this resets 
        the current acl state and set it as cannedAcl.
        @return QS3SetAclResponse response object.
        @note Returned response can be null if invalid input params were given. */
    QS3SetAclResponse *setCannedAcl(const QString &key, QS3::CannedAcl cannedAcl);
    
    /// Sets a new bucket name for future requests.
    /** @param QString bucket name */
    void setBucket(const QString &bucket);
    
    /// Returns the current bucket name.
    /** @return QString bucket name. */
    QString bucket() const;

signals:
    /// QS3ListObjectsResponse has finished.
    /** @note Do not store the emitted pointer. It will be automatically destroyed. */
    void finished(QS3ListObjectsResponse *response);

    /// QS3RemoveObjectResponse has finished.
    /** @note Do not store the emitted pointer. It will be automatically destroyed. */
    void finished(QS3RemoveObjectResponse *response);

    /// QS3CopyObjectResponse has finished.
    /** @note Do not store the emitted pointer. It will be automatically destroyed. */
    void finished(QS3CopyObjectResponse *response);
    
    /// QS3GetObjectResponse has finished.
    /** @note Do not store the emitted pointer. It will be automatically destroyed. */
    void finished(QS3GetObjectResponse *response);

    /// QS3PutAclResponse has finished. 
    /** @note Do not store the emitted pointer. It will be automatically destroyed. */
    void finished(QS3PutObjectResponse *response);
        
    /// QS3GetAclResponse has finished. 
    /** @note Do not store the emitted pointer. It will be automatically destroyed. */
    void finished(QS3GetAclResponse *response);
    
    /// QS3SetAclResponse has finished. 
    /** @note Do not store the emitted pointer. It will be automatically destroyed. */
    void finished(QS3SetAclResponse *response);
    
    /// Response failed with message.
    /** @note Do not store the emitted pointer. It will be automatically destroyed. */
    void failed(QS3Response *response, const QString &message);
    
    /// Internal errors in the case of fatal failure eg. could not internally map request to response.
    void errorMessage(const QString &message);
        
private slots:
    /// Private handler for internal Amazon replies.
    void onReply(QNetworkReply *reply);

private:
    /// Continues a list object request with current marker.
    void listObjectsContinue(QS3ListObjectsResponse *response);

    /// Executes the amazon Authorization header signing.
    /** @note Set any "x-amz-" headers before calling this functions. */
    void prepareRequest(QNetworkRequest *request, QString httpVerb);
    
    /// Generates proper url with query parameters.
    QS3UrlPair generateUrl(QString key, const Q3SQueryParams &queryParams = Q3SQueryParams());
    
    QS3Config config_;
    QNetworkAccessManager *network_;
    QHash<QNetworkReply*, QS3Response*> requests_;
};

