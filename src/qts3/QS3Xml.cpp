
#include "QS3Xml.h"
#include "QS3Defines.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDebug>

bool QS3Xml::parseListObjects(QS3ListObjectsResponse *response, const QByteArray &data)
{
    QDomDocument doc;
    if (!doc.setContent(data))
        return false;
    
    // <ListBucketResult>
    QDomElement root = doc.documentElement();
    if (root.isNull())
        return false;

    response->isTruncated = root.firstChildElement("IsTruncated").text() == "true" ? true : false;

    QDomNodeList contents = root.elementsByTagName("Contents");
    for(int i=0; i<contents.size(); ++i)
    {
        QS3Object object;

        /// @todo Set object.isDir properly, see <CommonPrefixes>

        QDomElement child = contents.item(i).firstChildElement();
        while(!child.isNull())
        {
            if (child.nodeName() == "Key")
                object.key = child.text();
            else if (child.nodeName() == "LastModified")
                object.lastModified = child.text();
            else if (child.nodeName() == "ETag")
                object.eTag = child.text();
            else if (child.nodeName() == "Size")
                object.size = child.text().toUInt();
            child = child.nextSiblingElement();
        }

        if (!object.key.isEmpty())
        {
            if (object.key.endsWith("/") && object.size == 0)
                object.isDir = true;
            response->objects << object;
        }
    }

    return true;
}

bool QS3Xml::parseAclObjects(QS3GetAclResponse *response, const QByteArray &data)
{
    QDomDocument doc;
    if (!doc.setContent(data))
        return false;

    // <AccessControlPolicy>
    QDomElement root = doc.documentElement();
    if (root.isNull())
        return false;

    QDomElement owner = root.firstChildElement("Owner");
    QString bucketOwnerName = !owner.isNull() ? owner.firstChildElement("DisplayName").text() : "";
    QString bucketOwnerId = !owner.isNull() ? owner.firstChildElement("ID").text() : "";
    if (bucketOwnerName.isEmpty() || bucketOwnerName.isEmpty())
    {
        qDebug() << "QS3Xml: Failed to find <ID> and/or <DisplayName> from <Owner>";
        return false;
    }

    QS3Acl acl;
    acl.ownerName = bucketOwnerName;
    acl.ownerId = bucketOwnerId;
    acl.key = response->url.path();
    if (acl.key.isEmpty())
        acl.key = "/";

    QDomElement accessList = root.firstChildElement("AccessControlList");
    if (accessList.isNull())
        return false;

    QDomNodeList child = accessList.elementsByTagName("Grant");
    for(int i=0; i<child.size(); ++i)
    {
        QDomNode grant = child.item(i);
        
        // Read needed data from <Grantee>
        QString permissionString = grant.firstChildElement("Permission").text();
        if (permissionString.isEmpty())
        {
            qDebug() << "QS3Xml: Failed to find <Permission> from <Grant>";
            continue;
        }
        QDomElement grantee = grant.firstChildElement("Grantee");
        QString granteeType = grantee.attribute("xsi:type", "CanonicalUser");
        
        // CanonicalUser means this is either the bucket owner or custom user.
        if (granteeType == "CanonicalUser")
        {
            QString username = grantee.firstChildElement("DisplayName").text();
            QString id = grantee.firstChildElement("ID").text();
            if (granteeType == "CanonicalUser" && (id.isEmpty() || username.isEmpty()))
            {
                qDebug() << "QS3Xml: Failed to find <ID> and/or <DisplayName> from <Grantee>";
                continue;
            }

            QS3AclPermissions *permissions = acl.getPermissionById(id);
            if (permissions)
            {
                if (permissionString == "FULL_CONTROL")
                    permissions->fullControl = true;
                else if (permissionString == "WRITE")
                    permissions->write = true;
                else if (permissionString == "WRITE_ACP")
                    permissions->writeACP = true;
                else if (permissionString == "READ")
                    permissions->read = true;
                else if (permissionString == "READ_ACP")
                    permissions->readACP = true;
            }
            else
            {
                QS3AclPermissions newPermissions;
                newPermissions.username = username;
                newPermissions.id = id;
                if (permissionString == "FULL_CONTROL")
                    newPermissions.fullControl = true;
                else if (permissionString == "WRITE")
                    newPermissions.write = true;
                else if (permissionString == "WRITE_ACP")
                    newPermissions.writeACP = true;
                else if (permissionString == "READ")
                    newPermissions.read = true;
                else if (permissionString == "READ_ACP")
                    newPermissions.readACP = true;
                    
                if (newPermissions.id == acl.ownerId)
                    acl.ownerUser = newPermissions;
                else
                    acl.userPermissions << newPermissions;
            }
        }
        // Spesific Amazon S3 groups
        else if (granteeType == "Group")
        {
            /// @todo Support http://acs.amazonaws.com/groups/s3/LogDelivery group
            QString groupUri = grantee.firstChildElement("URI").text();
            if (groupUri == "http://acs.amazonaws.com/groups/global/AllUsers")
            {
                if (permissionString == "FULL_CONTROL")
                    acl.allUsers.fullControl = true;
                else if (permissionString == "WRITE")
                    acl.allUsers.write = true;
                else if (permissionString == "WRITE_ACP")
                    acl.allUsers.writeACP = true;
                else if (permissionString == "READ")
                    acl.allUsers.read = true;
                else if (permissionString == "READ_ACP")
                    acl.allUsers.readACP = true;
            }
            else if (groupUri == "http://acs.amazonaws.com/groups/global/AuthenticatedUsers")
            {
                if (permissionString == "FULL_CONTROL")
                    acl.authenticatedUsers.fullControl = true;
                else if (permissionString == "WRITE")
                    acl.authenticatedUsers.write = true;
                else if (permissionString == "WRITE_ACP")
                    acl.authenticatedUsers.writeACP = true;
                else if (permissionString == "READ")
                    acl.authenticatedUsers.read = true;
                else if (permissionString == "READ_ACP")
                    acl.authenticatedUsers.readACP = true;
            }
        }
    }
    
    response->acl = acl;
    return true;       
}
