/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/

#include "gsdocedit.h"
#include "codeeditor.h"
#include "replacedata_dialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QStyle>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>
#include <QToolBar>
#include <QWidget>
#include <QCoreApplication>
#include <QDir>
#include <QSize>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>

#include <gSafe>

#include <docassembler.h>

GsDocEdit::GsDocEdit(QWidget *parent): QMainWindow(parent), editor(new CodeEditor(this)) 
{
    setCentralWidget(editor);
    setupMenus();
    loadReplaceMapFromFile();
    resize(800, 600);

    // Status bar with version on the left
    QLabel *verLabel = new QLabel(QString("Version: %1, gSafe: %2, Qt: %3")
                                      .arg(GSDC_VERSION)
                                      .arg(GSAFE_VERSION)
                                      .arg(QT_VERSION_STR), this);
    verLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->setStyleSheet("background-color: #afafaf; color: #101010;");
    statusBar()->addWidget(verLabel);
}

void GsDocEdit::setupMenus() 
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *newAct = new QAction(tr("&New"), this);
    newAct->setIcon(QIcon(":/icons/document-new.png"));
    connect(newAct, &QAction::triggered, this, &GsDocEdit::newDocument);
    fileMenu->addAction(newAct);

    QAction *openAct = new QAction(tr("&Open..."), this);
    openAct->setIcon(QIcon(":/icons/document-open.png"));
    connect(openAct, &QAction::triggered, this, &GsDocEdit::openDocument);
    fileMenu->addAction(openAct);

    QAction *saveAct = new QAction(tr("&Save"), this);
    saveAct->setIcon(QIcon(":/icons/document-save.png"));
    connect(saveAct, &QAction::triggered, this, &GsDocEdit::saveDocument);
    fileMenu->addAction(saveAct);

    QAction *saveAsAct = new QAction(tr("Save As..."), this);
    saveAsAct->setIcon(QIcon(":/icons/document-save-as.png"));
    connect(saveAsAct, &QAction::triggered, this, &GsDocEdit::saveDocumentAs);
    fileMenu->addAction(saveAsAct);

    QAction *quitAct = new QAction(tr("Quit"), this);
    quitAct->setIcon(QIcon(":/icons/dialog-close.png"));
    connect(quitAct, &QAction::triggered, this, &GsDocEdit::close);
    fileMenu->addAction(quitAct);

    QToolBar *toolBar = addToolBar(tr("Toolbar"));
    toolBar->setIconSize(QSize(24, 24));
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBar->addAction(newAct);
    toolBar->addAction(openAct);
    toolBar->addAction(saveAct);
    toolBar->addAction(saveAsAct);

    QMenu *funcMenu = menuBar()->addMenu(tr("&Functions"));

    QAction *previewAct = new QAction(tr("Preview"), this);
    QIcon previewIcon(":/icons/preview.png");
    previewAct->setIcon(previewIcon);
    funcMenu->addAction(previewAct);
    connect(previewAct, &QAction::triggered, this, &GsDocEdit::previewDocument);
    previewAct->setShortcut(QKeySequence(Qt::Key_F5)); // Set F6 as shortcut for previewAct

    QAction *extviewAct = new QAction(tr("Export document"), this);
    QIcon extviewIcon(":/icons/external.png");
    extviewAct->setIcon(extviewIcon);
    funcMenu->addAction(extviewAct);
    connect(extviewAct, &QAction::triggered, this, &GsDocEdit::exportDocument);
    extviewAct->setShortcut(QKeySequence(Qt::Key_F6)); // Set F5 as shortcut for extviewAct

    QAction *extviewArrayAct = new QAction(tr("Export document array according to CSV file"), this);
    QIcon extviewArrayIcon(":/icons/arraydoc.png");
    extviewArrayAct->setIcon(extviewArrayIcon);
    funcMenu->addAction(extviewArrayAct);
    connect(extviewArrayAct, &QAction::triggered, this, &GsDocEdit::exportArrayDocument);

    funcMenu->addSeparator();

    QAction *replaceDataAct = new QAction(tr("Edit Replacement Data"), this);
    QIcon icon(":/icons/replace_swap.png");
    
    replaceDataAct->setIcon(icon);
    if(replaceDataAct->icon().isNull())
        qDebug() << "replaceDataAct icon is still null";
    funcMenu->addAction(replaceDataAct);
    connect(replaceDataAct, &QAction::triggered, this, &GsDocEdit::editReplaceData);

    QAction *exportAct = new QAction(tr("Export Key-Value..."), this);
    exportAct->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    funcMenu->addAction(exportAct);
    connect(exportAct, &QAction::triggered, this, &GsDocEdit::exportReplaceMap);

    QAction *importAct = new QAction(tr("Import Key-Value..."), this);
    importAct->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    funcMenu->addAction(importAct);
    connect(importAct, &QAction::triggered, this, &GsDocEdit::importReplaceMap);

    QWidget *toolGap1 = new QWidget(this);
    QWidget *toolGap2 = new QWidget(this);
    QWidget *toolGap3 = new QWidget(this);
    toolGap1->setFixedWidth(12);
    toolGap2->setFixedWidth(12);
    toolGap3->setFixedWidth(12);
    toolBar->addWidget(toolGap1);
    toolBar->addAction(previewAct);
    toolBar->addAction(extviewAct);
    toolBar->addWidget(toolGap2);
    toolBar->addAction(extviewArrayAct);
    toolBar->addWidget(toolGap3);
    toolBar->addAction(replaceDataAct);

    // Egyéb menü
    QMenu *otherMenu = menuBar()->addMenu(tr("&Other"));

    QAction *debugConsoleAct = new QAction(tr("Debug console"), this);
    QIcon consoleIcon(":/icons/console.svg");
    debugConsoleAct->setIcon(consoleIcon);
    otherMenu->addAction(debugConsoleAct);
    connect(debugConsoleAct, &QAction::triggered, this, [](){ dconsole(); });

    QAction *aboutAct = new QAction(tr("About"), this);
    aboutAct->setIcon(QIcon(":/icons/information.png"));
    otherMenu->addAction(aboutAct);
    connect(aboutAct, &QAction::triggered, this, [this](){
            QMessageBox::information(this,tr("GsDocEdit - About"),
                                 QString(tr("GsDocEdit - %1 \nAuthor: Deák Péter (hyper80@gmail.com)\nLincense: GPLv2\n%2 - Qt: %3"))
                                     .arg(VERSION)
                                     .arg(__DATE__)
                                     .arg(QT_VERSION_STR));
    });

    setWindowTitle(tr("gSafe document editor"));
    setWindowIcon(QIcon(":/icons/gsdocedit.png"));
}

