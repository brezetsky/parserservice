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

    ParserSettings(qint64 ParseInterval = 10800, QString AbsUploadPath = "/var/www/uploads", qint64 MaxThreadsCount = 10,
                   QString DBHostName = "localhost", QString DBUserName = "root", QString DBUserPassword = "", QString DBName = "demoDB") {
        this->ParseInterval = ParseInterval;
        this->AbsUploadPath = AbsUploadPath;
        this->MaxThreadsCount = MaxThreadsCount;
        this->DBHostName = DBHostName;
        this->DBUserName = DBUserName;
        this->DBUserPassword = DBUserPassword;
        this->DBName = DBName;
    }
};


#endif // PARSERSETTINGS_H
