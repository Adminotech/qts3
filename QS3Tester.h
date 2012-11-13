
#pragma once

#include "QS3Fwd.h"
#include <QObject>

class QS3Tester : public QObject
{
Q_OBJECT

public:
    QS3Tester();
    ~QS3Tester();

private slots:
    void OnListObjectsRespose(QS3ListObjectsResponse *response);

private:
    QS3Client *client;
};
