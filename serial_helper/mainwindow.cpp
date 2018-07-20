#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>


// 提取空格号前所有字符（即所需数字量）
double extract_number(const QString& str){
    QString _str = str;
//    qDebug()<<_str;
    int index = _str.indexOf(" ");
//    qDebug()<<"extra"<<index;
    QString num = _str.mid(0, index);
//    qDebug()<<"extra"<<num;
    return num.toDouble();
}

// 十六进制转十进制辅助
char ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30; //'0'的ASCII是0x30，以此类推
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10; //a表示10，b表示11，以此类推
    else return (-1);
}

//  16进制转换函数
QByteArray QString2Hex(QString str)
{
    QByteArray senddata;
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);  // 理论上，String转为HEX发送，最多仅需要原字长的1/2，如12，ASCII逐字发送为“1”、“2”共计2bits
                             // 而HEX直接发送 0C，1bit
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata == 16) || (lowhexdata == 16) || (hexdata == -1) || (lowhexdata == -1))  // 拒绝非法字符
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        qDebug()<<"hexdata"<<hexdata;
        qDebug()<<"senddata[hexdatalen]"<<senddata[hexdatalen];
        qDebug()<<"senddata"<<senddata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);  // 考虑到空格情况，可进一步减少字长
    qDebug()<<"senddata.resize"<<senddata;
    return senddata;
}

bool check_HexData(const QString& HexData){
    QString data = HexData;
    int check = 1;
    for(int i=0;i<=data.length();i++){
        char x = data[i].toLatin1();
        check = ConvertHexChar(x);
        if(check<0)
            break;
    }
    if(check<0)
        return false;
    else
        return true;
}


QString MainWindow::readASCII(const QByteArray& serialData){
    QByteArray data = serialData;
    qDebug()<<"readASCII"<<QString(data);
    return QString(data);
}

QString MainWindow::readHEX(const QByteArray &serialData){
    QByteArray data = serialData;
    QString text = QString((data.toHex())).toUpper();
    qDebug()<<"text"<<text;
    qDebug()<<"toInt"<<(data.toHex()).toInt();
    QString textView;
    // 每一位十六进制符号用空格隔开
    for(int i = 0; i <= text.length(); i+=2){
        textView += text.mid(i,2) + QString(" ");
    }
    qDebug()<<"textvie"<<textView;
    return textView.mid(0,textView.length()-1);
}

qint64 MainWindow::writeASCII(const QString &sendData){
    QString send = sendData;
    qDebug()<<"send.toLocal8Bit(): "<<send.toLocal8Bit();
    qDebug()<<"send.toLocal8bit().data()"<<send.toLocal8Bit().data();
    return this->serial->write(send.toLocal8Bit().data());  //.tolocal8bit(),转为返回字符串的本地8位表示形式。
                                                            // 如果字符串包含本地8位编码不支持的字符，则返回的字节数组是不确定的。
                                                            // .data()，返回 char*指针
}

qint64 MainWindow::writeHEX(const QString &sendData){
    QString send = sendData;
    QByteArray sendHEX = QString2Hex(send);
    qDebug()<<"this->serial->write(sendHEX)"<<sendHEX.toHex();
//    qDebug()<<"this->serial->write(sendHEX)"<<sendHEX;
    return this->serial->write(sendHEX);    
}

