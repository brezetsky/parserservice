#include "pageparser.h"

PageParser::PageParser(ParserRow *r, QObject *parent) : QObject(parent)
{
    this->row = r;
}

void PageParser::load()
{
    qWarning("New webpage!");
    p = new QWebEngineView();
    connect(p, &QWebEngineView::loadFinished, this, &PageParser::parse);
    p->page()->setUrl(QUrl(row->category_url.toLatin1().constData()));
}

void PageParser::stop()
{
}

void PageParser::parse()
{
    QFile file;
    file.setFileName(":/jquery.min.js");
    file.open(QIODevice::ReadOnly);
    QString jQuery = file.readAll();
    jQuery.append("\nvar qt = { 'jQuery': jQuery.noConflict(true) };");
    file.close();
    p->page()->runJavaScript(jQuery);
    file.setFileName(":/parseProductLinks.js");
    file.open(QIODevice::ReadOnly);
    QString productLinksParser = file.readAll();
    file.close();
    productLinksParser = productLinksParser.replace("{item_selector}", row->item_selector);
    p->page()->runJavaScript(productLinksParser,[this](const QVariant &v) {
        emit getedLink(v.toString());
    });
    file.setFileName(":/getNextPageLink.js");
    file.open(QIODevice::ReadOnly);
    QString nextPageLinkParser = file.readAll();
    file.close();
    nextPageLinkParser = nextPageLinkParser.replace("{next_page_selector}", row->next_page_selector);
    p->page()->runJavaScript(nextPageLinkParser,[this](const QVariant &v) {
        runDestructor(v);
    });
}

void PageParser::runDestructor(const QVariant &v)
{
    p->close();
    if(v.toString() != "false")
    {
        row->category_url = v.toString();
        emit pageParseEnd(row);
    }
    else
    {
        emit parserEnd();
    }
}
