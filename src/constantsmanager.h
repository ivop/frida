#ifndef CONSTANTSMANAGER_H
#define CONSTANTSMANAGER_H

#include <QDialog>

namespace Ui {
class constantsManager;
}

class constantsManager : public QDialog
{
    Q_OBJECT

public:
    explicit constantsManager(QWidget *parent = nullptr);
    ~constantsManager();

private Q_SLOTS:
    void onGroups_itemSelectionChanged(void);

    void onGroups_cellChanged(int row, int column);
    void onValues_cellChanged(int row, int column);

    void actionDeleteGroup(void);
    void actionDeleteValue(void);

private:
    Ui::constantsManager *ui;
    void showGroups();
};

#endif // CONSTANTSMANAGER_H
