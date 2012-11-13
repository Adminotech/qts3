
#pragma once

#include "QS3API.h"
#include "QS3Fwd.h"
#include "QS3Defines.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

class QTS3SHARED_EXPORT QS3Client : public QObject
{
Q_OBJECT

public:
    explicit QS3Client(const QS3Config &config, QObject *parent = 0);
    ~QS3Client();

public slots:
    /// Get key.
    /** @param QString key aka path in the bucket.
        @return QS3GetObjectResponse response object. */
    QS3GetObjectResponse *get(const QString &key);
    
    /// Get acl for key.
    /** @param QString key. Default value is "/" that is bucket root.
        @return QS3AclResponse response object. */
    QS3AclResponse *getAcl(const QString &key = "/");
    
    /// List bucket objects.
    /** @param QString prefix for the request.
        @param QString delimiter for the request.
        @param uint maximum objects to return with single response.
        @return QS3ListObjectsResponse response object. 
        @note Maximum object mean how many objects are fetched per request. 
        If you want to control the total amount of returned objects use 
        prefix and delimiter for filtering. */
    QS3ListObjectsResponse *listObjects(const QString &prefix = "", const QString &delimiter = "", uint maxObjects = 1000);

    /// Sets a new bucket name for future requests.
    /** @param QString bucket name */
    void setBucket(const QString &bucket);
    
    /// Returns the current bucket name.
    /** @return QString bucket name. */
    QString bucket() const;

signals:
    /// QS3GetObjectResponse has finished.
    void finished(QS3GetObjectResponse *response);
    
    /// QS3ListObjectsResponse has finished.
    void finished(QS3ListObjectsResponse *response);
    
    /// QS3AclResponse has finished. 
    void finished(QS3AclResponse *response);
    
    /// Response failed with message.
    void failed(QS3Response *response, const QString &message);
    
    /// Internal errors in the case of fatal failure 
    /// eg. could not internally map request to response.
    void errorMessage(const QString &message);
        
private slots:
    void onReply(QNetworkReply *reply);

private:
    void listObjectsContinue(QS3ListObjectsResponse *response);

    void prepareRequest(QNetworkRequest *request, QString httpVerb = "GET");
    void addOrReplaceQuery(QUrl *url, const QString &key, const QString &value);
    QUrl generateUrl(QString key, const Q3SQueryParams &queryParams = Q3SQueryParams());
    
    QString generateOrderedQuery(QS3QueryPairList queryItems, const QStringList &keepKeys = QStringList());
    QString generateOrderedQuery(const Q3SQueryParams &queryItems);

    QS3Config config_;
    QNetworkAccessManager *network_;
    QHash<QNetworkReply*, QS3Response*> requests_;
};

