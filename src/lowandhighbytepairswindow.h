#ifndef LOWANDHIGHBYTEPAIRSWINDOW_H
#define LOWANDHIGHBYTEPAIRSWINDOW_H

#include <QDialog>

namespace Ui {
class LowAndHighBytePairsWindow;
}

class LowAndHighBytePairsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LowAndHighBytePairsWindow(QWidget *parent = nullptr);
    ~LowAndHighBytePairsWindow();

    bool pairsLowLowHighHigh;

    enum {
        NoLabels        = 0,
        LocalLabels     = 1,
        GlobalLabels    = 2
    };

    int generateLabels;

public Q_SLOTS:
    void accept() override;

private:
    Ui::LowAndHighBytePairsWindow *ui;
};

#endif // LOWANDHIGHBYTEPAIRSWINDOW_H
