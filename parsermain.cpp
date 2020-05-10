#include "parsermain.h"

ParserMain::ParserMain(ParserSettings *s, QObject *parent) : QObject(parent)
{
    QDateTime cdt = QDateTime::currentDateTime();
    QString date = cdt.toString("dd.MM.yyyy") + " 00:00:00";
    QDateTime bdt = QDateTime::fromString(date, "dd.MM.yyyy HH:mm:ss");
    settings = s;
#if defined(QTEST)
    qWarning("Test defined");
    exit(0);
#endif
    sitedb = QSqlDatabase::addDatabase("QMYSQL", "pmdb");
    sitedb.setHostName(settings->DBHostName);
    sitedb.setDatabaseName(settings->DBName);
    sitedb.setUserName(settings->DBUserName);
    sitedb.setPassword(settings->DBUserPassword);
    qint64 fromMaybeLastStartTime = (cdt.toTime_t() - bdt.toTime_t()) % settings->ParseInterval;
    qint64 firstTimer = settings->ParseInterval - fromMaybeLastStartTime;
    parseTimer = new QTimer(this);
    parseTimer->setInterval(firstTimer * 1000);
    connect(parseTimer, SIGNAL(timeout()),this,SLOT(load()));
    parseTimer->setSingleShot(true);
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

void ParserMain::load()
{
    if(!sitedb.open())
    {
        qWarning("Can't connect to database!");
        exit(0);
    }
    qWarning("Parser Start!!!");
    startTime = QDateTime::currentDateTime().toTime_t();
    QSqlQuery query(sitedb);
    query.exec("SELECT parse_interval, abs_upload_path, max_threads_count, yandex_translate_key FROM site_settings");
    QString AbsUploadPath;
    qint64 ParseInterval, MaxThreadsCount = 10;
    while (query.next()) {
        settings->ParseInterval = query.value(0).toInt();
        settings->AbsUploadPath = query.value(1).toString();
        settings->MaxThreadsCount = query.value(2).toInt();
        settings->YandexTranslateKey = query.value(3).toString();
    }
    //query.~QSqlQuery();
    emit stopAllThread();
    parserOffset = 0;
    if(parseTimer->interval() < settings->ParseInterval * 1000)
    {
        parseTimer->setInterval(settings->ParseInterval * 1000);
    }
    loadDbData();
    loadPage("page");
}

void ParserMain::manage_links(QString link, ParserRow *r)
{
    QJsonDocument doc = QJsonDocument::fromJson(link.toUtf8());
   //get the jsonObject
    QJsonObject jObject = doc.object();
    QVariantMap link_map = jObject.toVariantMap();
    foreach (QString k, link_map.keys()) {
        QString lk = link_map[k].toString();
        if(workers_count < settings->MaxThreadsCount)
        {
            workers_count++;
            WebPage *page = new WebPage(r, "link");
            connect(page, &WebPage::wLoadFinishedSignal, this, &ParserMain::wpFinished);
            //fprintf(stderr, "Fires link %s!\n", lk.toLatin1().constData());
            page->setUrl(QUrl(lk));
        }
        else
        {
            LinkObject *tlo = new LinkObject(r, lk);
            productLinks.append(tlo);
        }
    }
}

void ParserMain::manage_category_page(ParserRow *row, WebPage *wp)
{
    pageActiveLoaderCount--;
    wp->deleteLater();
    //qWarning(row->category_url.toLatin1().constData());
    workers_count--;
    categoryPages.prepend(row);
    if(productLinks.size() > 0)
    {
        while (productLinks.size() > 0 && workers_count < settings->MaxThreadsCount) {
            LinkObject *lo = productLinks.takeFirst();
            workers_count++;
            WebPage *page = new WebPage(lo->prow, "link");
            connect(page, &WebPage::wLoadFinishedSignal, this, &ParserMain::wpFinished);
            //fprintf(stderr, "Fires link %s!\n", lo->link.toLatin1().constData());
            page->setUrl(QUrl(lo->link));
        }
    }
    else
    {
        loadPage("page");
    }
}

void ParserMain::manage_parsers(WebPage *wp, QString sender_name)
{
    if(sender_name == "PageParser")
    {
        pageActiveLoaderCount--;
    }
    wp->deleteLater();
    workers_count--;
    if(sender_name == "TotalStop")
    {
        productLinks.clear();
        categoryPages.clear();
        workers_count = 0;
        pageActiveLoaderCount = 0;
        parserOffset = 0;
        qWarning("Parser complete!!!");
        qint64 totalTime = QDateTime::currentDateTime().toTime_t() - startTime;
        fprintf(stderr, "Total running time: %s seconds!", QString::number(totalTime).toLatin1().constData());
        sitedb.close();
    }
    else {
        if(productLinks.size() > 0)
        {
            while (productLinks.size() > 0 && workers_count < settings->MaxThreadsCount) {
                LinkObject *lo = productLinks.takeFirst();
                workers_count++;
                WebPage *page = new WebPage(lo->prow, "link");
                connect(page, &WebPage::wLoadFinishedSignal, this, &ParserMain::wpFinished);
                //fprintf(stderr, "Fires link %s!\n", lo->link.toLatin1().constData());
                page->setUrl(QUrl(lo->link));
            }
        }
        else
        {
            if(categoryPages.size() > 0)
            {
                loadPage("page");
            }
            else
            {
                loadDbData();
                if(categoryPages.size() > 0)
                {
                    loadPage("page");
                }
                if(categoryPages.size() <= 0 && pageActiveLoaderCount <= 0 && workers_count <= 0)
                {
                    qWarning("Parser complete!!!");
                    qint64 totalTime = QDateTime::currentDateTime().toTime_t() - startTime;
                    fprintf(stderr, "Total running time: %s seconds!", QString::number(totalTime).toLatin1().constData());
                    QSqlQuery query(sitedb);
                    query.prepare("UPDATE product_parsers SET date_last_parse = :last_parse;");
                    query.bindValue(":last_parse", QDateTime::currentDateTime().toTime_t());
                    query.exec();
                    sitedb.close();
                }
            }
        }
    }
}

void ParserMain::threadFinished()
{
    //qWarning("Thread finished!");
}

void ParserMain::wpFinished(bool ok, WebPage *wp, ParserRow *r, QString a)
{
    if(a == "page")
    {
        QThread *w = new QThread;
        PageParser *page_parser = new PageParser(r, wp);
        connect(page_parser, &PageParser::getedLink, this, &ParserMain::manage_links);
        connect(page_parser, &PageParser::pageParseEnd, this, &ParserMain::manage_category_page);
        connect(page_parser, &PageParser::pageParseEnd, w, &QThread::quit);
        connect(page_parser, &PageParser::parserEnd, this, &ParserMain::manage_parsers);
        connect(page_parser, &PageParser::parserEnd, w, &QThread::quit);
        connect(w, &QThread::started, page_parser, &PageParser::parse);
        connect(w, &QThread::finished, this, &ParserMain::threadFinished);
        connect(this, &ParserMain::stopAllThread, page_parser, &PageParser::stop);
        page_parser->moveToThread(w);
        w->start();
    }
    else if(a == "link")
    {
        QThread *w = new QThread;
        ProductParser *product_parser = new ProductParser(r, wp, sitedb, settings);
        connect(product_parser, &ProductParser::parserEnd, this, &ParserMain::manage_parsers);
        connect(product_parser, &ProductParser::sendLog, this, &ParserMain::printLog);
        connect(product_parser, &ProductParser::parserEnd, w, &QThread::quit);
        connect(w, &QThread::started, product_parser, &ProductParser::parse);
        connect(w, &QThread::finished, this, &ParserMain::threadFinished);
        connect(this, &ParserMain::stopAllThread, product_parser, &ProductParser::stop);
        product_parser->moveToThread(w);
        w->start();
    }
}

void ParserMain::loadPage(QString action)
{
    if(this->disabled == false)
    {
        while (workers_count < settings->MaxThreadsCount && categoryPages.size() > 0) {
            if(categoryPages.size() > 0)
            {
                ParserRow *r = categoryPages.takeFirst();
                fprintf(stderr, "Load page: %s!\n", r->category_url.toLatin1().constData());
                workers_count++;
                pageActiveLoaderCount++;
                WebPage *wp = new WebPage(r, action);
                connect(wp, &WebPage::wLoadFinishedSignal, this, &ParserMain::wpFinished);
                wp->setUrl(QUrl(r->category_url));
            }
        }
    }
}

void ParserMain::loadDbData()
{
    if(this->disabled == false)
    {
        //start parsing
        QSqlQuery getParserQuery(sitedb);
        getParserQuery.prepare("SELECT * FROM product_parsers "
                      "LIMIT :offset, :limit");
        getParserQuery.bindValue(":offset", parserOffset);
        getParserQuery.bindValue(":limit", settings->MaxThreadsCount);
        getParserQuery.exec();
        parserOffset = parserOffset + getParserQuery.size();
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
            p->additional_fields = getParserQuery.value(16).toString();
            p->date_format = getParserQuery.value(17).toString();
            p->price_formula = getParserQuery.value(18).toString();
            p->date_create = getParserQuery.value(19).toInt();
            p->date_last_parse = getParserQuery.value(20).toInt();
            categoryPages.append(p);
        }
    }
}

ParserMain::~ParserMain()
{
    emit stopAllThread();
}

void ParserMain::printLog(QString logMessage)
{
    qWarning(logMessage.toStdString().c_str());
}
