#ifndef CHANGESEGMENTWINDOW_H
#define CHANGESEGMENTWINDOW_H

#include <QDialog>

namespace Ui {
class changesegmentwindow;
}

class changesegmentwindow : public QDialog
{
    Q_OBJECT

public:
    explicit changesegmentwindow(QWidget *parent = 0);
    changesegmentwindow(quint64 oldaddr, quint64 *newaddr, QWidget *parent = 0);
    ~changesegmentwindow();

private slots:
    void onButtonBox_accepted();

private:
    Ui::changesegmentwindow *ui;
    quint64 *newaddress;
};

#endif // CHANGESEGMENTWINDOW_H
