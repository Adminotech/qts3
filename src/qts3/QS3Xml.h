
#pragma once

#include "QS3Fwd.h"

#include <QByteArray>

struct QS3Xml
{
    static bool parseListObjects(QS3ListObjectsResponse *response, const QByteArray &data);
    static bool parseAclObjects(QS3GetAclResponse *response, const QByteArray &data);
};