void GsDocEdit::newDocument() 
{
    if(editor->document() && editor->document()->isModified())
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Save Confirmation"),
            tr("The document has been modified. Do you want to save the changes?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
        );

        if(reply == QMessageBox::Yes)
        {
            saveDocument();
            if (editor->document()->isModified()) // unsuccessful save
                return;
        } 
        else if(reply == QMessageBox::Cancel)
        {
            return;
        }
    }
    editor->clear();
    currentFile.clear();
    setWindowTitle(tr("New Document - gSafe document editor"));
    if(editor->document())
        editor->document()->setModified(false);
}

void GsDocEdit::openDocument() 
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), QString(), tr("All Files (*.*);;Text Files (*.txt *.cpp *.h *.json)"));
    if(fileName.isEmpty())
        return;
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open the file."));
        return;
    }
    QTextStream in(&file);
    editor->setPlainText(in.readAll());
    currentFile = fileName;
    setWindowTitle(QString("%1 - gSafe document editor").arg(QFileInfo(fileName).fileName()));
    if(editor->document())
        editor->document()->setModified(false);
}

void GsDocEdit::saveDocument() 
{
    QString fileName = currentFile;
    if(fileName.isEmpty())
    {
        fileName = QFileDialog::getSaveFileName(this, tr("Save"), QString(), tr("Text Files (*.txt);;All Files (*.*)"));
        if (fileName.isEmpty())
            return;
        currentFile = fileName;
    }
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot save the file."));
        return;
    }
    QTextStream out(&file);
    out << editor->toPlainText();
    setWindowTitle(QString("%1 - gSafe document editor")
                       .arg(QFileInfo(fileName).fileName()));
    if(editor->document())
        editor->document()->setModified(false);
}

