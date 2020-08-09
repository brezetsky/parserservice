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
    QString YandexTranslateKey = "";
    QString ServiceTranslatorKey = "";
    qint64 ServiceTranslatorType = 1;

    ParserSettings(qint64 pi = 10800, QString aup = "/var/www/uploads", qint64 mtc = 10,
                   QString dbhn = "localhost", QString dbun = "root", QString dbup = "", QString dbn = "demoDB",
                   QString ytk = "nokey", QString stk = "nokey", qint64 stt = 1) {
        this->ParseInterval = pi;
        this->AbsUploadPath = aup;
        this->MaxThreadsCount = mtc;
        this->DBHostName = dbhn;
        this->DBUserName = dbun;
        this->DBUserPassword = dbup;
        this->DBName = dbn;
        this->YandexTranslateKey = ytk;
        this->ServiceTranslatorKey = stk;
        this->ServiceTranslatorType = stt;
    }
};


#endif // PARSERSETTINGS_H
