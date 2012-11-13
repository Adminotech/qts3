
#pragma once

#include "QS3Fwd.h"

#include <QByteArray>

struct QS3Xml
{
    static bool parseListObjects(QS3ListObjectsResponse *response, const QByteArray &data);
};