void GsDocEdit::saveDocumentAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), currentFile.isEmpty() ? QString() : currentFile, tr("Text Files (*.txt);;All Files (*.*)"));
    if(fileName.isEmpty())
        return;
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot save the file."));
        return;
    }
    QTextStream out(&file);
    out << editor->toPlainText();
    currentFile = fileName;
    setWindowTitle(QString("%1 - gSafe document editor").arg(QFileInfo(fileName).fileName()));
    if(editor->document())
        editor->document()->setModified(false);
}

void GsDocEdit::editReplaceData() 
{
    ReplaceDataDialog dlg(replaceMap, this);
    if(dlg.exec() == QDialog::Accepted)
    {
        replaceMap = dlg.results();
        saveReplaceMapToFile();
    }
}

static QString unquoteCsvField(const QString &s)
{
    if(s.size() >= 2 && s.startsWith('"') && s.endsWith('"'))
    {
        QString t = s.mid(1, s.size() - 2);
        return t.replace(QStringLiteral("\"\""), QStringLiteral("\""));
    }
    return s;
}

static QString quoteCsvField(const QString &s) 
{
    if(s.contains(';') || s.contains('"') || s.contains('\n'))
    {
        QString t = s;
        t.replace(QStringLiteral("\""), QStringLiteral("\"\""));
        return QStringLiteral("\"") + t + QStringLiteral("\"");
    }
    return s;
}

void GsDocEdit::loadReplaceMapFromFile() 
{
    QFile f("key-values.csv");
    if(!f.exists())
        return;
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&f);
    replaceMap.clear();
    while(!in.atEnd())
    {
        const QString line = in.readLine();
        if(line.isEmpty())
            continue;
        int commaPos = -1;
        bool inQuotes = false;
        for(int i = 0; i < line.size(); ++i)
        {
            QChar c = line[i];
            if(c == '"')
            {
                if (i + 1 < line.size() && line[i+1] == '"')
                {
                    ++i;
                    continue;
                }
                inQuotes = !inQuotes;
            } 
            else if (c == ';' && !inQuotes) 
            { 
                commaPos = i;
                break;
            }
        }
        if(commaPos == -1)
            continue;
        QString keyField = line.left(commaPos);
        QString valueField = line.mid(commaPos + 1);
        QString key = unquoteCsvField(keyField.trimmed());
        QString value = unquoteCsvField(valueField.trimmed());
        replaceMap.insert(key, value);
    }
}

void GsDocEdit::saveReplaceMapToFile() 
{
    QFile f("key-values.csv");
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&f);
    for(auto it = replaceMap.constBegin(); it != replaceMap.constEnd(); ++it)
    {
        out << quoteCsvField(it.key()) << ';' << quoteCsvField(it.value()) << '\n';
    }
}

void GsDocEdit::exportReplaceMap()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to CSV"), QString(), tr("CSV File (*.csv);;All Files (*.*)"));
    if(fileName.isEmpty())
        return;
    QFile f(fileName);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open the file for writing."));
        return;
    }
    QTextStream out(&f);
    for(auto it = replaceMap.constBegin(); it != replaceMap.constEnd(); ++it)
    {
        out << quoteCsvField(it.key()) << ';' << quoteCsvField(it.value()) << '\n';
    }
}

