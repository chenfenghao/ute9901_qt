#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QTime>
#include <QDate>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QBarSet>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Temp&Plot");
    QIcon icon(":/new/prefix1/pngs.png");
    setWindowIcon(icon);
    timer_plot = new QTimer(this);
    connect(timer_plot,SIGNAL(timeout()),this,SLOT(updata_plot()));
    line = new QSplineSeries(this);
    chart = new QChart();
    chart->addSeries(line);
    axisX = new QValueAxis(this);
    axisY = new QValueAxis(this);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    line->attachAxis(axisX);
    line->attachAxis(axisY);
//    RXCounts = 0;
//    TXCounts = 0;
//    TXLabel.setText("0");
//    RXLabel.setText("0");
//    this -> ui -> statusBar -> addPermanentWidget(new QLabel("TX"));
//    this -> ui -> statusBar -> addPermanentWidget(&TXLabel);
//    this -> ui -> statusBar -> addPermanentWidget(new QLabel("RX"));
//    this -> ui -> statusBar -> addPermanentWidget(&RXLabel);
//    pushButton_countClear.setText("Reset");
//    connect(&pushButton_countClear,&QPushButton::clicked,this,[=](){
//        RXCounts = 0;
//        TXCounts = 0;
//        TXLabel.setText("0");
//        RXLabel.setText("0");
//    });
//    this -> ui -> statusBar -> addPermanentWidget(&pushButton_countClear);
//    QLCDNumber *m_pLcdTime = new QLCDNumber(this);
    QPalette lcdpat = ui->lcdNumber_1->palette();
    lcdpat.setColor(QPalette::Normal,QPalette::WindowText,Qt::red);
    ui->lcdNumber_1->setPalette(lcdpat);
    ui->lcdNumber_2->setPalette(lcdpat);
    ui->lcdNumber_3->setPalette(lcdpat);
    ui->lcdNumber_4->setPalette(lcdpat);
    connect(&serial,&QSerialPort::readyRead,this,&MainWindow::readSerialData);
    connect(&serial,&QSerialPort::errorOccurred,this,[=](QSerialPort::SerialPortError portErr){
        this -> ui -> statusBar -> showMessage(QString("Serial error %1").arg(portErr),1000);
    });
    sendASCII = true;
    recvToFile = false;
    display = true;
    recvASCII = false;

    sendCyclic = false;
    connect(&sendTimer,&QTimer::timeout,this,&MainWindow::sendSerialData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_search_clicked()
{
    this -> ui  -> comboBox_name -> clear();
    foreach (QSerialPortInfo avaiablePort, QSerialPortInfo::availablePorts()) {
        this -> ui -> comboBox_name -> addItem(avaiablePort.portName());
    }
}


void MainWindow::on_pushButton_open_clicked()
{
    if(this->ui->pushButton_open->text() == "Open")
    {
        serial.setPortName(this->ui->comboBox_name->currentText());
        serial.setBaudRate(this->ui->comboBox_baud->currentText().toInt());
        switch (this -> ui -> comboBox_paity -> currentIndex()){
        case 0: serial.setParity(QSerialPort::NoParity); break;
        case 1: serial.setParity(QSerialPort::EvenParity); break;
        case 2: serial.setParity(QSerialPort::OddParity); break;
        case 3: serial.setParity(QSerialPort::SpaceParity); break;
        case 4: serial.setParity(QSerialPort::MarkParity); break;
        default: serial.setParity(QSerialPort::UnknownParity); break;
        }
        switch (this->ui->comboBox_dataBits->currentText().toInt()){
        case 5: serial.setDataBits(QSerialPort::Data5); break;
        case 6: serial.setDataBits(QSerialPort::Data6); break;
        case 7: serial.setDataBits(QSerialPort::Data7); break;
        case 8: serial.setDataBits(QSerialPort::Data8); break;
        default: serial.setDataBits(QSerialPort::UnknownDataBits); break;
        }
        switch (this->ui->comboBox_stopBits->currentIndex()){
        case 0: serial.setStopBits(QSerialPort::OneStop); break;
        case 1: serial.setStopBits(QSerialPort::OneAndHalfStop); break;
        case 2: serial.setStopBits(QSerialPort::TwoStop); break;
        default: serial.setStopBits(QSerialPort::UnknownStopBits); break;
        }
        serial.setFlowControl(QSerialPort::NoFlowControl);

        if(serial.open(QIODevice::ReadWrite))
        {
            this -> ui -> pushButton_search -> setEnabled(false);
            this -> ui -> comboBox_name -> setEnabled(false);
            this -> ui -> comboBox_baud -> setEnabled(false);
            this -> ui -> comboBox_paity -> setEnabled(false);
            this -> ui -> comboBox_dataBits -> setEnabled(false);
            this -> ui -> comboBox_stopBits -> setEnabled(false);
            this -> ui -> pushButton_open -> setText("Close");
            QSerialPortInfo serialInfo(serial);
        }else {
            QMessageBox::warning(this,"Open Error","Serialport Open Error!");
        }
    }else{
        serial.close();
        this -> ui -> pushButton_search -> setEnabled(true);
        this -> ui -> comboBox_name -> setEnabled(true);
        this -> ui -> comboBox_baud -> setEnabled(true);
        this -> ui -> comboBox_paity -> setEnabled(true);
        this -> ui -> comboBox_dataBits -> setEnabled(true);
        this -> ui -> comboBox_stopBits -> setEnabled(true);
        this -> ui -> pushButton_open -> setText("Open");
    }
}

void MainWindow::readSerialData()
{
    QByteArray recvData = serial.readAll();

//    RXCounts += recvData.length();
//    RXLabel . setText(QString::number(RXCounts));
    QTime current_time =   QTime::currentTime();
    QDate current_data =   QDate::currentDate();
    QString newData;

    if(recvASCII)
    {
        newData = QString(recvData);
    }
    else
    {
        newData = QString(recvData.toHex());
}
    this -> ui -> statusBar -> showMessage(newData);
    if(display)
    {
//        this -> ui -> textBrowser -> append(newData);
        QString time =current_time.toString("hh：mm：ss");
        QString show =QString("【%1】 %2").arg(time,newData);
        this -> ui -> textBrowser ->setPlainText(show);
        this -> ui -> statusBar -> showMessage(show);
    }
//        qDebug()<<recvData;
//        qDebug()<<newData.midRef(0,2);
//        qDebug()<<newData.midRef(2,8);
//        qDebug()<<newData.midRef(10,8);
//        qDebug()<<newData.midRef(18,8);
//        qDebug()<<newData.midRef(26,8);
//        qDebug()<<newData.midRef(34,8);
       QString c1 = newData.midRef(2,8).toString();
       QString c2 = newData.midRef(10,8).toString();
       QString c3 = newData.midRef(18,8).toString();
       QString c4 = newData.midRef(26,8).toString();
       QString c5 = newData.midRef(34,8).toString();

//        if(newData.endsWith(";\n")==true && newData.startsWith("#"))
//            newData=newData.mid(1,newData.size()-3);
//            QStringList list = newData.split(",", QString::SkipEmptyParts);
//            qDebug()<<list;
//            float ch1 = list.value(0).toFloat();
//            float ch2 = list.value(1).toFloat();
//            float ch3 = list.value(2).toFloat();
//            float ch4 = list.value(3).toFloat();
//            ch3_plot = ch1;

            bool ok;
            float ch1 = deal_data(c1);
            float ch2 = deal_data(c2);
            float ch3 = deal_data(c3);
            float ch4 = deal_data(c4);
            ui->lcdNumber_1->display(ch1);//ch1
            ui->lcdNumber_2->display(ch2);//ch2
            ui->lcdNumber_3->display(ch3);//ch3
            ui->lcdNumber_4->display(ch4);//ch4
            ch3_plot = ch1;


            if(recvToFile)
            {
                QTextStream out(&recvFile);
                out<<current_data.toString("yy年MM月dd日")<<current_time.toString("hh时mm分ss秒")<<","<<ch1<<","<<ch2<<","<<ch3<<","<<ch4<<endl;
                //out<<current_data.toString("yy年MM月dd日")<<current_time.toString("hh时mm分ss秒")<<","<<ch1<<","<<ch2<<","<<ch3<<","<<ch4<<endl;
            }
}

float MainWindow::deal_data(QString a)
{
    float result=0.0;
    if(a[0]=="1")
    {
        result = a[1].digitValue()+a[3].digitValue()*0.1+a[5].digitValue()*0.01+a[7].digitValue()*0.001;
    }
    if(a[2]=="1")
    {
        result = a[1].digitValue()*10+a[3].digitValue()*1.0+a[5].digitValue()*0.1+a[7].digitValue()*0.01;
    }
    if(a[4]=="1")
    {
        result = a[1].digitValue()*100.0+a[3].digitValue()*10.0+a[5].digitValue()+a[7].digitValue()*0.1;
    }
    if(a[6]=="1")
    {
        result = a[1].digitValue()*1000.0+a[3].digitValue()*100.0+a[5].digitValue()*10.0+a[7].digitValue()*1.0;
    }
    return result;
}

void MainWindow::on_radioButton_recv_hex_clicked()
{
    recvASCII = false;
}

void MainWindow::on_radioButton_recv_ascii_clicked()
{
    recvASCII = true;
}

void MainWindow::on_pushButton_clear_recv_clicked()
{
    this -> ui -> textBrowser -> clear();
}

void MainWindow::on_checkBox_stop_display_clicked(bool checked)
{
    display = !checked;
}

void MainWindow::on_checkBox_recv_to_file_clicked(bool checked)
{
    if(checked)
    {
        recvFile.setFileName(QString("功率计%1_%2.csv").arg(QDate::currentDate().toString("yy_MM_dd"),QTime::currentTime().toString("hh_mm_ss")));
        if(!recvFile.open(QIODevice::WriteOnly))
            this -> ui -> statusBar -> showMessage("File open failed, try again!",1000);
        else{
            recvToFile = true;
        }
    }else{
        recvToFile = false;
        recvFile.close();
    }
}

void MainWindow::on_radioButton_send_ascii_clicked()
{
    if(!sendASCII)
    {
        QString hexStr = this -> ui -> textEdit -> toPlainText();
        QString str = QByteArray::fromHex(hexStr.toLatin1());
        this -> ui -> textEdit -> setText(str);
        sendASCII = true;
    }
}

void MainWindow::on_radioButton_send_hex_clicked()
{
    if(sendASCII)
    {
        QString str = this -> ui -> textEdit -> toPlainText();
        QString hexStr = str.toLatin1().toHex(' ').toUpper();
        this -> ui -> textEdit -> setText(hexStr);
        sendASCII = false;
    }
}

void MainWindow::on_pushButton_clear_send_clicked()
{
    this -> ui -> textEdit ->clear();
}

void MainWindow::on_pushButton_send_clicked()
{
    if(this -> ui -> pushButton_send -> text() == "Send")
    {
        if(serial.isOpen())
        {

            if(sendCyclic)
            {
                sendTimer.start(period);
                this -> ui -> pushButton_send -> setText("Stop");
                this -> ui -> checkBox_send_cyclic -> setEnabled(false);
                this -> ui -> lineEdit_send_period -> setEnabled(false);
                  timer_plot->start(5000);//启动定时器
            }else{
                sendSerialData();
            }
        }
        else {
            this -> ui -> statusBar -> showMessage("Serial closed, please open first",1000);
        }
    }
    else if(this -> ui -> pushButton_send -> text() == "Stop") {
        sendTimer.stop();
        this -> ui -> pushButton_send -> setText("Send");
        this -> ui -> checkBox_send_cyclic -> setEnabled(true);
        this -> ui -> lineEdit_send_period -> setEnabled(true);
        timer_plot->stop();//关闭定时器
    }
}

void MainWindow::on_checkBox_send_cyclic_clicked(bool checked)
{
    if(checked)
    {
        if(this->ui->lineEdit_send_period -> text().isEmpty())
        {
            QMessageBox::information(this,"Waring","Please edit period first");
            this -> ui -> checkBox_send_cyclic -> setChecked(false);
        }else {
            period = this->ui->lineEdit_send_period -> text().toInt();
            sendCyclic = true;
        }
    }else{
        sendCyclic = false;
    }
}

void MainWindow::sendSerialData()
{
    QByteArray sendData;
    if(sendASCII)
    {
        sendData = this -> ui -> textEdit -> toPlainText().toLatin1();
    }else{
        sendData = QByteArray::fromHex(this -> ui -> textEdit -> toPlainText().toLatin1());
    }
    serial.write(sendData);
}

void MainWindow::on_pushButton_read_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Please select file","./","TXT(*.txt)");
    QFile sendFile(fileName);
    if(!sendFile.open(QIODevice::ReadOnly))
        QMessageBox::warning(this,"Error","File open failed, please try again");
    QString sendStr = sendFile.readAll();
    this -> ui -> textEdit -> setText(sendStr);
}


