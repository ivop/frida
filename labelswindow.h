#ifndef LABELSWINDOW_H
#define LABELSWINDOW_H

#include <QDialog>
#include <QTableWidget>

namespace Ui {
class labelswindow;
}

class labelswindow : public QDialog
{
    Q_OBJECT

public:
    explicit labelswindow(QWidget *parent = 0);
    ~labelswindow();

private slots:
    void on_tableAutoLabels_cellChanged(int row, int column);
    void on_tableUserLabels_cellChanged(int row, int column);
    void actionChange_To_Local_Label();
    void actionChange_To_User_Label();
    void actionDelete_Label();

    void on_doneButton_clicked();
    void on_addLabelButton_clicked();
    void on_exportButton_clicked();
    void on_importButton_clicked();

private:
    Ui::labelswindow *ui;

    void showLabels(QTableWidget *t, QMap<quint64, QString> *labels);
    void showAutoLabels(void);
    void showUserLabels(void);
    void showLocalLabels(void);

};

#endif // LABELSWINDOW_H
