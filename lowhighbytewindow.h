#ifndef LOWHIGHBYTEWINDOW_H
#define LOWHIGHBYTEWINDOW_H

#include <QDialog>

namespace Ui {
class lowhighbytewindow;
}

class lowhighbytewindow : public QDialog
{
    Q_OBJECT

public:
    explicit lowhighbytewindow(QWidget *parent = 0);
    lowhighbytewindow(quint64 location, bool low, quint8 byte,
                      quint16 *fulladdr, QWidget *parent = 0);
    ~lowhighbytewindow();

private slots:
    void onButtonBox_accepted();

private:
    Ui::lowhighbytewindow *ui;
    quint16 *fulladdr;
};

#endif // LOWHIGHBYTEWINDOW_H
