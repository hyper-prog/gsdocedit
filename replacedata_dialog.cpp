/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/

#include "replacedata_dialog.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QShortcut>
#include <QKeySequence>
#include <QAbstractItemDelegate>
#include <QLineEdit>

ReplaceDataDialog::ReplaceDataDialog(const QMap<QString, QString> &initial, QWidget *parent)
    : QDialog(parent)
{
    table = new QTableWidget(this);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({tr("Key"), tr("Value")});
    table->horizontalHeader()->setStretchLastSection(true);

    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setStyleSheet("QTableView::item:selected { background-color: #5378c9; }");

    for (auto it = initial.constBegin(); it != initial.constEnd(); ++it) 
    {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(it.key()));
        table->setItem(row, 1, new QTableWidgetItem(it.value()));
    }

    // Install a shortcut so Enter commits the editor and moves to next cell.
    QShortcut *enterShortcut = new QShortcut(QKeySequence(Qt::Key_Return), table);
    connect(enterShortcut, &QShortcut::activated, this, &ReplaceDataDialog::handleEnterKey);

    QPushButton *addBtn = new QPushButton(tr("New line"), this);
    QPushButton *removeBtn = new QPushButton(tr("Delete"), this);
    connect(addBtn, &QPushButton::clicked, this, &ReplaceDataDialog::addRow);
    connect(removeBtn, &QPushButton::clicked, this, &ReplaceDataDialog::removeSelectedRows);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addStretch();

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(table);
    main->addLayout(btnLayout);
    main->addWidget(box);

    setWindowTitle(tr("Edit Replacement Data"));
    resize(500, 400);
}

QMap<QString, QString> ReplaceDataDialog::results() const
{
    QMap<QString, QString> map;
    for (int r = 0; r < table->rowCount(); ++r)
    {
        QTableWidgetItem *k = table->item(r, 0);
        QTableWidgetItem *v = table->item(r, 1);
        if (k && !k->text().isEmpty())
            map.insert(k->text(), v ? v->text() : QString());
    }
    return map;
}

void ReplaceDataDialog::addRow()
{
    int row = table->rowCount();
    table->insertRow(row);
    table->setItem(row, 0, new QTableWidgetItem(QString()));
    table->setItem(row, 1, new QTableWidgetItem(QString()));
    table->setCurrentCell(row, 0);
    table->setFocus();
    QTableWidgetItem *it = table->item(row, 0);
    if (it)
        table->editItem(it);
}

void ReplaceDataDialog::removeSelectedRows()
{
    QList<QTableWidgetSelectionRange> ranges = table->selectedRanges();
    for (const auto &range : ranges) 
    {
        for (int r = range.bottomRow(); r >= range.topRow(); --r)
            table->removeRow(r);
    }
}

void ReplaceDataDialog::handleEnterKey()
{
    QWidget *ed = table->focusWidget();
    QModelIndex idx = table->currentIndex();
    int row = idx.row();
    int col = idx.column();
    QString entered;
    if (QLineEdit *le = qobject_cast<QLineEdit*>(ed))
    {
        entered = le->text();
    }
    else
    {
        // fallback: try to commit via delegate
        if (ed)
        {
            QAbstractItemDelegate *del = table->itemDelegate();
            if (del)
            {
                del->setModelData(ed, table->model(), idx);
                del->destroyEditor(ed, idx);
            }
        }
        QTableWidgetItem *it = table->item(row, col);
        if (it) entered = it->text();
    }

    // ensure item exists and set text
    QTableWidgetItem *curItem = table->item(row, col);
    if (!curItem)
    {
        curItem = new QTableWidgetItem(QString()); 
        table->setItem(row, col, curItem);
    }
    curItem->setText(entered);

    int newRow = row;
    int newCol = col;
    if (col < table->columnCount() - 1)
    {
        // move to next column in same row
        newCol = col + 1;
    }
    else
    {
        // last column: move to next row's first column
        newCol = 0;
        newRow = row + 1;
        if (newRow >= table->rowCount())
        {
            int r = table->rowCount();
            table->insertRow(r);
            // ensure new row has items
            table->setItem(r, 0, new QTableWidgetItem(QString()));
            table->setItem(r, 1, new QTableWidgetItem(QString()));
            newRow = r;
        }
    }

    if (!table->item(newRow, newCol))
        table->setItem(newRow, newCol, new QTableWidgetItem(QString()));
    table->setCurrentCell(newRow, newCol);
    table->editItem(table->item(newRow, newCol));
}
