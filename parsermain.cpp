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
        exit(0);
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
    connect(parseTimer, SIGNAL(timeout()),this,SLOT(load()));
    parseTimer->setSingleShot(true);
    parseTimer->start();
    sitedb.close();
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
    //qWarning("Timer fires!");
    if(!sitedb.open())
    {
        qWarning("Can't connect to database!");
        exit(0);
    }
    QSqlQuery query(sitedb);
    query.exec("SELECT parse_interval, abs_upload_path, max_threads_count FROM site_settings");
    QString AbsUploadPath;
    qint64 ParseInterval, MaxThreadsCount = 10;
    while (query.next()) {
        ParseInterval = query.value(0).toInt();
        AbsUploadPath = query.value(1).toString();
        MaxThreadsCount = query.value(2).toInt();
    }
    sitedb.close();
    settings->ParseInterval = ParseInterval;
    settings->AbsUploadPath = AbsUploadPath;
    settings->MaxThreadsCount = MaxThreadsCount;
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
    }
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
        ProductParser *product_parser = new ProductParser(r, wp, settings);
        connect(product_parser, &ProductParser::parserEnd, this, &ParserMain::manage_parsers);
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
        sitedb.close();
    }
}

void ParserMain::loadDbData()
{
    if(this->disabled == false)
    {
        if(!sitedb.open())
        {
            qWarning("Can't connect to database!");
            exit(0);
        }
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
            QJsonDocument doc = QJsonDocument::fromJson(getParserQuery.value(16).toString().toUtf8());
           //get the jsonObject
            QJsonObject jObject = doc.object();
            p->additional_fields = jObject.toVariantMap();
            p->price_formula = getParserQuery.value(17).toString();
            p->date_create = getParserQuery.value(18).toInt();
            p->date_last_parse = getParserQuery.value(19).toInt();
            categoryPages.append(p);
        }
    }
}

ParserMain::~ParserMain()
{
    emit stopAllThread();
}
