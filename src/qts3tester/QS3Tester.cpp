
#include "QS3Tester.h"
#include "QS3Client.h"

#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QFile>

QS3Tester::QS3Tester(const QStringList &params)
{
    if (params.size() < 3)
    {
        qDebug() << "Usage: qts3tester <accessKey> <secretKey> <bucketName> [prefix]";
        return; 
    }
    
    client = new QS3Client(QS3Config(params[0], params[1], params[2]), this);
    connect(client, SIGNAL(finished(QS3ListObjectsResponse*)), SLOT(OnListObjectsRespose(QS3ListObjectsResponse*)));
    connect(client, SIGNAL(finished(QS3RemoveObjectResponse*)), SLOT(OnRemoveObjectRespose(QS3RemoveObjectResponse*)));
    connect(client, SIGNAL(finished(QS3GetObjectResponse*)), SLOT(OnGetObjectRespose(QS3GetObjectResponse*)));
    connect(client, SIGNAL(finished(QS3PutObjectResponse*)), SLOT(OnPutObjectRespose(QS3PutObjectResponse*)));
    connect(client, SIGNAL(finished(QS3GetAclResponse*)), SLOT(OnAclGetRespose(QS3GetAclResponse*)));
    connect(client, SIGNAL(finished(QS3SetAclResponse*)), SLOT(OnAclSetRespose(QS3SetAclResponse*)));
    connect(client, SIGNAL(failed(QS3Response*, const QString&)), SLOT(OnFailed(QS3Response*, const QString&)));
    connect(client, SIGNAL(errorMessage(const QString&)), SLOT(OnErrorMessage(const QString&)));
    
    // List object
    //client->listObjects((params.size() > 3 ? params[3] : ""));
     
    // Get object
    //client->get("avatars/aaaa.testfile");
    
    // Get and set acl
    //client->getAcl("avatars/aaaa.testfile");
    //client->setCannedAcl("avatars/aaaa.testfile", QS3::BucketOwnerFullControl);
     
    // Put object
    //QFile binary("C:/Work/admino-tundra/bin/data/assets/castle/Cube.028.mesh");
    //client->put("avatars/test.mesh", &binary, QS3FileMetadata());
    //QFile xml("C:/Work/admino-tundra/bin/data/assets/castle/CastleProject.xml");
    //client->put("avatars/test3.xml", &xml, QS3FileMetadata("text/xml"), QS3::PublicRead);
     
    // Remove object
    //client->remove("avatars/remove.file");
}

QS3Tester::~QS3Tester()
{
}

void QS3Tester::OnListObjectsRespose(QS3ListObjectsResponse *response)
{
    // Print 25 first objects on screen
    qDebug() << "[QS3Tester]: QS3ListObjectsResponse with" << response->objects.size() << "objects";
    qDebug() << "[QS3Tester]:" << response->url.toString(QUrl::RemoveQuery).toStdString().c_str();
    if (response->objects.size() > 100)
        qDebug() << "[QS3Tester]: Here are the first 25 objects:";

    for(int i=0; i<response->objects.size(); ++i)
    {
        if (i >= 25)
            break;
        qDebug() << "[QS3Tester]:  " << response->objects[i].toString().toStdString().c_str();
    }

    qDebug() << "[QS3Tester]: Requesting first 5 non directory objects and their acl:";
    uint requests = 0;
    for(int i=0; i<response->objects.size(); ++i)
    {
        if (!response->objects[i].isDir)
        {
            client->get(response->objects[i].key);
            client->getAcl(response->objects[i].key);
            
            requests++;
            if (requests == 5)
                break;
        }
    }
}

void QS3Tester::OnRemoveObjectRespose(QS3RemoveObjectResponse *response)
{
    qDebug() << "[QS3Tester]: DELETE completed " << response->url.toString(QUrl::RemoveQuery).toStdString().c_str();
    qDebug() << "[QS3Tester]: Succeeded :" << response->succeeded;
    if (!response->succeeded)
        qDebug() << "[QS3Tester]: Error     :" << response->error;
}

void QS3Tester::OnGetObjectRespose(QS3GetObjectResponse *response)
{
    qDebug() << "[QS3Tester]: GET completed" << response->url.toString(QUrl::RemoveQuery).toStdString().c_str() << QString("size=%1").arg(response->data.size()).toStdString().c_str();
    qDebug() << "[QS3Tester]: Succeeded :" << response->succeeded;
    if (!response->succeeded)
        qDebug() << "[QS3Tester]: Error     :" << response->error;
}

void QS3Tester::OnPutObjectRespose(QS3PutObjectResponse *response)
{
    qDebug() << "[QS3Tester]: PUT completed " << response->url.toString(QUrl::RemoveQuery).toStdString().c_str();
    qDebug() << "[QS3Tester]: Succeeded :" << response->succeeded;
    if (!response->succeeded)
        qDebug() << "[QS3Tester]: Error     :" << response->error;
}

void QS3Tester::OnAclGetRespose(QS3GetAclResponse *response)
{
    qDebug() << "[QS3Tester]: ACL get completed" << response->url.toString(QUrl::RemoveQuery).toStdString().c_str();
    qDebug() << "[QS3Tester]: Succeeded :" << response->succeeded;
    if (response->succeeded)
    {
        qDebug() << "[QS3Tester]:" << response->acl.toString();
        qDebug() << "[QS3Tester]:  Owner        :" << response->acl.ownerUser.toString();
        qDebug() << "[QS3Tester]:  Authenticated:" << response->acl.authenticatedUsers.toString();
        qDebug() << "[QS3Tester]:  All Users    :" << response->acl.allUsers.toString();
        foreach(QS3AclPermissions permissions, response->acl.userPermissions)
            qDebug() << "[QS3Tester]:  User:        :" << permissions.toString();
    }
    else    
        qDebug() << "[QS3Tester]: Error     :" << response->error;
}

void QS3Tester::OnAclSetRespose(QS3SetAclResponse *response)
{
    qDebug() << "[QS3Tester]: ACL set completed " << response->url.toString(QUrl::RemoveQuery).toStdString().c_str();
    qDebug() << "[QS3Tester]: Succeeded :" << response->succeeded;
    if (!response->succeeded)
        qDebug() << "[QS3Tester]: Error     :" << response->error;
}

void QS3Tester::OnFailed(QS3Response *response, const QString &message)
{
    // Already printed in individual handlers
    //qDebug() << "[QS3Tester] Error:" << response->url.toString(QUrl::RemoveQuery).toStdString().c_str() << message.toStdString().c_str();
}

void QS3Tester::OnErrorMessage(const QString &message)
{
    qDebug() << "[QS3Tester] Error:" << message.toStdString().c_str();
}

int main(int argc, char **argv)
{
    QStringList params;
    for (int i=1; i<argc; ++i)
        params << argv[i];

    QCoreApplication app(argc, argv);
    QS3Tester tester(params);
    return app.exec();
}
