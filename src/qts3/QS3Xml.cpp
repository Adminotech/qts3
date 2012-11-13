
#include "QS3Xml.h"
#include "QS3Defines.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDebug>

namespace QS3Xml
{
    bool parseListObjects(QS3ListObjectsResponse *response, const QByteArray &data, QString &errorMessage)
    {
        QDomDocument doc;
        if (!doc.setContent(data, &errorMessage))
            return false;
        
        // <ListBucketResult>
        QDomElement root = doc.documentElement();
        if (root.isNull())
        {
            errorMessage = "Failed to get document root element. XML response was invalid.";
            return false;
        }

        response->isTruncated = root.firstChildElement(NODE_NAME_TRUNCATED).text() == "true" ? true : false;

        QDomNodeList contents = root.elementsByTagName(NODE_NAME_CONTENTS);
        for(int i=0; i<contents.size(); ++i)
        {
            QS3Object object;

            /// @todo Set object.isDir properly, see <CommonPrefixes>

            QDomElement child = contents.item(i).firstChildElement();
            while(!child.isNull())
            {
                if (child.nodeName() == NODE_NAME_KEY)
                    object.key = child.text();
                else if (child.nodeName() == NODE_NAME_LASTMODIFIED)
                    object.lastModified = child.text();
                else if (child.nodeName() == NODE_NAME_ETAG)
                    object.eTag = child.text();
                else if (child.nodeName() == NODE_NAME_SIZE)
                    object.size = child.text().toUInt();
                child = child.nextSiblingElement();
            }

            if (!object.key.isEmpty())
            {
                if (object.key.endsWith(ROOT_PATH) && object.size == 0)
                    object.isDir = true;
                response->objects << object;
            }
        }

        return true;
    }

    bool parseAclObjects(QS3GetAclResponse *response, const QByteArray &data, QString &errorMessage)
    {
        QDomDocument doc;
        if (!doc.setContent(data, &errorMessage))
            return false;

        // <AccessControlPolicy>
        QDomElement root = doc.documentElement();
        if (root.isNull())
        {
            errorMessage = "Failed to get document root element. XML response was invalid.";
            return false;
        }

        QDomElement owner = root.firstChildElement(NODE_NAME_OWNER);
        QString bucketOwnerName = !owner.isNull() ? owner.firstChildElement(NODE_NAME_DISPLAY_NAME).text() : "";
        QString bucketOwnerId = !owner.isNull() ? owner.firstChildElement(NODE_NAME_ID).text() : "";
        if (bucketOwnerName.isEmpty() || bucketOwnerName.isEmpty())
        {
            errorMessage = "Failed to find <ID> and/or <DisplayName> from <Owner>. XML response was invalid.";
            return false;
        }

        QS3Acl acl;
        acl.ownerName = bucketOwnerName;
        acl.ownerId = bucketOwnerId;
        acl.key = response->url.path();
        if (acl.key.isEmpty())
            acl.key = ROOT_PATH;

        QDomElement accessList = root.firstChildElement(NODE_NAME_ACC_CTRL_LIST);
        if (accessList.isNull())
            return false;

        QDomNodeList child = accessList.elementsByTagName(NODE_NAME_GRANT);
        for(int i=0; i<child.size(); ++i)
        {
            QDomNode grant = child.item(i);
            
            // Read needed data from <Grantee>
            QString permissionString = grant.firstChildElement(NODE_NAME_PERMISSION).text();
            if (permissionString.isEmpty())
            {
                errorMessage = "Failed to find <Permission> from <Grant>. XML response was invalid.";
                return false;
            }
            QDomElement grantee = grant.firstChildElement(NODE_NAME_GRANTEE);
            QString granteeType = grantee.attribute(ATTRIBUTE_XSI_TYPE, GRANTEE_TYPE_USER);
            
            // CanonicalUser means this is either the bucket owner or custom user.
            if (granteeType == GRANTEE_TYPE_USER)
            {
                QString username = grantee.firstChildElement(NODE_NAME_DISPLAY_NAME).text();
                QString id = grantee.firstChildElement(NODE_NAME_ID).text();
                if (id.isEmpty() || username.isEmpty())
                {
                    errorMessage = "QS3Xml: Failed to find <ID> and/or <DisplayName> from <Grantee>. XML response was invalid.";
                    return false;
                }

                QS3AclPermissions *permissions = acl.getPermissionById(id);
                if (permissions)
                {
                    if (permissionString == ACL_FULL_CONTROL)
                        permissions->fullControl = true;
                    else if (permissionString == ACL_WRITE)
                        permissions->write = true;
                    else if (permissionString == ACL_WRITE_ACP)
                        permissions->writeACP = true;
                    else if (permissionString == ACL_READ)
                        permissions->read = true;
                    else if (permissionString == ACL_READ_ACP)
                        permissions->readACP = true;
                }
                else
                {
                    QS3AclPermissions newPermissions;
                    newPermissions.username = username;
                    newPermissions.id = id;
                    if (permissionString == ACL_FULL_CONTROL)
                        newPermissions.fullControl = true;
                    else if (permissionString == ACL_WRITE)
                        newPermissions.write = true;
                    else if (permissionString == ACL_WRITE_ACP)
                        newPermissions.writeACP = true;
                    else if (permissionString == ACL_READ)
                        newPermissions.read = true;
                    else if (permissionString == ACL_READ_ACP)
                        newPermissions.readACP = true;
                        
                    if (newPermissions.id == acl.ownerId)
                        acl.ownerUser = newPermissions;
                    else
                        acl.userPermissions << newPermissions;
                }
            }
            // Specific Amazon S3 groups
            else if (granteeType == GRANTEE_TYPE_GROUP)
            {
                /// @todo Support http://acs.amazonaws.com/groups/s3/LogDelivery group
                QString groupUri = grantee.firstChildElement(NODE_NAME_URI).text();
                if (groupUri == GROUP_URI_ALL_USERS)
                {
                    if (permissionString == ACL_FULL_CONTROL)
                        acl.allUsers.fullControl = true;
                    else if (permissionString == ACL_WRITE)
                        acl.allUsers.write = true;
                    else if (permissionString == ACL_WRITE_ACP)
                        acl.allUsers.writeACP = true;
                    else if (permissionString == ACL_READ)
                        acl.allUsers.read = true;
                    else if (permissionString == ACL_READ_ACP)
                        acl.allUsers.readACP = true;
                }
                else if (groupUri == GROUP_URI_AUTH_USERS)
                {
                    if (permissionString == ACL_FULL_CONTROL)
                        acl.authenticatedUsers.fullControl = true;
                    else if (permissionString == ACL_WRITE)
                        acl.authenticatedUsers.write = true;
                    else if (permissionString == ACL_WRITE_ACP)
                        acl.authenticatedUsers.writeACP = true;
                    else if (permissionString == ACL_READ)
                        acl.authenticatedUsers.read = true;
                    else if (permissionString == ACL_READ_ACP)
                        acl.authenticatedUsers.readACP = true;
                }
            }
        }
        
        response->acl = acl;
        return true;       
    }
}