void MainWindow::autoSendData(){
    if(this->serial->isOpen()){
        QString sendData = this->ui->textEdit->toPlainText();
        if(this->ui->WriRadioASCII->isChecked()){
            qint64 num = this->writeASCII(sendData);
            qDebug()<<"write: "<<num;
            if(this->ui->ViewSendCheckBox->isChecked()){
                this->ui->textBrowser->append("\n");
                QDateTime currentTime = QDateTime::currentDateTime();
                QString current_date = currentTime.toString("yyyy-MM-dd");
                QString current_time = currentTime.toString("hh:mm:ss.zzz ");
                this->ui->textBrowser->append("Sent at "+current_date+" "+current_time+":");
                this->ui->textBrowser->append(sendData);
            }
        }
        else if(this->ui->WriRadioHex->isChecked()){
            qint64 num = this->writeHEX(sendData);
            qDebug()<<"write: "<<num;
            if(this->ui->ViewSendCheckBox->isChecked()){
                QByteArray hexSendData = QString2Hex(sendData);
                QString viewHexData = this->readHEX(hexSendData);
                this->ui->textBrowser->append("\n");
                QDateTime currentTime = QDateTime::currentDateTime();
                QString current_date = currentTime.toString("yyyy-MM-dd");
                QString current_time = currentTime.toString("hh:mm:ss.zzz ");
                this->ui->textBrowser->append("Sent at "+current_date+" "+current_time+":");
                this->ui->textBrowser->append(viewHexData);
                qDebug()<<"hexSendData"<<hexSendData;
            }
        }
    }
    else if(!this->serial->isOpen()){
        this->sendTimer->stop();
        delete sendTimer;
        this->ui->AutoCheckBox->setChecked(false);
        this->ui->SendSerialButton->setEnabled(true);
        this->ui->SendTimeSpinBox->setEnabled(true);
        QMessageBox::warning(this,"Warning",QString("无法与串口通讯\n请检查串口是否一打开"));
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->serial = new QSerialPort(this);
    // 使用QList管理SignalGroupBox下的Combox控件，对其初始化
    this->setSerialCombox = ui->SignalGroupBox->findChildren<QComboBox*>();
    this->ui->newlineCheckBox->setChecked(true);
    this->ui->AutoCheckBox->setChecked(false);
    // 串口接收设置超时处理，当接收时间大于设定值再一并取出处理
    this->serialTimer = new QTimer(this);
    this->sendTimer = new QTimer(this);
    // 初始化ASCII、HEX选择项
    this->ui->RecRadioASCII->setChecked(true);
    this->ui->WriRadioASCII->setChecked(true);
    // Disable打开串口键
    this->ui->openSerialButton->setEnabled(false);
    this->ui->Comb_serialPort->clear();  // 测试用
    // 使用QMap保存串口数据位
    this->DatabitsMap.insert(5,QSerialPort::Data5);
    this->DatabitsMap.insert(6,QSerialPort::Data6);
    this->DatabitsMap.insert(7,QSerialPort::Data7);
    this->DatabitsMap.insert(8,QSerialPort::Data8);
    // 使用QMap保存串口校验方式
    this->ParityMap.insert(this->ui->Comb_parityBits->itemText(0),QSerialPort::NoParity);
    this->ParityMap.insert(this->ui->Comb_parityBits->itemText(1),QSerialPort::OddParity);
    this->ParityMap.insert(this->ui->Comb_parityBits->itemText(2),QSerialPort::EvenParity);
    this->ParityMap.insert(this->ui->Comb_parityBits->itemText(3),QSerialPort::SpaceParity);
    this->ParityMap.insert(this->ui->Comb_parityBits->itemText(4),QSerialPort::MarkParity);
    // 使用QMap保存串口停止位
    this->StopMap.insert(1.0,QSerialPort::OneStop);
    this->StopMap.insert(1.5,QSerialPort::OneAndHalfStop);
    this->StopMap.insert(2.0,QSerialPort::TwoStop);
    // 使用QMAP保存串口流控制
    this->FlowControlMap.insert(0,QSerialPort::NoFlowControl);
    this->FlowControlMap.insert(1,QSerialPort::HardwareControl);
    this->FlowControlMap.insert(2,QSerialPort::SoftwareControl);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 保存选择的串口设置项
void MainWindow::setSerial(){
    //读取combox选项值
    this->SerialIndx = ui->Comb_serialPort->currentText();
    this->BaudRate = (ui->Comb_baudRate->currentIndex()+1)*9600;
    this->parityBit = ui->Comb_parityBits->currentText();
    this->DataBits = int(extract_number(ui->Comb_dataBits->currentText()));
    this->StopBits = extract_number(ui->Comb_stopBits->currentText());
    this->FlowContrlIndx = ui->Comb_flowContrl->currentIndex();
    QString serial_tip = QString("正在打开串口%1\n").arg(this->SerialIndx);
    this->ui->textBrowser->append(serial_tip);
}


// 打开串口
bool MainWindow::openSerial(){
    QMap<int, QSerialPort::DataBits> map;
    map.insert(5,QSerialPort::Data5);
    this->serial = new QSerialPort(this);
    this->serial->setPortName(this->SerialIndx);
    if(this->serial->open(QIODevice::ReadWrite)){
        // 设置串口波特率
        this->serial->setBaudRate(this->BaudRate);
        this->ui->textBrowser->append(QString("设定波特率： %1").arg(this->BaudRate));
        // 设置串口数据位
        this->serial->setDataBits(this->DatabitsMap[this->DataBits]);
        this->ui->textBrowser->append(QString("设定数据位： %1字节").arg(this->DataBits));
        // 设置串口校验位
        this->serial->setParity(this->ParityMap[this->parityBit]);
        this->ui->textBrowser->append(QString("设定校验位： %1").arg(this->parityBit));
        // 设置停止位
        this->serial->setStopBits(this->StopMap[this->StopBits]);
        this->ui->textBrowser->append(QString("设定停止位： %1字节").arg(this->StopBits));
        // 设置流控制
        this->serial->setFlowControl(this->FlowControlMap[this->FlowContrlIndx]);
        this->ui->textBrowser->append(QString("设定流控制： %1").arg(this->ui->Comb_flowContrl->itemText(this->FlowContrlIndx)));
        return true;
    }
    else
        return false;

}

// 启用定时器并接收信息
void MainWindow::timerSerialRead(){    
    this->mSerialRecData.append(this->serial->readAll());
    this->serialTimer->start(this->ui->RecTimerSpinBox->value());
    qDebug()<<"Timer start";
}

// 接收串口信息
void MainWindow::readSerialData(){
    // 停止计时器
    this->serialTimer->stop();
    QByteArray serialData = this->mSerialRecData;
    this->mSerialRecData.clear();
    QString textView;
    if(this->ui->RecRadioASCII->isChecked()){
        textView = this->readASCII(serialData);
    }
    else if(this->ui->RecRadioHex->isChecked()){
        textView = this->readHEX(serialData);
    }    
    if(this->ui->newlineCheckBox->isChecked()){
        this->ui->textBrowser->append("\n");
        QDateTime currentTime = QDateTime::currentDateTime();
        QString current_date = currentTime.toString("yyyy-MM-dd");
        QString current_time = currentTime.toString("hh:mm:ss.zzz ");
        this->ui->textBrowser->append("Received at "+current_date+" "+current_time+":");
        this->ui->textBrowser->append(textView);
    }
    else
        this->ui->textBrowser->insertPlainText(textView);

    qDebug()<<"QByte"<<serialData;
    qDebug()<<"QByte to int"<<(serialData.toHex()).toInt();
    qDebug()<<"QByte to Hex"<<serialData.toHex();
    qDebug()<<"QByte to QString"<<QString(serialData);
}

void MainWindow::on_openSerialButton_clicked()
{
    if(ui->openSerialButton->text()==QString("打开串口")){
        ui->openSerialButton->setText(QString("关闭串口"));
        ui->scanSerialPortButton->setEnabled(false);
        // Combox控件失效
        QList<QComboBox*>::iterator i;
        for(i=setSerialCombox.begin();i!=setSerialCombox.end();i++){
            (*i)->setEnabled(false);            
        }
        // 保存设置选项
        MainWindow::setSerial();
        // 打开串口
        if(MainWindow::openSerial()){
            this->ui->textBrowser->append(QString("%1已打开\n\n").arg(this->SerialIndx));
            this->serialTimer = new QTimer(this);   // 必须新注册一个timer，用该新注册timer做处理
                                                    // 防止多次触发timeout()的事情
            // connect收到串口信号，触发处理
            QObject::connect(this->serial, SIGNAL(readyRead()),this,SLOT(timerSerialRead()));
            QObject::connect(this->serialTimer,SIGNAL(timeout()),this,SLOT(readSerialData()));
//            qDebug()<<"pinoutSignals"<<serial->pinoutSignals();
        }
        else{
            this->ui->textBrowser->append(QString("%1打开失败\n\n").arg(this->SerialIndx));
        }
    }

    else if(ui->openSerialButton->text()==QString("关闭串口")){
        ui->openSerialButton->setText(QString("打开串口"));
        this->serialTimer->stop();  // 若已回收了该指针，则无需停止，反之亦然，当然，既停止又回收也无妨
        delete this->serialTimer;  // 回收该指针，这一步非常重要。如果上述两步都没有进行，那么，当串口通讯错误而再次打开串口时
                                   // 就会一下子触发好几个timeout()信号（即原有的错误通讯也能触发timer启动，因收不到信号而无法stop
                                   // 故而timeout()信号屡屡触发
                                   // 若timer在使用时重新注册过，亦可，但最好还是要回收内存
        this->ui->Comb_serialPort->clear();
        ui->openSerialButton->setEnabled(false);
        ui->scanSerialPortButton->setEnabled(true);
        QList<QComboBox*>::iterator i;
        for(i=setSerialCombox.begin();i!=setSerialCombox.end();i++)
            (*i)->setEnabled(true);
        this->serial->close();
//        delete this->serial;
        QString serial_tip = QString("串口%1已关闭\n\n").arg(this->SerialIndx);
        this->ui->textBrowser->append(serial_tip);
    }
}

void MainWindow::on_clearRecButton_clicked()
{
    this->ui->textBrowser->clear();
}

void MainWindow::on_scanSerialPortButton_clicked()
{
    this->ui->textBrowser->append(QString("开始扫描可用串口 ..."));
    this->ui->Comb_serialPort->clear();
    foreach (const QSerialPortInfo& info, QSerialPortInfo::availablePorts()) {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite)){
            this->ui->textBrowser->append("\n");            
            this->ui->Comb_serialPort->addItem(info.portName());
            this->ui->textBrowser->append("Port Name: "+info.portName());
            this->ui->textBrowser->append("Dev Inf: "+info.description());
            this->ui->textBrowser->append("Dev Man: "+info.manufacturer());
            QString isbusy;
            if(info.isBusy())
                isbusy = "Yes";
            else {
                isbusy = "No";
            }
            this->ui->textBrowser->append("Busy: "+isbusy);
            this->ui->textBrowser->append("\n");
            serial.close();
        }
    }
    if(this->ui->Comb_serialPort->count()==0){
        QMessageBox::warning(this, "Warning", QString("无可用串口\n请检查当前串口配置"));
        this->ui->textBrowser->append(QString("无可用串口\n"));
        }
    else if(this->ui->Comb_serialPort->count()>0){
        this->ui->textBrowser->append(QString("扫描结束\n"));
        this->ui->openSerialButton->setEnabled(true);
    }
}

