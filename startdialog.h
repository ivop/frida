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
    void onButtonBrowseExistingProject_clicked();
    void onButtonBrowseFileDisasm_clicked();
    void onButtonLoadExistingProject_clicked();
    void onButtonNewProject_clicked();

private:
    Ui::StartDialog *ui;
};

#endif // STARTDIALOG_H
