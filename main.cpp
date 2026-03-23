/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include "gsdocedit.h"

QList<QString> availableLanguages = {
    "hu"
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QTranslator qtTranslator;
    QTranslator qtBaseTranslator;
    QTranslator myTranslator;
    QString locale = QLocale::system().name();
    for(const QString& lang : availableLanguages)
    {
        if (locale.startsWith(lang))
        {
            if(qtTranslator.load(":/qt_translations/qt_" + lang + ".qm"))
                app.installTranslator(&qtTranslator);

            if(qtBaseTranslator.load(":/qt_translations/qtbase_" + lang + ".qm"))
                app.installTranslator(&qtBaseTranslator);

            if(myTranslator.load(":/"+ lang + ".qm"))
                app.installTranslator(&myTranslator);
        }
    }

    GsDocEdit w;
    w.show();
    return app.exec();
}
