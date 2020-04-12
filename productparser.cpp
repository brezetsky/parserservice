#include "productparser.h"

ProductParser::ProductParser(ParserRow *row, QString link, QObject *parent) : QObject(parent)
{
    p = new QWebEnginePage();
}
