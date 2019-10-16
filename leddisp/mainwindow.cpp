#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        ui->m_serialPortComboBox->addItem(info.portName());

    ui->m_waitResponseSpinBox->setRange(0, 10000);
    ui->m_waitResponseSpinBox->setValue(1000);
    ui->m_serialPortComboBox->setFocus();
    setWindowTitle(tr("Blocking Master"));

    connect(&m_thread, &MasterThread::response, this, &MainWindow::showResponse);
    connect(&m_thread, &MasterThread::error, this, &MainWindow::processError);
    connect(&m_thread, &MasterThread::timeout, this, &MainWindow::processTimeout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showResponse(const QString &s)
{
    setControlsEnabled(true);
    ui->m_trafficLabel->setText(tr("Traffic, transaction #%1:"
                               "\n\r-request: %2"
                               "\n\r-response: %3")
                            .arg(++m_transactionCount)
                            .arg(ui->m_requestLineEdit->text())
                            .arg(s));
}

void MainWindow::processError(const QString &s)
{
    setControlsEnabled(true);
    ui->m_statusLabel->setText(tr("Status: Not running, %1.").arg(s));
    ui->m_trafficLabel->setText(tr("No traffic."));
}

void MainWindow::processTimeout(const QString &s)
{
    setControlsEnabled(true);
    ui->m_statusLabel->setText(tr("Status: Running, %1.").arg(s));
    ui->m_trafficLabel->setText(tr("No traffic."));
}

void MainWindow::setControlsEnabled(bool enable)
{
    ui->m_runButton->setEnabled(enable);
    ui->m_serialPortComboBox->setEnabled(enable);
    ui->m_waitResponseSpinBox->setEnabled(enable);
    ui->m_requestLineEdit->setEnabled(enable);
}

void MainWindow::on_m_runButton_clicked()
{
    setControlsEnabled(false);
    ui->m_statusLabel->setText(tr("Status: Running, connected to port %1.")
                           .arg(ui->m_serialPortComboBox->currentText()));
    m_thread.transaction(ui->m_serialPortComboBox->currentText(),
                         ui->m_waitResponseSpinBox->value(),
                         ui->m_requestLineEdit->text());
}
