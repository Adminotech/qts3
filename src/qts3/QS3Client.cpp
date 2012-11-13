
#include "QS3Client.h"
#include "QS3Internal.h"
#include "QS3Xml.h"

#include <QUrl>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

QS3Client::QS3Client(const QS3Config &config, QObject *parent) :
    QObject(parent),
    config_(config),
    network_(new QNetworkAccessManager(this))
{
    connect(network_, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReply(QNetworkReply*)));
}

QS3Client::~QS3Client()
{
}

void QS3Client::setBucket(const QString &bucket)
{
    config_.bucket = bucket;
}

QString QS3Client::bucket() const
{
    return config_.bucket;
}

QS3GetObjectResponse *QS3Client::get(const QString &key)
{
    QNetworkRequest request(generateUrl(key));
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    QS3GetObjectResponse *response = new QS3GetObjectResponse(request.url());
    requests_[reply] = response;
    
    return response;
}

QS3AclResponse *QS3Client::getAcl(const QString &key)
{
    Q3SQueryParams params;
    params["acl"] = "";
    
    QNetworkRequest request(generateUrl(key, params));
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    QS3AclResponse *response = new QS3AclResponse(request.url());
    requests_[reply] = response;
    
    return response;
}

QS3ListObjectsResponse *QS3Client::listObjects(const QString &prefix, const QString &delimiter, uint maxObjects)
{
    Q3SQueryParams params;
    if (!prefix.isEmpty()) params["prefix"] = prefix;
    if (!delimiter.isEmpty()) params["delimiter"] = delimiter;
    if (maxObjects > 0) params["max-keys"] = QString::number(maxObjects);

    QNetworkRequest request(generateUrl("/", params));
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    QS3ListObjectsResponse *response = new QS3ListObjectsResponse(request.url());
    requests_[reply] = response;

    return response;
}

void QS3Client::listObjectsContinue(QS3ListObjectsResponse *response)
{
    addOrReplaceQuery(&response->url, "marker", response->objects.last().key);

    QNetworkRequest request(response->url);
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    requests_[reply] = response;
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
        emit failed(responseBase, "Network error: " + reply->errorString());
        responseBase->deleteLater();
        return;
    }
    
    bool internalError = false;
    switch (responseBase->type)
    {
        case QS3::GetObject:
        {
            QS3GetObjectResponse *response = qobject_cast<QS3GetObjectResponse*>(responseBase);
            if (response)
            {
                response->data = reply->readAll();
                emit finished(response);
            }
            else
                internalError = true;
            break;
        }
        case QS3::ListObjects:
        {
            QS3ListObjectsResponse *response = qobject_cast<QS3ListObjectsResponse*>(responseBase);
            if (response)
            {
                if (!QS3Xml::parseListObjects(response, reply->readAll()))
                {
                    emit failed(responseBase, "Failed to parse XML response");
                    responseBase->deleteLater();
                    return;
                }
                if (response->isTruncated && !response->objects.isEmpty())
                {
                    listObjectsContinue(response);
                    return;
                }
                emit finished(response);
            }
            else
                internalError = true;
            break;
        }
        case QS3::Acl:
        {
            QS3AclResponse *response = qobject_cast<QS3AclResponse*>(responseBase);
            if (response)
            {
                if (!QS3Xml::parseAclObjects(response, reply->readAll()))
                {
                    emit failed(responseBase, "Failed to parse XML response");
                    responseBase->deleteLater();
                    return;
                }
                emit finished(response);
            }
            else
                internalError = true;
            break;
        }
        default:
        {
            emit errorMessage("Could not map reply to S3 response object with " + cleanUrl);
            break;
        }
    }
    
    if (!internalError)
        responseBase->emitFinished();
    else
        emit failed(responseBase, "Fatal internal error");
    responseBase->deleteLater();
}