void GsDocEdit::importReplaceMap()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import CSV"), QString(), tr("CSV File (*.csv);;All Files (*.*)"));
    if(fileName.isEmpty())
        return;
    QFile f(fileName);
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open the file."));
        return;
    }
    
    QTextStream in(&f);
    QMap<QString, QString> newMap;
    while(!in.atEnd())
    {
        QString line = in.readLine();
        if(line.isEmpty())
            continue;
        int commaPos = -1;
        bool inQuotes = false;
        for(int i = 0; i < line.size(); ++i)
        {
            QChar c = line[i];
            if (c == '"') 
            {
                if (i + 1 < line.size() && line[i+1] == '"')
                {
                    ++i;
                    continue;
                }
                inQuotes = !inQuotes;
            } 
            else if(c == ';' && !inQuotes)
            {
                commaPos = i;
                break;
            }
        }
        if (commaPos == -1) 
            continue;
        QString keyField = line.left(commaPos);
        QString valueField = line.mid(commaPos + 1);
        QString key = unquoteCsvField(keyField.trimmed());
        QString value = unquoteCsvField(valueField.trimmed());
        newMap.insert(key, value);
    }
    // Replace current map and persist to default key-values.csv
    replaceMap = newMap;
    saveReplaceMapToFile();
}

QMap<QString, QString> merge_maps(const QMap<QString, QString> &base, const QMap<QString, QString> &overrides)
{
    QMap<QString, QString> result = base;
    for (auto it = overrides.constBegin(); it != overrides.constEnd(); ++it)
    {
        result[it.key()] = it.value();
    }
    return result;
}

/* This function receives a QMap<QString,QString> and splits it into a 
   more QMap<QString, QString> maps according to the keys prefixes which
   are separated by a dot ('.') character.
   For example, a key "user.name" will be placed into a map with the name "user"
   The result is a QMap where the keys are the prefixes and the values are the corresponding QMap<QString,QString> */
QMap< QString, QMap<QString,QString> > split_stringmaps(QMap<QString,QString> input)
{
    QMap< QString, QMap<QString,QString> > result;

    for (auto it = input.constBegin(); it != input.constEnd(); ++it)
    {
        const QString &fullKey = it.key();
        QString value = it.value();
        
        int dotIndex = fullKey.indexOf('.');
        if (dotIndex == -1)
        {
            // No prefix, put in a default map
            result["values"].insert(fullKey, value);
        }
        else
        {
            QString prefix = fullKey.left(dotIndex);
            QString subKey = fullKey.mid(dotIndex + 1);
            result[prefix].insert(subKey, value);
        }
    }

    return result;
}

void GsDocEdit::closeEvent(QCloseEvent *event)
{
    if (editor && editor->document() && editor->document()->isModified())
    {
        const QMessageBox::StandardButton ret = QMessageBox::question(
            this,
            tr("Save Confirmation"),
            tr("The document is not saved.\nDo you want to save the changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );

        if (ret == QMessageBox::Save)
        {
            saveDocument();
            if (editor->document()->isModified())
            {
                // User cancelled save or save failed
                event->ignore();
                return;
            }
            event->accept();
            return;
        }
        else if (ret == QMessageBox::Discard)
        {
            event->accept();
            return;
        }
        else
        {
            event->ignore();
            return;
        }
    }
    event->accept();
}

QString GsDocEdit::getRawDocumentCode()
{
    HTextProcessor tproc;
    tproc.clearValueMaps();

    auto splitMaps = split_stringmaps(replaceMap);
    for (auto it = splitMaps.constBegin(); it != splitMaps.constEnd(); ++it)
        tproc.addValueMap(it.key(), it.value());
    
    QString rcode = tproc.processDoc(editor->toPlainText());
    return rcode;
}

void GsDocEdit::previewDocument()
{
    const QString tempDir = QDir::tempPath();
    const QString fileName = "preview.pdf";
    const QString filePath = QDir(tempDir).filePath(fileName);

    DocAssembler *da = new DocAssembler(editor->toPlainText());
    auto splitMaps = split_stringmaps(replaceMap);
    for (auto it = splitMaps.constBegin(); it != splitMaps.constEnd(); ++it)
        da->addValueMap(it.key(), it.value());

    try {
        da->generatePdfDocument(filePath);
    } catch (const GSafeException &e) {
        QMessageBox::critical(this, tr("Error"), tr("An error occurred while generating the PDF:\n%1").arg(e.what()));
        delete da;
        return;
    }

    delete da;

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN)
    // Use Windows 'start' via cmd to open with associated application
    const QString nativePath = QDir::toNativeSeparators(filePath);
    const QString cmd = QString("start %1").arg(nativePath);
    QProcess::startDetached("cmd", QStringList() << "/c" << cmd);
#else
    // Fallback: use QDesktopServices to open the file
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
#endif

}

void GsDocEdit::exportDocument()
{
    QString outputFile = QFileDialog::getSaveFileName(this, tr("Export PDF"), QString(), tr("PDF Files (*.pdf);;All Files (*.*)"));
    if (outputFile.isEmpty())
        return;
    
    DocAssembler *da = new DocAssembler(editor->toPlainText());
    auto splitMaps = split_stringmaps(replaceMap);
    for (auto it = splitMaps.constBegin(); it != splitMaps.constEnd(); ++it)
        da->addValueMap(it.key(), it.value());

    try {
        da->generatePdfDocument(outputFile);
    } catch (const GSafeException &e) {
        QMessageBox::critical(this, tr("Error"), tr("An error occurred while generating the PDF:\n%1").arg(e.what()));
        delete da;
        return;
    }

    delete da;

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN)
    // Use Windows 'start' via cmd to open with associated application
    const QString nativePath = QDir::toNativeSeparators(outputFile);
    const QString cmd = QString("start %1").arg(nativePath);
    QProcess::startDetached("cmd", QStringList() << "/c" << cmd);
#else
    // Fallback: use QDesktopServices to open the file
    QDesktopServices::openUrl(QUrl::fromLocalFile(outputFile));
#endif

}

