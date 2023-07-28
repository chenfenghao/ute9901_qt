#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QtCharts>
using namespace QtCharts;
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
    void readSerialData();
    void sendSerialData();

    void on_pushButton_search_clicked();

    void on_pushButton_open_clicked();

    void on_radioButton_recv_hex_clicked();

    void on_radioButton_recv_ascii_clicked();

    void on_pushButton_clear_recv_clicked();

    void on_checkBox_stop_display_clicked(bool checked);

    void on_checkBox_recv_to_file_clicked(bool checked);

    void on_radioButton_send_ascii_clicked();

    void on_radioButton_send_hex_clicked();

    void on_pushButton_clear_send_clicked();

    void on_pushButton_send_clicked();

    void on_checkBox_send_cyclic_clicked(bool checked);

    void on_pushButton_read_clicked();

    void updata_plot();

    float deal_data(QString a);
private:
    Ui::MainWindow *ui;

    QSerialPort serial;

    int TXCounts;
    QLabel TXLabel;
    int RXCounts;
    QLabel RXLabel;
    QPushButton pushButton_countClear;

    bool recvToFile;
    bool display;
    bool recvASCII;

    bool sendASCII;

    QFile recvFile;

    bool sendCyclic;
    int period; //ms
    QTimer sendTimer;
    QTimer *timer_plot;
    QSplineSeries* line;
    //曲线点的最大数量
    int line_max = 720;
    //float ch1_plot;
    //float ch2_plot;
    float ch3_plot;
    //float ch4_plot;
    //绘图变量和坐标
    QChart* chart;
    QValueAxis *axisX;
    QValueAxis *axisY;
};

#endif // MAINWINDOW_H
