
#pragma once

#include "QS3Fwd.h"

#include <QString>
#include <QByteArray>

namespace QS3Xml
{
    bool parseError(QS3Error &dest, const QByteArray &data, QString &errorMessage);
    bool parseListObjects(QS3ListObjectsResponse *response, const QByteArray &data, QString &errorMessage);
    bool parseAclObjects(QS3GetAclResponse *response, const QByteArray &data, QString &errorMessage);

    static QString ROOT_PATH = "/";

    static QString NODE_NAME_CONTENTS       = "Contents";
    static QString NODE_NAME_KEY            = "Key";
    static QString NODE_NAME_ETAG           = "ETag";
    static QString NODE_NAME_SIZE           = "Size";
    static QString NODE_NAME_LASTMODIFIED   = "LastModified";
    static QString NODE_NAME_TRUNCATED      = "IsTruncated";
    static QString NODE_NAME_OWNER          = "Owner";
    static QString NODE_NAME_DISPLAY_NAME   = "DisplayName";
    static QString NODE_NAME_ID             = "ID";
    static QString NODE_NAME_ACC_CTRL_LIST  = "AccessControlList";
    static QString NODE_NAME_GRANT          = "Grant";
    static QString NODE_NAME_GRANTEE        = "Grantee";
    static QString NODE_NAME_PERMISSION     = "Permission";
    static QString NODE_NAME_URI            = "URI";

    static QString NODE_NAME_ERROR          = "Error";
    static QString NODE_NAME_CODE           = "Code";
    static QString NODE_NAME_MESSAGE        = "Message";
    static QString NODE_NAME_RESOURCE       = "Resource";
    static QString NODE_NAME_REQUEST_ID     = "RequestId";

    static QString GRANTEE_TYPE_USER        = "CanonicalUser";
    static QString GRANTEE_TYPE_GROUP       = "Group";

    static QString ATTRIBUTE_XSI_TYPE       = "xsi:type";

    static QString ACL_FULL_CONTROL         = "FULL_CONTROL";
    static QString ACL_WRITE                = "WRITE";
    static QString ACL_WRITE_ACP            = "WRITE_ACP";
    static QString ACL_READ                 = "READ";
    static QString ACL_READ_ACP             = "READ_ACP";

    static QString GROUP_URI_ALL_USERS      = "http://acs.amazonaws.com/groups/global/AllUsers";
    static QString GROUP_URI_AUTH_USERS     = "http://acs.amazonaws.com/groups/global/AuthenticatedUsers";
    static QString GROUP_URI_LOG_DELIVERY   = "http://acs.amazonaws.com/groups/s3/LogDelivery";
}
