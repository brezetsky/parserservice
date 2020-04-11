#include "pageparser.h"

PageParser::PageParser(QObject *parent) : QObject(parent)
{
    p = new QWebEnginePage();
}
