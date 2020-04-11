#ifndef PARSERSERVICE_H
#define PARSERSERVICE_H
#include<QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSettings>
#include "qtservice.h"
#include "parsermain.h"


class ParserService : public QtService<QApplication>
{
public:
    ParserService(int argc, char **argv);

protected:
    void start();

    void pause();

    void resume();

private:
    QSqlDatabase parserdb;
    ParserMain *pm;
    ParserSettings *s;
};

#endif // PARSERSERVICE_H