void GsDocEdit::exportArrayDocument()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Export Document Array"));

    QLineEdit *csvEdit = new QLineEdit(&dlg);
    csvEdit->setMinimumWidth(350);
    QPushButton *csvBrowse = new QPushButton(tr("Browse..."), &dlg);

    QLineEdit *dirEdit = new QLineEdit(&dlg);
    dirEdit->setMinimumWidth(350);
    QPushButton *dirBrowse = new QPushButton(tr("Browse..."), &dlg);

    QHBoxLayout *csvRow = new QHBoxLayout;
    csvRow->addWidget(csvEdit);
    csvRow->addWidget(csvBrowse);

    QHBoxLayout *dirRow = new QHBoxLayout;
    dirRow->addWidget(dirEdit);
    dirRow->addWidget(dirBrowse);

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("CSV file:"), csvRow);
    form->addRow(tr("Output directory:"), dirRow);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);
    mainLayout->addLayout(form);
    mainLayout->addWidget(buttons);

    connect(csvBrowse, &QPushButton::clicked, &dlg, [&](){
        QString f = QFileDialog::getOpenFileName(&dlg, tr("Select CSV file"), csvEdit->text(), tr("CSV Files (*.csv);;All Files (*.*)"));
        if(!f.isEmpty())
            csvEdit->setText(f);
    });

    connect(dirBrowse, &QPushButton::clicked, &dlg, [&](){
        QString d = QFileDialog::getExistingDirectory(&dlg, tr("Select output directory"), dirEdit->text());
        if(!d.isEmpty())
            dirEdit->setText(d);
    });

    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if(dlg.exec() != QDialog::Accepted)
        return;

    const QString csvFile = csvEdit->text().trimmed();
    const QString outputDirectory = dirEdit->text().trimmed();

    if(csvFile.isEmpty() || outputDirectory.isEmpty())
        return;

    csvArrayGenerator(csvFile, outputDirectory);
}

int GsDocEdit::generateDocumentArrayItem(QMap<QString, QString> replacemap, QString outputDirectory,int o_index)
{
    DocAssembler *da = new DocAssembler(editor->toPlainText());
    auto splitMaps = split_stringmaps(merge_maps(replaceMap, replacemap));
    for(auto it = splitMaps.constBegin(); it != splitMaps.constEnd(); ++it)
        da->addValueMap(it.key(), it.value());
    
    try {
        da->generatePdfDocument(outputDirectory + QDir::separator() + QString::asprintf("output_%03d.pdf",o_index));
    } catch (const GSafeException &e) {
        sdebug(QString("Error in generateDocumentArrayItem: %1").arg(e.what()));
        delete da;
        return 1;
    }
    return 0;
}

