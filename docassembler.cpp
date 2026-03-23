/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/

#include <docassembler.h>

DocAssembler::DocAssembler(QString documentSource)
{
    rawDocumentSource = documentSource;
    textProcessor = new HTextProcessor();
    workingDirectory = QDir::currentPath() + QDir::separator() + "work";
    sourceDocDirectory = QDir::currentPath() + QDir::separator() + "documents";
    enable_render_warnings = false;
}

DocAssembler::~DocAssembler()
{
    delete textProcessor;
}

void DocAssembler::generatePdfDocument(QString outputFile)
{
    minumimGenPageCount = 0;
    if(QFile(outputFile).exists())
        if(!QFile::remove(outputFile))
        {
            throw GSafeException(tr("Error, Cannot remove existing output file: %1").arg(outputFile));
        }

    preprocessDocument();
    generateFilenames();
    generateBasePdf();
    finishingPdf();
    
    deleteWorkfileIfExists(outputFile);
    if(!QFile::copy(filenames["result"],outputFile))
    {
        throw GSafeException(tr("Error, Cannot copy result file to output file: %1").arg(outputFile));
    } 
    
    deleteWorkfileIfExists(filenames["base"]);
    deleteWorkfileIfExists(filenames["underlay"]);
    deleteWorkfileIfExists(filenames["result"]);
}

int DocAssembler::getPageCountOfPdf(QString filename)
{
    filenames["qpdf"] = QDir::currentPath() + QDir::separator() + "qpdf" + QDir::separator() + "qpdf.exe";
    if(!QFile::exists(filenames["qpdf"]))
    {
        sdebug(QString("Error, qpdf.exe not found..."));
        throw GSafeException(tr("Error, the qpdf.exe not found in qpdf directory.\nPlease download the qpdf binary from https://github.com/qpdf/qpdf"));
        return -1;
    }

    QProcess qpdf;
    QStringList arguments;
    arguments << "--show-npages" << filename;
    qpdf.setReadChannel(QProcess::StandardOutput);
    qpdf.start(filenames["qpdf"],arguments);
    if(!qpdf.waitForFinished(-1))
    {
        throw GSafeException(tr("Error, Cannot run qpdf --show-npages command to the underlay pdf!"));
        return -1;
    }
    
    QString output = qpdf.readAllStandardOutput().trimmed();
    bool ok;
    int pageCount = output.toInt(&ok);
    if (!ok) {
        throw GSafeException(tr("Error, Cannot parse output of qpdf --show-npages command on the underlay pdf!"));
        return -1;
    }
    return pageCount;
    if(qpdf.exitStatus() != QProcess::NormalExit || qpdf.exitCode() != 0)
    {
        throw GSafeException(tr("Error, The qpdf --show-npages command to failed to run on the underlay pdf!"));
        return -1;
    }
    return pageCount;
}

int DocAssembler::deleteWorkfileIfExists(QString filename)
{
    if(QFile::exists(filename))
    {
        if(!QFile::remove(filename))
        {
            sdebug(QString("Error deleting file: %1").arg(filename));
            return 1;
        }
    }
    return 0;
}

int DocAssembler::preprocessDocument()
{
    preprocessedDoc = "";
    read_annotations.clear();

    preprocessedDoc = textProcessor->processDoc(rawDocumentSource);
    read_annotations = textProcessor->annotations();
    return 0;
}

int DocAssembler::generateFilenames()
{
    QDir currdir = QDir::current();
    if(!currdir.mkpath(workingDirectory))
    {
        throw GSafeException(tr("Error, Cannot create working directory: %1").arg(workingDirectory));
    }

    if(!QDir(workingDirectory).exists())
    {
        throw GSafeException(tr("Error, Cannot create working directory: %1").arg(workingDirectory));
    }

    filenames["base"] = workingDirectory + QDir::separator() + "gen.pdf";
    filenames["result"] = filenames["base"];
    if(read_annotations.contains("UnderlayPdf"))
    {
        filenames["original_underlay"] = sourceDocDirectory + QDir::separator() + read_annotations["UnderlayPdf"];
        if(QFile(filenames["original_underlay"]).exists())
        {
            filenames["underlay"] = workingDirectory + QDir::separator() + "underlay.pdf";
            filenames["result"] = workingDirectory + QDir::separator() + "result.pdf";
            
            if(QFile(filenames["underlay"]).exists())
                if(!QFile::remove(filenames["underlay"]))
                    throw GSafeException(tr("Error, Cannot remove previous work file: %1").arg(filenames["underlay"]));

            if(!QFile::copy(filenames["original_underlay"],filenames["underlay"]))
                throw GSafeException(tr("Error, Cannot copy original underlay file to working directory: %1").arg(filenames["underlay"]));
            minumimGenPageCount = getPageCountOfPdf(filenames["underlay"]);
            return 0;
        }
        else
        {
            throw GSafeException(tr("Error, The underlay pdf file specified in the document annotation not found: %1").arg(filenames["original_underlay"]));
        }
    }
    return 0;
}

