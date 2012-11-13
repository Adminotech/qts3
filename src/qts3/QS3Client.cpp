
#include "QS3Client.h"
#include "QS3Internal.h"
#include "QS3Xml.h"

#include <QUrl>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDebug>
#include <QMimeData>
 
QS3Client::QS3Client(const QS3Config &config, QObject *parent) :
    QObject(parent),
    config_(config),
    network_(new QNetworkAccessManager(this))
{
    QS3::initStaticData();

    connect(network_, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReply(QNetworkReply*)));
}

QS3Client::~QS3Client()
{
    disconnect(network_, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReply(QNetworkReply*)));
    
    foreach(QNetworkReply *ongoingReply, requests_.keys())
    {
        if (!ongoingReply)
            continue;
        ongoingReply->abort();
        ongoingReply->deleteLater();
        QS3Response *ongoingResponse = requests_[ongoingReply];
        if (ongoingResponse)
            delete ongoingResponse;
    }
    requests_.clear();
}

void QS3Client::setBucket(const QString &bucket)
{
    config_.bucket = bucket;
}

QString QS3Client::bucket() const
{
    return config_.bucket;
}

QS3ListObjectsResponse *QS3Client::listObjects(const QString &prefix, const QString &delimiter, uint maxObjects)
{
    Q3SQueryParams params;
    if (!prefix.isEmpty()) params["prefix"] = prefix;
    if (!delimiter.isEmpty()) params["delimiter"] = delimiter;
    if (maxObjects > 0) params["max-keys"] = QString::number(maxObjects);

    QNetworkRequest request(generateUrl(QS3::ROOT_PATH, params));
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    QS3ListObjectsResponse *response = new QS3ListObjectsResponse(request.url());
    requests_[reply] = response;

    return response;
}

void QS3Client::listObjectsContinue(QS3ListObjectsResponse *response)
{
    QS3::addOrReplaceQuery(&response->url, "marker", response->objects.last().key);

    QNetworkRequest request(response->url);
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    requests_[reply] = response;
}

QS3RemoveObjectResponse *QS3Client::remove(const QString &key)
{
    if (key.trimmed().isEmpty() || key.trimmed() == QS3::ROOT_PATH)
    {
        qDebug() << "QS3Client::remove() Error: Cannot be called with empty or \"/\" key.";
        return 0;
    }

    QNetworkRequest request(generateUrl(key));
    prepareRequest(&request, "DELETE");
    QNetworkReply *reply = network_->deleteResource(request);

    QS3RemoveObjectResponse *response = new QS3RemoveObjectResponse(request.url());
    requests_[reply] = response;

    return response;
}

QS3GetObjectResponse *QS3Client::get(const QString &key)
{
    if (key.trimmed().isEmpty() || key.trimmed() == QS3::ROOT_PATH)
    {
        qDebug() << "QS3Client::get() Error: Cannot be called with empty or \"/\" key.";
        return 0;
    }

    QNetworkRequest request(generateUrl(key));
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    QS3GetObjectResponse *response = new QS3GetObjectResponse(request.url());
    requests_[reply] = response;
    
    return response;
}

QS3PutObjectResponse *QS3Client::put(const QString &key, QFile *file, const QS3FileMetadata &metadata, QS3::CannedAcl cannedAcl)
{
    if (key.trimmed().isEmpty() || key.trimmed() == QS3::ROOT_PATH)
    {
        qDebug() << "QS3Client::put() Error: Cannot be called with empty or \"/\" key.";
        return 0;
    }
    
    if (!file)
    {
        qDebug() << "QS3Client::put() Error: Input QFile is null.";
        return 0;
    }
    if (!file->exists())
    {
        qDebug() << "QS3Client::put() Error: Input QFile does not exist on disk:" << file->fileName();
        return 0;
    }
    if (!file->open(QIODevice::ReadOnly))
    {
        qDebug() << "QS3Client::put() Error: Input QFile could not be opened in read only mode.";
        return 0;
    }
    QByteArray data = file->readAll();
    file->close();
    
    return put(key, data, metadata, cannedAcl);
}

