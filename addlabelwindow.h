#ifndef ADDLABELWINDOW_H
#define ADDLABELWINDOW_H

#include <QDialog>

namespace Ui {
class addLabelWindow;
}

class addLabelWindow : public QDialog
{
    Q_OBJECT

public:
    explicit addLabelWindow(QWidget *parent = 0);
    ~addLabelWindow();

private slots:
    void on_pushButtonCancel_clicked();

    void on_pushButtonAdd_clicked();

private:
    Ui::addLabelWindow *ui;
};

#endif // ADDLABELWINDOW_H
