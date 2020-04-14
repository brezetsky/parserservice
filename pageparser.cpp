#include "pageparser.h"

PageParser::PageParser(ParserRow *r, WebPage *p, QObject *parent) : QObject(parent)
{
    this->row = r;
    this->page = p;
}

void PageParser::parse()
{
    QFile file;
    file.setFileName(":/jquery.min.js");
    file.open(QIODevice::ReadOnly);
    QString jQuery = file.readAll();
    jQuery.append("\nvar qt = { 'jQuery': jQuery.noConflict(true) };");
    file.close();
    page->runJavaScript(jQuery);
    file.setFileName(":/parseProductLinks.js");
    file.open(QIODevice::ReadOnly);
    QString productLinksParser = file.readAll();
    file.close();
    productLinksParser = productLinksParser.replace("{item_selector}", row->item_selector);
    page->runJavaScript(productLinksParser,[this](const QVariant &v) {
        emit getedLink(v.toString(), row);
    });
    file.setFileName(":/getNextPageLink.js");
    file.open(QIODevice::ReadOnly);
    QString nextPageLinkParser = file.readAll();
    file.close();
    nextPageLinkParser = nextPageLinkParser.replace("{next_page_selector}", row->next_page_selector);
    page->runJavaScript(nextPageLinkParser,[this](const QVariant &v) {
        runDestructor(v);
    });
}

void PageParser::runDestructor(const QVariant &v)
{
    if(v.toString() != "false")
    {
        row->category_url = v.toString();
        emit pageParseEnd(row, page);
    }
    else
    {
        emit parserEnd(page, "PageParser");
    }
}

void PageParser::stop()
{
    emit parserEnd(page, "TotalStop");
}
