#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>

namespace Ui {
class StartDialog;
}

class StartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartDialog(QWidget *parent = 0);
    ~StartDialog();

    bool load_existing_project = false;
    bool create_new_project    = false;

private slots:
    void on_buttonBrowseExistingProject_clicked();
    void on_buttonBrowseFileDisasm_clicked();
    void on_buttonBrowseSaveFilename_clicked();
    void on_buttonLoadExistingProject_clicked();
    void on_buttonNewProject_clicked();

private:
    Ui::StartDialog *ui;
};

#endif // STARTDIALOG_H
