/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/

#pragma once
#include <QDialog>
#include <QMap>
#include <QString>

class QTableWidget;

class ReplaceDataDialog : public QDialog 
{
    Q_OBJECT

public:
    explicit ReplaceDataDialog(const QMap<QString, QString> &initial, QWidget *parent = nullptr);
    QMap<QString, QString> results() const;

private slots:
    void addRow();
    void removeSelectedRows();
    void handleEnterKey();
    
private:
    QTableWidget *table;
};
