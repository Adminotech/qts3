
#if defined(QTS3_LIBRARY)
#undef QTS3_LIBRARY
#endif

#include "QS3Tester.h"
#include "QS3Client.h"

#include <QCoreApplication>
#include <QDebug>

static QString AMAZON_S3_ACCESS_KEY  = "";
static QString AMAZON_S3_SECRET_KEY  = "";
static QString AMAZON_S3_BUCKET_NAME = "";

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QS3Tester tester;
    return app.exec();
}

QS3Tester::QS3Tester()
{
    if (AMAZON_S3_ACCESS_KEY.isEmpty() || AMAZON_S3_SECRET_KEY.isEmpty() || AMAZON_S3_BUCKET_NAME.isEmpty())
    {
        qDebug() << "To run the tester define static variable is QS3Tester.cpp";
        return;
    }

    client = new QS3Client(QS3Config(AMAZON_S3_ACCESS_KEY, AMAZON_S3_SECRET_KEY, AMAZON_S3_BUCKET_NAME), this);

    // List object
    QS3ListObjectsResponse *response = client->listObjects("/", "somedirpath", "", 25);
    connect(response, SIGNAL(finished(QS3ListObjectsResponse*)), SLOT(OnListObjectsRespose(QS3ListObjectsResponse*)));
}

QS3Tester::~QS3Tester()
{
}

void QS3Tester::OnListObjectsRespose(QS3ListObjectsResponse *response)
{
    qDebug() << endl << response->url.toString() << "Count:" << response->objects.size();
    foreach(QS3Object obj, response->objects)
        qDebug() << "  " << obj.toString().toStdString().c_str();
}
