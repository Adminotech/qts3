
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
