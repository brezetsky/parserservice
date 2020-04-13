#include "pageparser.h"

PageParser::PageParser(ParserRow *r, QObject *parent) : QObject(parent)
{
    //this->row = r;
}

void PageParser::load()
{
    QString p = "test";
    p = "test2";
    //p = new QWebEnginePage();
    //connect(p, &QWebEnginePage::loadFinished, this, &PageParser::parse);
    //p->setUrl(QUrl(row->category_url.toLatin1().constData()));
    exit(0);
}

void PageParser::stop()
{
}

void PageParser::parse()
{
    /*QFile file;
    file.setFileName(":/jquery.min.js");
    file.open(QIODevice::ReadOnly);
    QString jQuery = file.readAll();
    jQuery.append("\nvar qt = { 'jQuery': jQuery.noConflict(true) };");
    file.close();
    p->runJavaScript(jQuery);
    file.setFileName(":/parseProductLinks.js");
    file.open(QIODevice::ReadOnly);
    QString productLinksParser = file.readAll();
    file.close();
    productLinksParser = productLinksParser.replace("{item_selector}", row->item_selector);
    p->runJavaScript(productLinksParser,[this](const QVariant &v) {
        emit getedLink(v.toString());
    });
    file.setFileName(":/getNextPageLink.js");
    file.open(QIODevice::ReadOnly);
    QString nextPageLinkParser = file.readAll();
    file.close();
    nextPageLinkParser = nextPageLinkParser.replace("{next_page_selector}", row->next_page_selector);
    p->runJavaScript(nextPageLinkParser,[this](const QVariant &v) {
        row->category_url = v.toString();
        //emit pageParseEnd(row);
    });*/
}
