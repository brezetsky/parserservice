#ifndef PARSERSETTINGS_H
#define PARSERSETTINGS_H

#include <QObject>

struct ParserSettings
{
    qint64 ParseInterval;
    QString AbsUploadPath;
    qint64 MaxThreadsCount = 10;
    QString DBHostName = "localhost";
    QString DBUserName = "root";
    QString DBUserPassword = "";
    QString DBName = "demoDB";

    ParserSettings(qint64 pi = 10800, QString aup = "/var/www/uploads", qint64 mtc = 10,
                   QString dbhn = "localhost", QString dbun = "root", QString dbup = "", QString dbn = "demoDB") {
        this->ParseInterval = pi;
        this->AbsUploadPath = aup;
        this->MaxThreadsCount = mtc;
        this->DBHostName = dbhn;
        this->DBUserName = dbun;
        this->DBUserPassword = dbup;
        this->DBName = dbn;
    }
};


#endif // PARSERSETTINGS_H