QS3PutObjectResponse *QS3Client::put(const QString &key, const QByteArray &data, const QS3FileMetadata &metadata, QS3::CannedAcl cannedAcl)
{
    if (key.trimmed().isEmpty() || key.trimmed() == QS3::ROOT_PATH)
    {
        qDebug() << "QS3Client::put() Error: Cannot be called with empty or \"/\" key.";
        return 0;
    }
    if (data.isEmpty())
    {
        qDebug() << "QS3Client::put() Error: Input data is empty.";
        return 0;
    }

    QByteArray aclHeader = "";
    if (cannedAcl != QS3::NoCannedAcl)
    {
        aclHeader = QS3::cannedAclToHeader(cannedAcl);
        if (aclHeader.isEmpty())
            qDebug() << "QS3Client::put() Warning: Input QS3::CannedAcl is invalid:" << cannedAcl;
    }

    // Setup headers
    QNetworkRequest request(generateUrl(key));
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.size());
    if (!metadata.contentType.isEmpty())
        request.setHeader(QNetworkRequest::ContentTypeHeader, metadata.contentType);
    if (!metadata.contentEncoding.isEmpty())
        request.setRawHeader("Content-Encoding", metadata.contentEncoding.toUtf8());
    if (!aclHeader.isEmpty())
        request.setRawHeader(QS3::AMAZON_HEADER_ACL, aclHeader);

    prepareRequest(&request, "PUT");
    QNetworkReply *reply = network_->put(request, data);

    QS3PutObjectResponse *response = new QS3PutObjectResponse(request.url());
    requests_[reply] = response;

    return response;
}

QS3GetAclResponse *QS3Client::getAcl(const QString &key)
{
    Q3SQueryParams params;
    params["acl"] = "";
    
    QNetworkRequest request(generateUrl(key, params));
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    QS3GetAclResponse *response = new QS3GetAclResponse(request.url());
    requests_[reply] = response;
    
    return response;
}

QS3SetAclResponse *QS3Client::setCannedAcl(const QString &key, QS3::CannedAcl cannedAcl)
{
    if (key.trimmed().isEmpty() || key.trimmed() == QS3::ROOT_PATH)
    {
        qDebug() << "QS3Client::setCannedAcl() Error: Cannot be called with empty or \"/\" key.";
        return 0;
    }

    QByteArray aclHeader = QS3::cannedAclToHeader(cannedAcl);
    if (aclHeader.isEmpty())
    {
        qDebug() << "QS3Client::setCannedAcl() Error: Input QS3::CannedAcl is invalid:" << cannedAcl;
        return 0;
    }

    Q3SQueryParams params;
    params["acl"] = "";

    QNetworkRequest request(generateUrl(key, params));
    request.setRawHeader(QS3::AMAZON_HEADER_ACL, aclHeader);
    prepareRequest(&request, "PUT");
    QNetworkReply *reply = network_->put(request, "");

    QS3SetAclResponse *response = new QS3SetAclResponse(request.url());
    requests_[reply] = response;

    return response;
}

