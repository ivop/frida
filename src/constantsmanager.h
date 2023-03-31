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

private:
    Ui::constantsManager *ui;
};

#endif // CONSTANTSMANAGER_H
