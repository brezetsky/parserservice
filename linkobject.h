#ifndef LINKOBJECT_H
#define LINKOBJECT_H

#include <QObject>
#include "parserrow.h"

struct LinkObject
{
    ParserRow *prow;
    QString link;

    LinkObject(ParserRow *r, QString l) {
        prow = r;
        link = l;
    }
};

#endif // LINKOBJECT_H