void QS3Client::onReply(QNetworkReply *reply)
{
    if (!reply)
        return;
    reply->deleteLater();

    QString cleanUrl = reply->url().toString(QUrl::RemoveQuery);
    if (!requests_.contains(reply))
    {
        emit errorMessage("Could not map reply to S3 response object with " + cleanUrl);
        return;
    }

    // Remove request from internal map
    QS3Response *responseBase = requests_.take(reply);
    if (!responseBase)
    {
        emit errorMessage("Base response is null for " + cleanUrl);
        return;
    }
    if (reply->error() != QNetworkReply::NoError)
    {
        responseBase->succeeded = false;
        responseBase->error = reply->errorString();
        emit failed(responseBase, responseBase->error);
        responseBase->emitFinished();
        responseBase->deleteLater();
        return;
    }

    bool errors = false;
    bool castError = false;
    QString errorMessage = "";

    responseBase->succeeded = true;
    switch (responseBase->type)
    {
        case QS3::ListObjects:
        {
            QS3ListObjectsResponse *response = qobject_cast<QS3ListObjectsResponse*>(responseBase);
            if (response)
            {
                if (QS3Xml::parseListObjects(response, reply->readAll(), errorMessage))
                {
                    if (response->isTruncated && !response->objects.isEmpty())
                    {
                        listObjectsContinue(response);
                        return;
                    }
                    emit finished(response);
                }
                else
                    errors = true;
            }
            else
                castError = true;
            break;
        }
        case QS3::RemoveObject:
        {
            QS3RemoveObjectResponse *response = qobject_cast<QS3RemoveObjectResponse*>(responseBase);
            if (response)
                emit finished(response);
            else
                castError = true;
            break;
        }
        case QS3::GetObject:
        {
            QS3GetObjectResponse *response = qobject_cast<QS3GetObjectResponse*>(responseBase);
            if (response)
            {
                response->data = reply->readAll();
                emit finished(response);
            }
            else
                castError = true;
            break;
        }
        case QS3::PutObject:
        {
            QS3PutObjectResponse *response = qobject_cast<QS3PutObjectResponse*>(responseBase);
            if (response)
                emit finished(response);
            else
                castError = true;
            break;
        }
        case QS3::GetAcl:
        {
            QS3GetAclResponse *response = qobject_cast<QS3GetAclResponse*>(responseBase);
            if (response)
            {
                if (QS3Xml::parseAclObjects(response, reply->readAll(), errorMessage))
                    emit finished(response);
                else
                    errors = true;
            }
            else
                castError = true;
            break;
        }
        case QS3::SetAcl:
        {
            QS3SetAclResponse *response = qobject_cast<QS3SetAclResponse*>(responseBase);
            if (response)
                emit finished(response);
            else
                castError = true;
            break;
        }
        default:
        {
            errors = true;
            errorMessage = "Unknown response type " + QString::number((int)responseBase->type) + " from " + cleanUrl;
            break;
        }
    }
    
    if (castError)
    {
        errors = true;
        errorMessage = "Internal QS3Client error";
    }
    if (errors)
    {
        responseBase->succeeded = false;
        responseBase->error = errorMessage;
        emit failed(responseBase, responseBase->error);
    }

    responseBase->emitFinished();
    responseBase->deleteLater();
}

void QS3Client::prepareRequest(QNetworkRequest *request, QString httpVerb)
{   
    // See more from spec http://docs.amazonwebservices.com/AmazonS3/latest/dev/RESTAuthentication.html

    // Timestamp
    QString timestamp = QS3::generateTimestamp();
    request->setRawHeader(QS3::STANDARD_HEADER_DATE, timestamp.toUtf8());

    // Resource path with bucket and url path.
    QString resource = QS3::ROOT_PATH + config_.bucket + request->url().path();

    // Keep special amazon header keys and their values. These and only these need to be taken into account in the signing.
    QString query = QS3::generateOrderedQuery(request->url().queryItems(), QS3::AMAZON_QUERY_KEYS);
    if (!query.isEmpty())
        resource += query;

    // Combine amazon headers for signing.
    QString headers;
    foreach (QByteArray hdr, request->rawHeaderList())
        if (hdr.toLower().startsWith(QS3::AMAZON_HEADER_PREFIX))
            headers.append(hdr.toLower() + ":" + request->rawHeader(hdr) + QS3::NEWLINE);
            
    QVariant contentType = request->header(QNetworkRequest::ContentTypeHeader);

    // Sign string.
    QString data = httpVerb + QS3::NEWLINE       // HTTP-Verb
                 + QS3::NEWLINE                  // Content-MD5
                 + (!contentType.isNull() ? contentType.toString() : "") + QS3::NEWLINE  // Content-Type
                 + timestamp + QS3::NEWLINE      // Date
                 + headers + resource;                  // CanonicalizedAmzHeaders + CanonicalizedResource

    // Returns Base64(HMAC-SHA1(UTF-8-Encoding-Of(StringToSign, YourSecretAccessKeyID))
    QString signature = QS3::hmacSha1(data, config_.secredKey);

    // Add the main authorization header to the request.
    QString authHeader = "AWS " + config_.accessKey + ":" + signature;
    request->setRawHeader(QS3::STANDARD_HEADER_AUTHORIZATION, authHeader.toUtf8());
}

QUrl QS3Client::generateUrl(QString key, const Q3SQueryParams &queryParams)
{
    QString urlStr = "http://" + config_.bucket;
    urlStr += (config_.host.startsWith(".") ? config_.host : "." + config_.host);
    if (!key.startsWith(QS3::ROOT_PATH))
        key = QS3::ROOT_PATH + key;
    urlStr = QUrl(urlStr + key).toString() + QS3::generateOrderedQuery(queryParams);
    return QUrl(urlStr);
}
