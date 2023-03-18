#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetSelectionRange>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showSegments(void);
    void showHex(void);
    void showAscii(void);
    void showDisassembly(void);

    void closeEvent (QCloseEvent *event);

public slots:
    void actionTrace(void);
    void actionSet_To_Undefined(void);
    void actionSet_To_Byte(void);
    void actionSet_To_Code(void);
    void actionSet_To_WordLE(void);
    void actionSet_To_WordBE(void);
    void actionSet_To_DWordLE(void);
    void actionSet_To_DWordBE(void);
    void actionSet_To_QWordLE(void);
    void actionSet_To_QWordBE(void);
    void actionSet_To_XWordLE(void);
    void actionSet_To_XWordBE(void);
    void actionSet_To_ASCII(void);
    void actionSet_To_ATASCII(void);
    void actionSet_To_PETSCII(void);
    void actionSet_To_ANTIC_Screen_Codes(void);
    void actionSet_To_CBM_Screen_Codes(void);
    void actionSet_Flag_Labelled(void);
    void actionSet_Flag_Low_Byte(void);
    void actionSet_Flag_High_Byte(void);
    void actionSet_Flag_Clear(void);
    void actionComment(void);
    void actionDelete_Segment(void);
    void actionChange_Start_Address(void);
    void actionAdd_Label(void);

private slots:
    void linkHexASCIISelection(void);
    void linkASCIIHexSelection(void);
    void linkSelections(QTableWidget *from, QTableWidget *to);
    void onHexSectionClicked(int index);
    void onDisassemblySectionClicked(int index);

    void onLabelsButton_clicked();
    void onExitButton_clicked();
    void onSaveButton_clicked();
    void onExportAsmButton_clicked();

    void onTableSegments_itemSelectionChanged();
    void onTableSegments_cellChanged(int row, int column);
    void onTableDisassembly_cellChanged(int row, int column);

    void onComboFonts_activated(int index);
    void onRadioButtonFullscreen_toggled(bool checked);
    void onComboBox_currentIndexChanged(int index);
    void onTableDisassembly_doubleClicked(const QModelIndex &index);

    void rememberValue(int value);

private:
    Ui::MainWindow *ui;
    void Set_To_Foo(QList<QTableWidgetSelectionRange> ranges, quint8 datatype);
    void Set_Flag(QList<QTableWidgetSelectionRange> ranges, quint8 flag);
    void Set_Flag_Low_or_High_Byte(bool bLow);
};

#endif // MAINWINDOW_H
