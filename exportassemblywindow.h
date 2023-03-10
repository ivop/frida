#ifndef EXPORTASSEMBLYWINDOW_H
#define EXPORTASSEMBLYWINDOW_H

#include <QDialog>

namespace Ui {
class exportAssemblyWindow;
}

class exportAssemblyWindow : public QDialog
{
    Q_OBJECT

public:
    explicit exportAssemblyWindow(QWidget *parent = nullptr);
    ~exportAssemblyWindow();


    int asm_format;

public slots:
    void accept() override;

private:
    Ui::exportAssemblyWindow *ui;
};

enum {
    ASM_FORMAT_VERBATIM = 0,
    ASM_FORMAT_MADS
};

#endif // EXPORTASSEMBLYWINDOW_H
