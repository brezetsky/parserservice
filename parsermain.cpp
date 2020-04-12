#include "parsermain.h"

ParserMain::ParserMain(ParserSettings *s, QObject *parent) : QObject(parent)
{
    QDateTime cdt = QDateTime::currentDateTime();
    QString date = cdt.toString("dd.MM.yyyy") + " 00:00:00";
    QDateTime bdt = QDateTime::fromString(date, "dd.MM.yyyy HH:mm:ss");
    QDateTime ldt = bdt;
    settings = s;
    sitedb = QSqlDatabase::addDatabase("QMYSQL", "pmdb");
    sitedb.setHostName(settings->DBHostName);
    sitedb.setDatabaseName(settings->DBName);
    sitedb.setUserName(settings->DBUserName);
    sitedb.setPassword(settings->DBUserPassword);
    bool ok = sitedb.open();
    if(!ok)
    {
        qWarning("Can't connect to database!");
    }
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
    QString d = this->disabled == true ? "true" : "false";
    if(this->disabled == false)
    {
        //start parsing
        QSqlQuery getParserQuery(sitedb);
        getParserQuery.prepare("SELECT * FROM product_parsers "
                      "LIMIT :offset, :limit");
        getParserQuery.bindValue(":offset", parserOffset);
        getParserQuery.bindValue(":limit", settings->MaxThreadsCount);
        getParserQuery.exec();
        parserOffset = parserOffset + settings->MaxThreadsCount;
        ParserRow *p;
        while (getParserQuery.next()) {
            p = new ParserRow();
            p->id = getParserQuery.value(0).toInt();
            p->category_id = getParserQuery.value(1).toInt();
            p->category_url = getParserQuery.value(2).toString();
            p->item_selector = getParserQuery.value(3).toString();
            p->next_page_selector = getParserQuery.value(4).toString();
            p->article_selector = getParserQuery.value(5).toString();
            p->title_selector = getParserQuery.value(6).toString();
            p->photo_selector = getParserQuery.value(7).toString();
            p->price_selector = getParserQuery.value(8).toString();
            p->description_selector = getParserQuery.value(9).toString();
            p->location_selector = getParserQuery.value(10).toString();
            p->location_etalon = getParserQuery.value(11).toString();
            p->location_full_selector = getParserQuery.value(12).toString();
            p->logistic_price = getParserQuery.value(13).toFloat();
            p->end_time_selector = getParserQuery.value(14).toString();
            p->publicate_status = getParserQuery.value(15).toInt();
            QJsonDocument doc = QJsonDocument::fromJson(getParserQuery.value(16).toString().toUtf8());
           //get the jsonObject
            QJsonObject jObject = doc.object();
            p->additional_fields = jObject.toVariantMap();
            p->price_formula = getParserQuery.value(17).toString();
            p->date_create = getParserQuery.value(18).toInt();
            p->date_last_parse = getParserQuery.value(19).toInt();
            categoryPages.append(p);
        }
        while (workers.size() < settings->MaxThreadsCount && categoryPages.size() > 0) {
            if(categoryPages.size() > 0)
            {
                QThread *w = new QThread;
                ParserRow *tpr = categoryPages.takeFirst();
                PageParser *page_parser = new PageParser(tpr);
                connect(page_parser, &PageParser::geteedLink, this, &ParserMain::manage_links);
                connect(page_parser, &PageParser::pageParseEnd, this, &ParserMain::manage_category_page);
                connect(page_parser, &PageParser::parserEnd, this, &ParserMain::manage_parsers);
                connect(w, &QThread::started, page_parser, &PageParser::load);
                connect(this, &ParserMain::stopAllThread, page_parser, &PageParser::stop);
                connect(page_parser, &PageParser::destroyed, w, &QThread::terminate);
                delete tpr;
                page_parser->moveToThread(w);
                w->start();
                workers.append(w);
            }
        }
    }
}

void ParserMain::manage_links(QString link)
{

}

void ParserMain::manage_category_page(ParserRow *row)
{

}

void ParserMain::manage_parsers()
{

}

