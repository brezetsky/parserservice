#include "parsermain.h"

ParserMain::ParserMain(ParserSettings *s, QObject *parent) : QObject(parent)
{
    QDateTime cdt = QDateTime::currentDateTime();
    QString date = cdt.toString("dd.MM.yyyy") + " 00:00:00";
    QDateTime bdt = QDateTime::fromString(date, "dd.MM.yyyy HH:mm:ss");
    QDateTime ldt = bdt;
    settings = s;
    while(cdt > bdt)
    {
        cdt = QDateTime::currentDateTime();
        ldt = bdt;
        bdt = bdt.addSecs(settings->ParseInterval);
    }
    qint64 firstTimer = cdt.toTime_t() - ldt.toTime_t();
    parseTimer = new QTimer(this);
    parseTimer->setInterval(firstTimer * 1000);
    connect(parseTimer, SIGNAL(timeout()),this,SLOT(parse()));
    parseTimer->start();
}

void ParserMain::pause()
{
    this->disabled = true;
}

void ParserMain::resume()
{
    this->disabled = false;
}

void ParserMain::parse()
{
    if(parseTimer->interval() < settings->ParseInterval * 1000)
    {
        parseTimer->setInterval(settings->ParseInterval * 1000);
    }
    if(!this->disabled)
    {
        //start parsing

    }
}

