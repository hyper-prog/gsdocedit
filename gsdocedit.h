/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/

#pragma once
#include <QMainWindow>
#include <QString>
#include <QMap>

#define GSDC_VERSION "1.012"

class CodeEditor;
class QCloseEvent;
class QProgressBar;
class QLabel;

class GsDocEdit : public QMainWindow 
{
    Q_OBJECT
public:
    GsDocEdit(QWidget *parent = nullptr);

    QMap<QString, QString> replaceMap;
    QMap<QString, QString> askReplaceMap;

    QString getRawDocumentCode();
    int csvArrayGenerator(QString csvFile, QString outputDirectory);
    int generateDocumentArrayItem(QMap<QString, QString> replacemap, QString outputDirectory,int o_index);

    QMap<QString, QString> generateFixVariableMap();

private slots:
    void editReplaceData();
    void exportReplaceMap();
    void importReplaceMap();
    void previewDocument();
    void exportDocument();
    void exportArrayDocument();
    void newDocument();
    void openDocument();
    void openlistDocument();
    void saveDocument();
    void saveDocumentAs();

    void askRequiredData();

private:
    void openFile(const QString &fileName);
    void setupMenus();
    void loadReplaceMapFromFile();
    void saveReplaceMapToFile();

protected:
    void closeEvent(QCloseEvent *event) override;

    CodeEditor *editor;
    QString currentFile;
};

class FancyProgressBar
{
protected:
    int max,val,success,failed;
    QWidget *pd;
    QProgressBar *pro;
    QLabel *lab1,*lab2;

    void setLabelText();
public:
    FancyProgressBar(QWidget *parent,QString title,int maximum);
    ~FancyProgressBar();

    void stepOneSuccess(void);
    void stepOneFailed(void);
};