// textbroser自动刷新
void MainWindow::on_textBrowser_textChanged()
{
    this->ui->textBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::on_AutoCheckBox_pressed()
{
    if(!this->ui->AutoCheckBox->isChecked()){
        this->ui->SendSerialButton->setEnabled(false);
        this->sendTimer = new QTimer(this);
        this->sendTimer->start(this->ui->SendTimeSpinBox->value());
        this->ui->SendTimeSpinBox->setEnabled(false);
        QObject::connect(this->sendTimer,SIGNAL(timeout()),this,SLOT(autoSendData()));
    }
    else if(this->ui->AutoCheckBox->isChecked()){
        this->ui->SendSerialButton->setEnabled(true);
        this->ui->SendTimeSpinBox->setEnabled(true);
        this->sendTimer->stop();
    }
}

void MainWindow::on_clearSendButton_clicked()
{
    this->ui->textEdit->clear();
    char a[5]="Bbcd";
    int b;
    b = a[0]-'A'+10;

//    char aa = a.toLatin1();
    qDebug()<<"a-'A'+10"<<a[0]-'A'+10;
    qDebug()<<"char b"<<(char)b;
//    qDebug()<<"toLatin1()"<<a.toLatin1().toHex();
}


void MainWindow::on_SendSerialButton_clicked()
{
    if(this->serial->isOpen()){
        QString sendData = this->ui->textEdit->toPlainText();
        if(this->ui->WriRadioASCII->isChecked()){
            qint64 num = this->writeASCII(sendData);
            qDebug()<<"write: "<<num;
            if(this->ui->ViewSendCheckBox->isChecked()){
                this->ui->textBrowser->append("\n");
                QDateTime currentTime = QDateTime::currentDateTime();
                QString current_date = currentTime.toString("yyyy-MM-dd");
                QString current_time = currentTime.toString("hh:mm:ss.zzz ");
                this->ui->textBrowser->append("Sent at "+current_date+" "+current_time+":");
                this->ui->textBrowser->append(sendData);
            }
        }
        else if(this->ui->WriRadioHex->isChecked()){
            QString checkHexData = sendData;
            qint64 num = this->writeHEX(sendData);
            qDebug()<<"write: "<<num;
            if(this->ui->ViewSendCheckBox->isChecked()){
                QByteArray hexSendData = QString2Hex(sendData);
                QString viewHexData = this->readHEX(hexSendData);
                this->ui->textBrowser->append("\n");
                QDateTime currentTime = QDateTime::currentDateTime();
                QString current_date = currentTime.toString("yyyy-MM-dd");
                QString current_time = currentTime.toString("hh:mm:ss.zzz ");
                this->ui->textBrowser->append("Sent at "+current_date+" "+current_time+":");
                this->ui->textBrowser->append(viewHexData);
                qDebug()<<"hexSendData"<<hexSendData;
            }
        }
    }
    else if(!this->serial->isOpen()){
        qDebug()<<"isopen"<<this->serial->isOpen();
        QMessageBox::warning(this, "Warning", QString("需先打开串口\n方可发送信息"));
    }


}