void MainWindow::updata_plot()
{
    QVector<QPointF> list;
    QVector<QPointF> newlist;
    list = line->pointsVector();//获取现在图中列表
    if (list.size() < line_max)
    {
        //保持原来
        newlist = list;
    }
    else
    {
        //错位移动
        for(int i =1 ; i< list.size();i++)
        {
            newlist.append(QPointF(i-1,list.at(i).y()));
        }
    }
    newlist.append(QPointF(newlist.size(),ch3_plot));//最后补上新的数据
    line->replace(newlist);//替换更新


    line->setName("temp_fridge");//设置曲线名称
    line->setPen(QColor(0, 0, 255));//设置曲线颜色
    line->setUseOpenGL(true);//openGl 加速

    chart->setTitle("Temp Data");//设置图标标题
    chart->removeSeries(line);
    chart->addSeries(line);
    chart->createDefaultAxes();//设置坐标轴


//    axisX->setRange(-80,20);//范围
//    axisX->setTitleText("times(secs)");//标题
//    axisX->setTickCount(10);//分隔个数
//    axisX->setLineVisible(true);//可视化
//    axisX->setLinePenColor(Qt::blue);//颜色

//    axisY->setRange(-90,20);
//    axisY->setTitleText("value");
//    axisY->setTickCount(6);
//    axisY->setLineVisible(true);
//    axisY->setLinePenColor(Qt::blue);

//    chart->setAxisX(axisX,line);
//    chart->setAxisY(axisY,line);

    ui->widget_plot->setChart(chart);
}
