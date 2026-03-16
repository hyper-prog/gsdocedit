/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include "gsdocedit.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Load translation only if system language is Hungarian
    QTranslator translator;
    QString locale = QLocale::system().name();
    if (locale.startsWith("hu")) {
        QString translationFile = QString(":/hu.qm");
        if (translator.load(translationFile, QApplication::applicationDirPath()) || translator.load(translationFile)) {
            app.installTranslator(&translator);
        }
    }

    GsDocEdit w;
    w.show();
    return app.exec();
}
