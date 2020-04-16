#ifndef PRODOCTITEM_H
#define PRODOCTITEM_H

#include <QObject>

struct ProductItem
{
    QString id = "0";
    QString article;
    QString category_id;
    QMap<QString,QString> title;
    QMap<QString,QString> anons;
    QString main_img_id;
    QMap<QString,QString> content;
    QString price;
    QMap<QString,QString> location;
    QString end_time;
    QMap<QString,QString> description;
    QString slug;
    QString ident_name;
    QString status;
};

#endif // PRODOCTITEM_H
