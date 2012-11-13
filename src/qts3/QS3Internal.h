
#pragma once

#include <QString>
#include <QDateTime>
#include <QHash>
#include <QCryptographicHash>
#include <QByteArray>

namespace QS3Helpers
{
    bool QueryItemCompare(const QS3QueryPair &q1, const QS3QueryPair &q2)
    {
        return (q1.first.compare(q2.first) < 0);
    }
    
    static QString newline = "\n";
    static QHash<uint, QString> months;
    static QHash<uint, QString> days;

    static void checkTimestampInit()
    {
        if (months.isEmpty())
        {
            months[1] = "Jan";
            months[2] = "Feb";
            months[3] = "Mar";
            months[4] = "Apr";
            months[5] = "May";
            months[6] = "Jun";
            months[7] = "Jul";
            months[8] = "Aug";
            months[9] = "Sep";
            months[10] = "Oct";
            months[11] = "Nov";
            months[12] = "Dec";
        }
        if (days.isEmpty())
        {
            days[1] = "Mon";
            days[2] = "Tue";
            days[3] = "Wed";
            days[4] = "Thu";
            days[5] = "Fri";
            days[6] = "Sat";
            days[7] = "Sun";
        }
    }

    static QString generateTimestamp()
    {
        checkTimestampInit();

        // Generate the needed timestamp
        // http://www.ietf.org/rfc/rfc2616.txt 3.3 Date/Time Formats
        // Example: Tue, 27 Mar 2007 19:36:42 GMT
        QString formatted;
        QDateTime current = QDateTime::currentDateTime().toUTC();
        formatted.append(days[current.date().dayOfWeek()] + ", ");      // 'Tue, '
        formatted.append(current.toString("dd") + " ");                 // '27 '
        formatted.append(months[current.date().month()] + " ");         // 'Mar '
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
}
