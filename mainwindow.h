#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "device.h"
#include "udevrule.h"

#include <QDateTime>
#include <QMainWindow>
#include <QTextStream>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    explicit MainWindow(QWidget *parent = 0);    
    void addSymlinksToTable();
    void addPortsToTable();
    UdevRule parseUdevRule(QString line);
    ~MainWindow();    

    void clearDetailsPanel();
    void showPortDetails(QString port);
    bool validate_apply();
    void createSymlink(QString symlink, QString permisions, QString filename, Device dev);
public slots:
    void refreshSerialPortList();    
    void tableCellClick(int, int);
    void showSymlinkDetails(QString serial);
    void showPortDetails(QString serial, QString port);

private slots:
    void on_refresh_clicked();


    void on_b_delete_clicked();

    void on_b_apply_clicked();

    void on_field_rule_file_name_editingFinished();

    void on_b_generate_clicked();

private:
    Ui::MainWindow *ui;
    QList<UdevRule> ruleList;
    QList<Device> deviceList;
    int lastDetailIdx;
};

#endif // MAINWINDOW_H