/* This function receive a CSV file name and path as parameter.
   It reads the CSV file, and do the following:
   The first row of the CSV contains string which are used as keys.
   The leftmost rows contains the values for the keys in the first row.
   The function calls the foo(QMap<QString, QString>) function for each row with a map where the keys are the values of the first row and the values are the corresponding values of the current row. */
int GsDocEdit::csvArrayGenerator(QString csvFile,QString outputDirectory)
{
    QFile f(csvFile);
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open the CSV file."));
        return 1;
    }
    QTextStream in(&f);
    QStringList keys;
    if(!in.atEnd())
    {
        QString headerLine = in.readLine();
        keys = headerLine.split(';');
        for(int i = 0; i < keys.size(); ++i)
        {
            QString cKey = unquoteCsvField(keys[i].trimmed());
            while(cKey.startsWith("."))
                cKey = cKey.mid(1);
            keys[i] = cKey;
        }
    }

    QStringList lines;
    while(!in.atEnd())
    {
        QString line = in.readLine();
        if(line.isEmpty())
            continue;
        lines.push_back(line);
    }

    int o_index = 1;
    FancyProgressBar *progress = new FancyProgressBar(this,tr("Generating documents..."),lines.count());
    int i;
    for (i = 0 ; i < lines.count() ; ++i )
    {
        QStringList values = lines[i].split(';');
        QMap<QString, QString> rowMap;
        for(int i = 0; i < qMin(keys.size(), values.size()); ++i)
            rowMap[keys[i]] = unquoteCsvField(values[i].trimmed());
        if(generateDocumentArrayItem(rowMap,outputDirectory,o_index) == 0)
            progress->stepOneSuccess();
        else
            progress->stepOneFailed();
        ++o_index;
    }
    delete progress;
    return 0;
}

// ------------------------------------------------------------------------------ //
FancyProgressBar::FancyProgressBar(QWidget *parent,QString title,int maximum)
{
    success = 0;
    failed = 0;
    max = maximum;
    pd = new QWidget(parent,Qt::Popup | Qt::SplashScreen	| Qt::WindowStaysOnTopHint);

    QLabel *labt = new QLabel(pd);
    labt->setText(title);
    QPalette p = pd->palette();
    p.setColor(QPalette::Window,QColor(180,180,180));
    pd->setPalette(p);
    pro = new QProgressBar(pd);
    lab1 = new QLabel(pd);
    lab1->setTextFormat(Qt::RichText);
    lab1->setText("-");
    lab2 = new QLabel(pd);
    lab2->setTextFormat(Qt::RichText);
    lab2->setText("-");
    QVBoxLayout *lay = new QVBoxLayout(pd);
    QHBoxLayout *laysub = new QHBoxLayout(0);
    lay->addWidget(labt);
    lay->addWidget(pro);
    lay->addLayout(laysub);
    laysub->addWidget(lab1);
    laysub->addStretch();
    laysub->addWidget(lab2);
    pd->resize(300,50);
    pd->move(parent->mapToGlobal(QPoint(25 , 25)));
    pro->setMinimum(0);
    pro->setMaximum(max);
    val=0;
    pro->setValue(val);
    pd->show();
}

void FancyProgressBar::stepOneSuccess(void)
{
    ++val;
    ++success;
    pro->setValue(val);
    setLabelText();
    QApplication::processEvents();
}

void FancyProgressBar::stepOneFailed(void)
{
    ++val;
    ++failed;
    pro->setValue(val);
    setLabelText();
    QApplication::processEvents();
}

void FancyProgressBar::setLabelText()
{
    lab1->setText(
        QString("<strong>%1 / %2</strong>")
            .arg(max)
            .arg(val));
    lab2->setText(
        QString("<span style=\"background-color: #aaffaa;\"><strong> %1 </strong></span> <span style=\"background-color: #ffaaaa;\"><strong> %2 </strong></span>")
            .arg(success)
            .arg(failed));
}

FancyProgressBar::~FancyProgressBar()
{
    pd->close();
    delete pd;
}