void QS3Client::prepareRequest(QNetworkRequest *request, QString httpVerb)
{   
    // See more from spec http://docs.amazonwebservices.com/AmazonS3/latest/dev/RESTAuthentication.html

    // Timestamp
    QString timestamp = QS3Helpers::generateTimestamp();
    request->setRawHeader("Date", timestamp.toUtf8());

    // Resource path with bucket and url path.
    QString resource = "/" + config_.bucket + request->url().path();

    // Keep special amazon header keys and their values. These and only these need to be taken into account in the signing.
    QStringList keepKeys; keepKeys << "versioning" << "location" << "acl" << "torrent" << "lifecycle" << "versionid";
    QString query = generateOrderedQuery(request->url().queryItems(), keepKeys);
    if (!query.isEmpty())
        resource += query;

    // Combine amazon headers for signing.
    QString headers;
    foreach (QByteArray hdr, request->rawHeaderList())
        if (hdr.toLower().startsWith("x-amz-"))
            headers.append(hdr.toLower() + ":" + request->rawHeader(hdr) + "\n");

    // Sign string.
    QString data = httpVerb + QS3Helpers::newline       // HTTP-Verb
                 + QS3Helpers::newline                  // Content-MD5
                 + QS3Helpers::newline                  // Content-Type
                 + timestamp + QS3Helpers::newline      // Date
                 + headers + resource;                  // CanonicalizedAmzHeaders + CanonicalizedResource

    // Returns Base64(HMAC-SHA1(UTF-8-Encoding-Of(StringToSign, YourSecretAccessKeyID))
    QString signature = QS3Helpers::hmacSha1(data, config_.secredKey);

    // Add the main authorization header to the request.
    QString authHeader;
    authHeader.append("AWS ");
    authHeader.append(config_.accessKey);
    authHeader.append(":");
    authHeader.append(signature);
    request->setRawHeader("Authorization", authHeader.toUtf8());
}

void QS3Client::addOrReplaceQuery(QUrl *url, const QString &key, const QString &value)
{
    // Set the marker
    if (url->hasQueryItem("marker"))
    {
        QS3QueryPairList query = url->queryItems();
        for(int i=0; i<query.size(); ++i)
        {
            QS3QueryPair &pair = query[i];
            if (pair.first == key)
                pair.second = value;
        }
        url->setQueryItems(query);
    }
    else
        url->addQueryItem(key, value);
}
QUrl QS3Client::generateUrl(QString key, const Q3SQueryParams &queryParams)
{
    QString urlStr = "http://" + config_.bucket;
    urlStr += (config_.host.startsWith(".") ? config_.host : "." + config_.host);
    if (!key.startsWith("/"))
        key = "/" + key;
    urlStr = QUrl(urlStr + key).toString() + generateOrderedQuery(queryParams);
    return QUrl(urlStr);
}

QString QS3Client::generateOrderedQuery(QS3QueryPairList queryItems, const QStringList &keepKeys)
{
    if (queryItems.isEmpty())
        return "";

    qSort(queryItems.begin(), queryItems.end(), QS3Helpers::QueryItemCompare);
    Q3SQueryParams queryParams;
    foreach(QS3QueryPair queryPair, queryItems)
    {
        if (keepKeys.isEmpty())
            queryParams[queryPair.first] = queryPair.second;
        else if (keepKeys.contains(queryPair.first, Qt::CaseInsensitive))
            queryParams[queryPair.first] = queryPair.second;
    }
    return generateOrderedQuery(queryParams);
}

QString QS3Client::generateOrderedQuery(const Q3SQueryParams &queryParams)
{
    if (queryParams.isEmpty())
        return "";

    QString query = "?";
    foreach(QString queryKey, queryParams.keys())
    {
        if (queryParams[queryKey].isEmpty())
            query += queryKey + "&";
        else
            query += queryKey + "=" + queryParams[queryKey] + "&";
    }
    if (query.endsWith("&"))
        query = query.left(query.length()-1);
    return query;
}