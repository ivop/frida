#ifndef COMMENTWINDOW_H
#define COMMENTWINDOW_H

#include <QDialog>

namespace Ui {
class commentwindow;
}

class commentwindow : public QDialog
{
    Q_OBJECT

public:
    explicit commentwindow(QWidget *parent = 0);
    commentwindow(QString s, QString c, QWidget *parent = 0);
    ~commentwindow();

    QString retrieveComment(void);

private:
    Ui::commentwindow *ui;
    quint64 address;
};

#endif // COMMENTWINDOW_H
