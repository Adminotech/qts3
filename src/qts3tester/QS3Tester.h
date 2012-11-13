
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
    void OnAclRespose(QS3AclResponse *response);
    
    void OnErrorMessage(const QString &message);
    void OnFailed(QS3Response *response, const QString &message);

private:
    QS3Client *client;
};
