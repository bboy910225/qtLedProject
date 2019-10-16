#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include "masterthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showResponse(const QString &s);
    void processError(const QString &s);
    void processTimeout(const QString &s);

    void on_m_runButton_clicked();

private:
    void setControlsEnabled(bool enable);

private:
    Ui::MainWindow *ui;
    MasterThread m_thread;
    int m_transactionCount = 0;
};

#endif // MAINWINDOW_H
