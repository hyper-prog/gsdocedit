/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/

#ifndef DOCASSEMBLER_H
#define DOCASSEMBLER_H

#include <QtCore>
#include <gSafe>

class DocAssembler : public QObject {
    Q_OBJECT

public:
    DocAssembler(QString documentSource);
    ~DocAssembler();

    void generatePdfDocument(QString outputFile);

    void addValueMap(QString name,const QMap<QString,QString>& m);
    void addValueList(QString name,const QList<QString>& l);
    void addValueMapPtr(QString name,QMap<QString,QString>* m);
    void clearValueMaps();

protected:
    QString rawDocumentSource;
    HTextProcessor *textProcessor;
    int minumimGenPageCount;

    QString workingDirectory;
    QString sourceDocDirectory;
    
    QString preprocessedDoc;
    QMap<QString,QString> filenames;
    QMap<QString,QString> read_annotations;
    
    int preprocessDocument();
    int generateFilenames();
    int generateBasePdf();
    int finishingPdf();

    int getPageCountOfPdf(QString filename);
    int deleteWorkfileIfExists(QString filename);

public:
    bool enable_render_warnings;

    QMap<QString,HPageTileRendererPosition> lastRenderStoredPositions;
};

QMap<QString,QString> getFilenameTitlePairsFromFolder(QString folder);
QMap<QString,QString> getAnnotationValuesFromText(QString documentSource);
QList<QString>        getAnnotationLinesFromText(QString documentSource);
QMap<QString,QString> getAnnotationValuesFromFile(QString filename);


#endif // DOCASSEMBLER_H
