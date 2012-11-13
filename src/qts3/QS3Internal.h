
#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QHash>
#include <QCryptographicHash>
#include <QByteArray>

namespace QS3
{   
    static QString NEWLINE = "\n";
    static QString ROOT_PATH = "/";

    static QStringList AMAZON_QUERY_KEYS;
    static QHash<uint, QString> MONTHS;
    static QHash<uint, QString> DAYS;

    static QByteArray AMAZON_HEADER_PREFIX = "x-amz-";
    static QByteArray AMAZON_HEADER_ACL = "x-amz-acl";
    static QByteArray STANDARD_HEADER_AUTHORIZATION = "Authorization";
    static QByteArray STANDARD_HEADER_DATE = "Date";

    static void initStaticData()
    {
        AMAZON_QUERY_KEYS.clear();
        AMAZON_QUERY_KEYS << "versioning" << "location" 
                          << "acl" << "torrent" 
                          << "lifecycle" << "versionid";

        MONTHS.clear();
        MONTHS[1] = "Jan";
        MONTHS[2] = "Feb";
        MONTHS[3] = "Mar";
        MONTHS[4] = "Apr";
        MONTHS[5] = "May";
        MONTHS[6] = "Jun";
        MONTHS[7] = "Jul";
        MONTHS[8] = "Aug";
        MONTHS[9] = "Sep";
        MONTHS[10] = "Oct";
        MONTHS[11] = "Nov";
        MONTHS[12] = "Dec";

        DAYS.clear();
        DAYS[1] = "Mon";
        DAYS[2] = "Tue";
        DAYS[3] = "Wed";
        DAYS[4] = "Thu";
        DAYS[5] = "Fri";
        DAYS[6] = "Sat";
        DAYS[7] = "Sun";
    }

    bool QueryItemCompare(const QS3QueryPair &q1, const QS3QueryPair &q2)
    {
        return (q1.first.compare(q2.first) < 0);
    }

    static QString generateTimestamp()
    {
        // Generate the needed timestamp
        // http://www.ietf.org/rfc/rfc2616.txt 3.3 Date/Time Formats
        // Example: Tue, 27 Mar 2007 19:36:42 GMT
        QString formatted;
        QDateTime current = QDateTime::currentDateTime().toUTC();
        formatted.append(DAYS[current.date().dayOfWeek()] + ", ");      // 'Tue, '
        formatted.append(current.toString("dd") + " ");                 // '27 '
        formatted.append(MONTHS[current.date().month()] + " ");         // 'Mar '
        formatted.append(current.toString("yyyy hh:mm:ss") + " GMT");   // '2007 19:36:42 GMT'
        return formatted;
    }

    // Copied from http://www.d-pointer.com/solutions/kqoauth/ Class: KQOAuthUtils
    static QString hmacSha1(const QString &message, const QString &key)
    {
        QByteArray keyBytes = key.toAscii();
        int keyLength;              // Lenght of key word
        const int blockSize = 64;   // Both MD5 and SHA-1 have a block size of 64.

        keyLength = keyBytes.size();
        // If key is longer than block size, we need to hash the key
        if (keyLength > blockSize) {
            QCryptographicHash hash(QCryptographicHash::Sha1);
            hash.addData(keyBytes);
            keyBytes = hash.result();
        }

        /* http://tools.ietf.org/html/rfc2104  - (1) */
        // Create the opad and ipad for the hash function.
        QByteArray ipad;
        QByteArray opad;

        ipad.fill( 0, blockSize);
        opad.fill( 0, blockSize);

        ipad.replace(0, keyBytes.length(), keyBytes);
        opad.replace(0, keyBytes.length(), keyBytes);

        /* http://tools.ietf.org/html/rfc2104 - (2) & (5) */
        for (int i=0; i<64; i++) {
            ipad[i] = ipad[i] ^ 0x36;
            opad[i] = opad[i] ^ 0x5c;
        }

        QByteArray workArray;
        workArray.clear();

        workArray.append(ipad, 64);
        /* http://tools.ietf.org/html/rfc2104 - (3) */
        workArray.append(message.toAscii());


        /* http://tools.ietf.org/html/rfc2104 - (4) */
        QByteArray sha1 = QCryptographicHash::hash(workArray, QCryptographicHash::Sha1);

        /* http://tools.ietf.org/html/rfc2104 - (6) */
        workArray.clear();
        workArray.append(opad, 64);
        workArray.append(sha1);

        sha1.clear();

        /* http://tools.ietf.org/html/rfc2104 - (7) */
        sha1 = QCryptographicHash::hash(workArray, QCryptographicHash::Sha1);
        return QString(sha1.toBase64());
    }
    
    static QByteArray cannedAclToHeader(QS3::CannedAcl cannedAcl)
    {
        switch(cannedAcl)
        {
            case QS3::Private:
                return "private";
            case QS3::PublicRead:
                return "public-read";
            case QS3::PublicReadWrite:
                return "public-read-write";
            case QS3::AuthenticatedRead:
                return "authenticated-read";
            case QS3::BucketOwnerRead:
                return "bucket-owner-read";
            case QS3::BucketOwnerFullControl:
                return "bucket-owner-full-control";
            default:
                return "";
        }
        return "";
    }
    
    static void addOrReplaceQuery(QUrl *url, const QString &key, const QString &value)
    {
        // Set the marker
        if (url->hasQueryItem("marker"))
        {
            QS3QueryPairList query = url->queryItems();
            for(int i=0; i<query.size(); ++i)
            {
                QS3QueryPair &pair = query[i];
                if (pair.first == key)
                    pair.second = value;
            }
            url->setQueryItems(query);
        }
        else
            url->addQueryItem(key, value);
    }

    static QString generateOrderedQuery(const Q3SQueryParams &queryParams)
    {
        if (queryParams.isEmpty())
            return "";

        QString query = "?";
        foreach(QString queryKey, queryParams.keys())
        {
            if (queryParams[queryKey].isEmpty())
                query += queryKey + "&";
            else
                query += queryKey + "=" + queryParams[queryKey] + "&";
        }
        if (query.endsWith("&"))
            query = query.left(query.length()-1);
        return query;
    }
    
    static QString generateOrderedQuery(QS3QueryPairList queryItems, const QStringList &keepKeys)
    {
        if (queryItems.isEmpty())
            return "";

        qSort(queryItems.begin(), queryItems.end(), QS3::QueryItemCompare);
        Q3SQueryParams queryParams;
        foreach(QS3QueryPair queryPair, queryItems)
        {
            if (keepKeys.isEmpty())
                queryParams[queryPair.first] = queryPair.second;
            else if (keepKeys.contains(queryPair.first, Qt::CaseInsensitive))
                queryParams[queryPair.first] = queryPair.second;
        }
        return generateOrderedQuery(queryParams);
    }
}
