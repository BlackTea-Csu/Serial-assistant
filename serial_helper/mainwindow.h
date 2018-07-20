#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMainWindow>
#include <QList>
#include <QComboBox>
#include <QMessageBox>
#include <QDebug>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QIcon>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_openSerialButton_clicked();

    void on_scanSerialPortButton_clicked();

    void readSerialData();

    void timerSerialRead();

    void autoSendData();

    void on_textBrowser_textChanged();

    void on_clearRecButton_clicked();

    void on_AutoCheckBox_pressed();

    void on_clearSendButton_clicked();

    void on_SendSerialButton_clicked();

private:
    Ui::MainWindow *ui;
    QList<QComboBox*> setSerialCombox;
    QString SerialIndx;
    int BaudRate;
    QString parityBit;
    int DataBits;
    double StopBits;
    int FlowContrlIndx;
    QSerialPort *serial;  // 自有串口
    QMap<int,QSerialPort::DataBits> DatabitsMap;
    QMap<QString,QSerialPort::Parity> ParityMap;
    QMap<double,QSerialPort::StopBits> StopMap;
    QMap<int,QSerialPort::FlowControl> FlowControlMap;
    QTimer *serialTimer;
    QTimer *sendTimer;
    QByteArray mSerialRecData;


protected:
    void setSerial();
    bool openSerial();
    QString readASCII(const QByteArray& serialData);
    QString readHEX(const QByteArray& serialData);
    qint64 writeASCII(const QString& sendData);
    qint64 writeHEX(const QString& sendData);
};

#endif // MAINWINDOW_H
