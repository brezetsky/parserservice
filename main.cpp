#include <parserservice.h>
#include <QStringList>
#include <QDir>
#include <QSettings>

int main(int argc, char *argv[])
{
#if !defined(Q_OS_WIN)
    // QtService stores service settings in SystemScope, which normally require root privileges.
    // To allow testing this example as non-root, we change the directory of the SystemScope settings file.
    //QString path = QDir::homePath() + "/.ps";
    //QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, path);
    //qWarning("(Example uses dummy settings file: %s/QtSoftware.conf)", path.toLatin1().constData());
#endif
    ParserService service(argc, argv);
    return service.exec();
}
