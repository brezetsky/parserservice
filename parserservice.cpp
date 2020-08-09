#include "parserservice.h"

ParserService::ParserService(int argc, char **argv) : QtService<QApplication>(argc, argv, "Qt Parser Daemon")
{
    setServiceDescription("Parser Service for shop");
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

void ParserService::start()
{
    QApplication *app = application();
    QSettings settings(QSettings::SystemScope, "QtSoftware");
    settings.beginGroup("services");
    settings.beginGroup(serviceName());
    settings.beginGroup("db");

    QString DBHostName = settings.value("host").toString();
    QString DBUserName = settings.value("user").toString();
    QString DBUserPassword = settings.value("password").toString();
    QString DBName = settings.value("dbname").toString();

    settings.endGroup();
    settings.endGroup();
    settings.endGroup();
    parserdb = QSqlDatabase::addDatabase("QMYSQL", "psdb");
    parserdb.setHostName(DBHostName);
    parserdb.setDatabaseName(DBName);
    parserdb.setUserName(DBUserName);
    parserdb.setPassword(DBUserPassword);
    bool ok = parserdb.open();
    if(!ok)
    {
        qWarning("Can't connect to database!");
    }
    QSqlQuery query(parserdb);
    query.exec("SELECT parse_interval, abs_upload_path, max_threads_count, yandex_translate_key, service_translator_key, service_translator_type FROM site_settings");
    QString AbsUploadPath, ytk, stk;
    qint64 MaxThreadsCount = 10, ParseInterval, stt;
    while (query.next()) {
        ParseInterval = query.value(0).toInt();
        AbsUploadPath = query.value(1).toString();
        MaxThreadsCount = query.value(2).toInt();
        ytk = query.value(3).toString();
        stk = query.value(4).toString();
        stt = query.value(5).toInt();
    }
    parserdb.close();
    s = new ParserSettings(ParseInterval, AbsUploadPath, MaxThreadsCount, DBHostName, DBUserName, DBUserPassword, DBName, ytk, stk, stt);
    pm = new ParserMain(s);
}

void ParserService::pause()
{
    pm->pause();
}

void ParserService::resume()
{
    pm->resume();
}
