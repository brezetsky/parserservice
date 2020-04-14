#include "productparser.h"

ProductParser::ProductParser(ParserRow *r, WebPage *wp, ParserSettings *s, QObject *parent) : QObject(parent)
{
    p = wp;
    row = r;
    settings = s;
}

void ProductParser::stop()
{
    emit parserEnd(p, "TotalStop");
}

void ProductParser::parse()
{
    emit parserEnd(p, "ProductParser");
}
