#ifndef JUMPTOWINDOW_H
#define JUMPTOWINDOW_H

#include <QDialog>

namespace Ui {
class jumpToWindow;
}

class jumpToWindow : public QDialog
{
    Q_OBJECT

public:
    explicit jumpToWindow(QWidget *parent = nullptr, QStringList *list = nullptr);
    ~jumpToWindow();

    QString jump_to_location;

public slots:
    void accept() override;

private:
    Ui::jumpToWindow *ui;
};

#endif // JUMPTOWINDOW_H
