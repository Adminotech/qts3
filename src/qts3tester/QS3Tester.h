
#pragma once

#include "QS3Fwd.h"
#include <QObject>

class QS3Tester : public QObject
{
Q_OBJECT

public:
    QS3Tester(const QStringList &params);
    ~QS3Tester();

private slots:
    void OnListObjectsRespose(QS3ListObjectsResponse *response);
    void OnGetObjectRespose(QS3GetObjectResponse *response);
    void OnPutObjectRespose(QS3PutObjectResponse *response);
    void OnAclGetRespose(QS3GetAclResponse *response);
    void OnAclSetRespose(QS3SetAclResponse *response);
    
    void OnErrorMessage(const QString &message);
    void OnFailed(QS3Response *response, const QString &message);

private:
    QS3Client *client;
};
