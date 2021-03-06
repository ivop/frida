#include "labelswindow.h"
#include "ui_labelswindow.h"
#include "frida.h"
#include "addlabelwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

labelswindow::labelswindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::labelswindow)
{
    QTableWidget *t;
    ui->setupUi(this);

    t = ui->tableUserLabels;
    t->addAction(ui->actionChange_To_Local_Label);
    t->addAction(ui->actionDelete_Label);

    t = ui->tableLocalLabels;
    t->addAction(ui->actionChange_To_User_Label);
    t->addAction(ui->actionDelete_Label);

    showAutoLabels();
    showUserLabels();
    showLocalLabels();
}

labelswindow::~labelswindow()
{
    delete ui;
}

void labelswindow::on_doneButton_clicked() {
    close();
}

void labelswindow::showLabels(QTableWidget *t, QMap<quint64, QString> *labels) {
    QList<quint64> keys = labels->keys();
    QString hex;
    QTableWidgetItem *item;
    int n = keys.size();

    t->setRowCount(0);
    t->setRowCount(n);

    for (int i=0; i<n; i++) {
        hex = QString("%1").arg(keys[i], 4, 16, QChar('0'));
        item = new QTableWidgetItem(hex);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        t->setItem(i,0, item);
        t->setItem(i,1, new QTableWidgetItem(labels->value(keys[i])));
    }
}

void labelswindow::showAutoLabels(void) {
    showLabels(ui->tableAutoLabels, &autoLabels);
}

void labelswindow::showUserLabels(void) {
    showLabels(ui->tableUserLabels, &userLabels);
}

void labelswindow::showLocalLabels(void) {
    showLabels(ui->tableLocalLabels, &segments[currentSegment].localLabels);
}

// ---------------------------------------------------------------------------

static void get_contents(QTableWidget *t, int row, QString *label,
                                                        quint64 *address) {
    QString addr  = t->item(row, 0)->text();
    *label = t->item(row, 1)->text();
    *address = addr.toULongLong(0, 16);
    return;
}

void labelswindow::on_tableAutoLabels_cellChanged(int row, int column) {
    QTableWidget *t = ui->tableAutoLabels;
    QString label;
    quint64 address;

    if (!t->item(row, column)->isSelected()) return; // not by user action

    get_contents(t, row, &label, &address);

    autoLabels.remove(address);
    userLabels.insert(address, label);
    showAutoLabels();
    showUserLabels();
}

void labelswindow::on_tableUserLabels_cellChanged(int row, int column) {
    QTableWidget *t = ui->tableUserLabels;
    QString label;
    quint64 address;

    if (!t->item(row,column)->isSelected()) return; // not by user action

    get_contents(t, row, &label, &address);
    userLabels.insert(address, label);
    showUserLabels();
}

void labelswindow::actionChange_To_Local_Label() {
    QTableWidget *t = ui->tableUserLabels;
    QString label;
    quint64 address;
    QList<QTableWidgetSelectionRange> Ranges = t->selectedRanges();

    if (Ranges.isEmpty()) return;

    // UI is setup properly and only one cell is selected

    int row = Ranges.at(0).topRow();

    get_contents(t, row, &label, &address);

    userLabels.remove(address);
    segments[currentSegment].localLabels.insert(address, label);
    showUserLabels();
    showLocalLabels();
}

void labelswindow::actionChange_To_User_Label() {
    QTableWidget *t = ui->tableLocalLabels;
    QString label;
    quint64 address;
    QList<QTableWidgetSelectionRange> Ranges = t->selectedRanges();

    if (Ranges.isEmpty()) return;

    // UI is setup properly and only one cell is selected

    int row = Ranges.at(0).topRow();

    get_contents(t, row, &label, &address);

    segments[currentSegment].localLabels.remove(address);
    userLabels.insert(address, label);
    showUserLabels();
    showLocalLabels();
}

void labelswindow::actionDelete_Label() {
    QTableWidget *t;
    QMap<quint64,QString> *labels;
    QString label;
    quint64 address;

    if (ui->tableUserLabels->hasFocus()) {
        t = ui->tableUserLabels;
        labels = &userLabels;
    } else if (ui->tableLocalLabels->hasFocus()) {
        t = ui->tableLocalLabels;
        labels = &segments[currentSegment].localLabels;
    } else {
        return;
    }

    QList<QTableWidgetSelectionRange> Ranges = t->selectedRanges();
    if (Ranges.isEmpty())
        return;

    // UI is setup properly and only one cell is selected

    int row = Ranges.at(0).topRow();

    get_contents(t, row, &label, &address);


    QMessageBox msg;
    msg.setText("Do you really want to delete " + label);
    msg.setStandardButtons(QMessageBox::Yes);
    msg.addButton(QMessageBox::No);
    msg.setDefaultButton(QMessageBox::No);
    if(msg.exec() != QMessageBox::Yes) return;

    labels->remove(address);
    showLabels(t, labels);
}

void labelswindow::on_addLabelButton_clicked() {
    addLabelWindow alw;
    alw.exec();
    showUserLabels();
}

// ---------------------------------------------------------------------------

void labelswindow::on_exportButton_clicked() {
    QString name = QFileDialog::getOpenFileName(this, "Export labels to...");

    if (name.isEmpty()) return;

    QFile file(name);
    if (file.exists()) {
        QMessageBox msg;
        msg.setText("File " + name + " exists. Overwrite?");
        msg.setStandardButtons(QMessageBox::Yes);
        msg.addButton(QMessageBox::No);
        msg.setDefaultButton(QMessageBox::No);
        if(msg.exec() != QMessageBox::Yes)
            return;
    }

    file.open(QIODevice::WriteOnly);
    if (!file.isOpen()) {
        QMessageBox msg;
        msg.setText("Failed to open " + name + "\n" + file.errorString());
        msg.exec();
        return;
    }

    QTextStream out(&file);

    QList<quint64> keys = userLabels.keys();
    for (int i=0; i<keys.size(); i++) {
        out << hex << showbase << keys.at(i) << " "
            << userLabels.value(keys.at(i)) << endl;
    }

    if (file.error()) {
        QMessageBox msg;
        msg.setText("Error: " + file.errorString());
        msg.exec();
    }

    file.close();
}

void labelswindow::on_importButton_clicked() {
    QString name = QFileDialog::getOpenFileName(this, "Import labels from...");

    if (name.isEmpty()) return;

    QFile file(name);
    if (!file.exists()) {
        QMessageBox msg;
        msg.setText("No such file!");
        msg.exec();
        return;
    }

    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        QMessageBox msg;
        msg.setText("Failed to open " + name + "\n" + file.errorString());
        msg.exec();
        return;
    }

    QTextStream in(&file);

    quint64 addr;
    QString label;

    while (!in.atEnd()) {
        in >> addr >> label;
        if (in.atEnd()) break;
        if (!label.isEmpty())
            userLabels.insert(addr, label);
    }
    file.close();
    showUserLabels();
}
