
#include "QS3Client.h"
#include "QS3Internal.h"
#include "QS3Xml.h"

#include <QUrl>
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

QS3GetObjectResponse *QS3Client::get(const QString &key)
{
    QNetworkRequest request(generateUrl(key));
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    QS3GetObjectResponse *response = new QS3GetObjectResponse(request.url());
    requests_[reply] = response;
    return response;
}

QS3ListObjectsResponse *QS3Client::listObjects(const QString &key, const QString &prefix, const QString &delimiter, uint maxObjects)
{
    Q3SQueryParams params;
    if (!prefix.isEmpty()) params["prefix"] = prefix;
    if (!delimiter.isEmpty()) params["delimiter"] = delimiter;
    if (maxObjects > 0) params["max-keys"] = QString::number(maxObjects);

    QNetworkRequest request(generateUrl(key, params));
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    QS3ListObjectsResponse *response = new QS3ListObjectsResponse(request.url());
    requests_[reply] = response;

    qDebug() << response->url.toString();

    return response;
}

void QS3Client::listObjectsContinue(QS3ListObjectsResponse *response)
{
    addOrReplaceQuery(&response->url, "marker", response->objects.last().key);

    QNetworkRequest request(response->url);
    prepareRequest(&request, "GET");
    QNetworkReply *reply = network_->get(request);

    qDebug() << response->url.toString();

    requests_[reply] = response;
}

void QS3Client::onReply(QNetworkReply *reply)
{
    reply->deleteLater();

    if (!requests_.contains(reply))
    {
        qDebug() << "[QS3Client]: Error: Could not map reply to S3 response object:" << reply->url().toString();
        return;
    }

    QS3Response *responseBase = requests_.take(reply);
    switch (responseBase->type)
    {
        case QS3::GetObject:
        {
            QS3GetObjectResponse *response = qobject_cast<QS3GetObjectResponse*>(responseBase);
            if (response)
                response->data = reply->readAll();
            break;
        }
        case QS3::ListObjects:
        {
            QS3ListObjectsResponse *response = qobject_cast<QS3ListObjectsResponse*>(responseBase);
            if (response)
            {
                if (QS3Xml::parseListObjects(response, reply->readAll()))
                {
                    if (response->isTruncated && !response->objects.isEmpty())
                    {
                        listObjectsContinue(response);
                        return;
                    }
                }
                else
                    responseBase->error = "Failed to parse QS3ListObjectsResponse XML response";
            }
            break;
        }
        default:
        {
            qDebug() << "[QS3Client]: Error: Could not resolve response request type:" << responseBase->type;
            break;
        }
    }

    if (responseBase->error.isEmpty())
        responseBase->emitFinished();
    else
        responseBase->emitFailed();

    responseBase->deleteLater();
}

void QS3Client::prepareRequest(QNetworkRequest *request, QString httpVerb)
{   
    QString timestamp = QS3Helpers::generateTimestamp();
    request->setRawHeader("Date", timestamp.toUtf8());

    /// @todo Support ?acl etc. http://docs.amazonwebservices.com/AmazonS3/latest/dev/RESTAuthentication.html#ConstructingTheAuthenticationHeader
    QString resource = "/" + config_.bucket + request->url().path();

    QString headers;
    foreach (QByteArray hdr, request->rawHeaderList())
        if (hdr.toLower().startsWith("x-amz-"))
            headers.append(hdr.toLower() + ":" + request->rawHeader(hdr) + "\n");

    QString data = httpVerb + QS3Helpers::newline       // HTTP-Verb
                 + QS3Helpers::newline                  // Content-MD5
                 + QS3Helpers::newline                  // Content-Type
                 + timestamp + QS3Helpers::newline      // Date
                 + headers + resource;                  // CanonicalizedAmzHeaders + CanonicalizedResource

    // Base64(HMAC-SHA1(UTF-8-Encoding-Of(StringToSign, YourSecretAccessKeyID))
    QString signature = QS3Helpers::hmacSha1(data, config_.secredKey);

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
        QList<QPair<QString, QString> > query = url->queryItems();
        for(int i=0; i<query.size(); ++i)
        {
            QPair<QString, QString> &pair = query[i];
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
    QUrl url(urlStr + key);
    if (!queryParams.isEmpty())
        foreach(QString queryKey, queryParams.keys())
            url.addQueryItem(queryKey, queryParams[queryKey]);
    return url;
}
