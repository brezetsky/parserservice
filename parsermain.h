#ifndef PARSERMAIN_H
#define PARSERMAIN_H

#include <QObject>
#include <QTimer>
#include <QDate>
#include "parsersettings.h"


class ParserMain : public QObject
{
    Q_OBJECT
public:
    explicit ParserMain(ParserSettings *s, QObject *parent = nullptr);
    void pause();
    void resume();

signals:

public slots:
    void parse();

private:
    bool disabled = false;
    QTimer *parseTimer;
    ParserSettings *settings;
};

#endif // PARSERMAIN_H