int DocAssembler::generateBasePdf()
{
    int dpi = 200;

    if(QFile(filenames["base"]).exists())
        if(!QFile::remove(filenames["base"]))
            throw GSafeException(tr("Error, Cannot remove previous work file: %1").arg(filenames["base"]));

    QPdfWriter pw(filenames["base"]);

    pw.setResolution(dpi);
    pw.setPageSize(QPageSize(QPageSize::A4));  /*8.26 x 11.69  -> *200 -> 1652 x 2338*/
    pw.setPageOrientation(QPageLayout::Portrait);

    /*
    QMap<QString,QString>::Iterator i;
    for(i = attachmentFiles.begin() ; i != attachmentFiles.end() ; ++i )
        pw.addFileAttachment(i.key(),i.value().toUtf8());
    */

    QPainter pp(&pw);
    pp.setWindow(0,0,1652,2338); // PageSite A4 on 200 dpi (Set elsewhere...)
    HPageTileRenderer renderer(&pp);
    renderer.resolutionDpi = dpi;
    if(enable_render_warnings)
        renderer.setUnknownCommandWarning(true);

    connect(&renderer, &HPageTileRenderer::startNewPage,
            this,[&pw]() { pw.newPage(); });

    if(minumimGenPageCount > 0)
        preprocessedDoc += QString("\nnpuc#%1").arg(minumimGenPageCount);
    renderer.renderFromInstructions(preprocessedDoc);

    lastRenderStoredPositions = renderer.storedPositions();
    pp.end();
    return 0;
}

int DocAssembler::finishingPdf()
{
    if(!filenames.contains("underlay") || filenames["underlay"].isEmpty())
        return 0;

    //It should be checked earlier
    if(!filenames.contains("qpdf") || filenames["qpdf"].isEmpty())
        return 0;
    
    if(QFile::exists(filenames["result"]))
        if(!QFile::remove(filenames["result"]))
            throw GSafeException(tr("Error, Cannot remove previous work file: %1").arg(filenames["result"]));

    QProcess qpdf;
    QStringList arguments;
    arguments << "--overlay" << filenames["base"] << "--" << filenames["underlay"] << filenames["result"];
    qpdf.start(filenames["qpdf"],arguments);
    if(!qpdf.waitForFinished(-1))
    {
        throw GSafeException(tr("Error, Cannot run qpdf to merge the underlay and base pdf files!"));
        return 1;
    }

    if(qpdf.exitStatus() != QProcess::NormalExit || qpdf.exitCode() != 0)
    {
        throw GSafeException(tr("Error, Qpdf returned error: %1 %2").arg(qpdf.exitCode()).arg(qpdf.readAllStandardError()));
        return 1;
    }
    return 0;
}

void DocAssembler::addValueMap(QString name,const QMap<QString,QString>& m)
{
    textProcessor->addValueMap(name,m);
}

void DocAssembler::addValueList(QString name,const QList<QString>& l)
{
    textProcessor->addValueList(name,l);
}

void DocAssembler::addValueMapPtr(QString name,QMap<QString,QString>* m)
{
    textProcessor->addValueMapPtr(name,m);
}

void DocAssembler::clearValueMaps()
{
    textProcessor->clearValueMaps();
}

QMap<QString,QString> getFilenameTitlePairsFromFolder(QString folder)
{
    QMap<QString,QString> result;
    QDir dir(folder);
    if(!dir.exists())
        return result;

    QStringList nameFilters;
    nameFilters << "*.pot" << "*.POT" << "*.Pot";
    QFileInfoList fileList = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoSymLinks);
    foreach (const QFileInfo &fileInfo, fileList)
    {
        QString filename = fileInfo.fileName();
        QMap<QString, QString> annotations = getAnnotationValuesFromFile(fileInfo.absoluteFilePath());
        QString title = annotations.value("Title", fileInfo.baseName());
        result[filename] = title;
    }
    return result;
}

QMap<QString,QString> getAnnotationValuesFromFile(QString filename)
{
    QMap<QString,QString> result;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    QTextStream in(&file);
    result = getAnnotationValuesFromText(in.readAll());
    file.close();
    return result;
}

QMap<QString,QString> getAnnotationValuesFromText(QString documentSource)
{
    QMap<QString,QString> result;
    QTextStream in(&documentSource);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.startsWith("//") && line.contains("@") && line.contains(":"))
        {
            QString commentsubline = line.mid(2).trimmed();
            if(commentsubline.startsWith("@"))
            {
                QStringList parts = commentsubline.mid(1).split(":");
                if(parts.count() == 2)
                    result[parts[0].trimmed()] = parts[1].trimmed();
            }
            continue;
        }
    }
    return result;
}

QList<QString> getAnnotationLinesFromText(QString documentSource)
{
    QList<QString> result;
    QTextStream in(&documentSource);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.startsWith("//") && line.contains("@") && line.contains(":"))
        {
            QString commentsubline = line.mid(2).trimmed();
            if(commentsubline.startsWith("@"))
                result.push_back(commentsubline.mid(1).trimmed());
            continue;
        }
    }
    return result;
}
