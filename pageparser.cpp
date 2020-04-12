#include "pageparser.h"

PageParser::PageParser(ParserRow *r, QObject *parent) : QObject(parent)
{
    row = r;
}

void PageParser::load()
{

}

void PageParser::stop()
{
    p->~QWebEnginePage();
    delete p;
    destroyed();
}
