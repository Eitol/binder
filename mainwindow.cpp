#include "mainwindow.h"
#include "udevrule.h"
#include "udevrule.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QtSerialPort/QSerialPortInfo>


#define BLANK_STRING "N/A"
#define UDEV_RULES_DIR "/etc/udev/rules.d/"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lastDetailIdx=-1;
    connect(ui->table, SIGNAL(cellClicked(int, int)), this, SLOT(tableCellClick(int, int)));
    this->refreshSerialPortList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshSerialPortList()
{
    ruleList.clear();
    deviceList.clear();
    ui->table->setRowCount(0);
    this->addPortsToTable();
    if (ui->cb_disconected->isChecked()){
        this->addSymlinksToTable();
    }
}

void MainWindow::clearDetailsPanel()
{
    ui->field_symlink->clear();
    ui->field_rule_file_name->clear();
    ui->field_permissions->clear();
    ui->view_details->clear();
}

void MainWindow::tableCellClick(int, int)
{    
    QModelIndex currentIndex = ui->table->currentIndex();
    if (lastDetailIdx == currentIndex.row()){
        return;
    }
    lastDetailIdx=currentIndex.row();
    clearDetailsPanel();
    QString serial = ui->table->item(currentIndex.row(), 0)->text();
    if (serial==BLANK_STRING || serial.size()==0){
        return;
    }
    QString port = ui->table->item(currentIndex.row(), 1)->text();
    if (port.indexOf("tty")==-1){
        showSymlinkDetails(serial);
    }else{
        showPortDetails(serial,port);
    }
    ui->b_delete->setEnabled(ui->field_symlink->text()!=0);
}

void MainWindow::showSymlinkDetails(QString serial)
{
    foreach (UdevRule rule, ruleList) {
        if (rule.getComponent("serial",true).value==serial){
            ui->field_symlink->setText(rule.getComponent("SYMLINK").value);
            ui->field_rule_file_name->setText(rule.getFile());
            UdevRule::Component permisionC = rule.getComponent("MODE");
            if (!permisionC.isEmpty()){
                ui->field_permissions->setText(permisionC.value);
            }
        }
    }
}

void MainWindow::showPortDetails(QString serial,QString port)
{
    showSymlinkDetails(serial);
    showPortDetails(port);
}

void MainWindow::showPortDetails(QString port)
{
    QString out;
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    const QString blankString = QObject::tr("N/A");
    QString description;
    QString manufacturer;
    QString serialNumber;

    for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
        if (serialPortInfo.portName()!=port){
            continue;
        }
        description = serialPortInfo.description();
        manufacturer = serialPortInfo.manufacturer();
        serialNumber = serialPortInfo.serialNumber();
        ui->view_details->append("Port: "+serialPortInfo.portName());
        ui->view_details->append("Location: "+serialPortInfo.systemLocation());
        ui->view_details->append("Description: "+(!description.isEmpty() ? description : blankString));
        ui->view_details->append("Manufacturer: "+(!manufacturer.isEmpty() ? manufacturer : blankString));
        ui->view_details->append("Serial number: "+(!serialNumber.isEmpty() ? serialNumber : blankString));
        ui->view_details->append("Vendor Identifier: "+(serialPortInfo.hasVendorIdentifier() ? QByteArray::number(serialPortInfo.vendorIdentifier(), 16).rightJustified(4, '0') : blankString));
        ui->view_details->append("Product Identifier: "+(serialPortInfo.hasProductIdentifier() ? QByteArray::number(serialPortInfo.productIdentifier(), 16).rightJustified(4, '0') : blankString));
        ui->view_details->append("Busy: "+(serialPortInfo.isBusy() ? QObject::tr("Yes") : QObject::tr("No")));
    }
}

void MainWindow::addPortsToTable()
{
    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &serialPortInfo : serialPortInfos){
        QString serialNumber = serialPortInfo.serialNumber();
        if (serialNumber.isEmpty() && !ui->cb_virtuals->isChecked()){
            continue;
        }
        QString idVendor = QByteArray::number(serialPortInfo.vendorIdentifier(), 16).rightJustified(4, '0');;;
        QString idProduct = QByteArray::number(serialPortInfo.productIdentifier(), 8).rightJustified(4, '0');;
        QString port = serialPortInfo.portName();
        QString time = QFileInfo(serialPortInfo.systemLocation()).lastModified().toString("hh:mm:ss.zzz");
        QTableWidgetItem *cellSerial = new QTableWidgetItem(!serialNumber.isEmpty() ? serialNumber : BLANK_STRING);
        QTableWidgetItem *cellPort = new QTableWidgetItem(port);
        QTableWidgetItem *cellTime = new QTableWidgetItem(time);
        ui->table->insertRow(ui->table->rowCount());
        int row = ui->table->rowCount() -1;
        ui->table->setItem(row, 0, cellSerial);
        ui->table->setItem(row, 1, cellPort);
        ui->table->setItem(row, 2, cellTime);
        this->deviceList.append(Device(serialNumber,idVendor,idProduct));
    }
}

UdevRule MainWindow::parseUdevRule(QString line)
{
    line=line.trimmed();
    UdevRule r;
    if (line.size()==0 || line[0]=='#'){
        return UdevRule();
    }
    QStringList lineComponents = line.split(",");
    // Regex for capturing udev rules components:
    // e.g    key    op value
    //      SUBSYSTEM=="block"
    //      |        key          |op|      value      |
    //      ATTR{loop/backing_file}=="/var/lib/docker/*"
    //                             K E Y       O P            V A L U E
    QRegularExpression re("^([^\\+=]+)([\\+=]+)\\\"([^\\\"]*)\\\"$");
    foreach (QString lineComponent, lineComponents) {
        lineComponent = lineComponent.trimmed();
        QRegularExpressionMatch match = re.match(lineComponent);
        if (match.hasMatch()) {
            UdevRule::Component c;
            c.key = match.captured(1);
            c.op = match.captured(2);
            c.value = match.captured(3);
            r.addComponent(c);
        }
    }
    return r;
}

void MainWindow::addSymlinksToTable(){
    QStringList	 listUdevFiles = QDir(UDEV_RULES_DIR).entryList(QDir::Files | QDir::NoSymLinks);
    foreach (QString fileName, listUdevFiles) {
        QFile file(UDEV_RULES_DIR+fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            qDebug()<<"ERRROR";
            return;
        }

        while (!file.atEnd()){
            QByteArray line = file.readLine();
            UdevRule rule = parseUdevRule(line);
            UdevRule::Component serialComponent = rule.getComponent("serial",true);
            if (!serialComponent.isEmpty() && rule.hasSymlink()){
                if (serialComponent.isEmpty()){
                    continue;
                }
                bool isPort = false;
                foreach (Device d, deviceList) {
                    if (serialComponent.value == d.getSerial()){
                        isPort = true;
                        break;
                    }
                }
                if (!isPort){
                    QTableWidgetItem *cellTime = new QTableWidgetItem(serialComponent.value);
                    QTableWidgetItem *cellDisc = new QTableWidgetItem("DISCONNECTED ->"+rule.getComponent("SYMLINK").value);
                    QTableWidgetItem *cell____ = new QTableWidgetItem("-----------");
                    ui->table->insertRow(ui->table->rowCount());
                    int row = ui->table->rowCount() -1;
                    ui->table->setItem(row, 0, cellTime);
                    ui->table->setItem(row, 1, cellDisc);
                    ui->table->setItem(row, 2, cell____);
                }
                rule.setFile(fileName);
                this->ruleList.append(rule);
            }
        }
    }
}

void MainWindow::on_refresh_clicked()
{
    this->refreshSerialPortList();
}

//void MainWindow::deleteRuleBySerial(QString serial)
//{
//    foreach (UdevRule rule, ruleList) {
//        if (rule.getComponent("serial",true).value==serial){
//            ui->field_symlink->setText(rule.getComponent("SYMLINK").value);
//            ui->field_rule_file_name->setText(rule.getFile());
//            UdevRule::Component permisionC = rule.getComponent("MODE");
//            if (!permisionC.isEmpty()){
//                ui->field_permissions->setText(permisionC.value);
//            }
//        }
//    }
//}

void MainWindow::on_b_delete_clicked()
{
    QModelIndex currentIndex = ui->table->currentIndex();
    QString serial = ui->table->item(currentIndex.row(), 0)->text();
    QFile file(UDEV_RULES_DIR+ui->field_rule_file_name->text());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }
    QByteArray fileText;
    while (!file.atEnd()){
        QByteArray line = file.readLine();
        UdevRule rule = parseUdevRule(line);
        UdevRule::Component serialComponent = rule.getComponent("serial",true);
        if (!serialComponent.isEmpty() && serial==serialComponent.value){
            continue;
        }
        fileText+=line;
    }
    file.remove();
    file.close();

    QFile fileW(UDEV_RULES_DIR+ui->field_rule_file_name->text());
    if (!fileW.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox messageBox;
        messageBox.critical(0,"Error","You do not have permissions to write to the file "+QString(UDEV_RULES_DIR)+ui->field_rule_file_name->text()+". Please run the program using kdesu or other similar commands");
        messageBox.setFixedSize(500,200);
        return;
    }
    fileW.write(fileText);
    fileW.close();
    this->refreshSerialPortList();
    clearDetailsPanel();
}


bool MainWindow::validate_apply(){
    QString s= ui->field_symlink->text();
    QString f= ui->field_rule_file_name->text();
    QString p= ui->field_permissions->text();
    QMessageBox messageBox;
    messageBox.setFixedSize(500,200);
    if (!QRegExp("[\\w\\d-_]*").exactMatch(s)  ){
        messageBox.critical(0,"Error","The symlink may be alphanumeric or contains the character _ or -");
        return false;
    }

    if (!QRegExp("[\\w\\d_-]+\\.rules").exactMatch(f)  ){
        messageBox.critical(0,"Error","The filename may be alphanumeric or contains the character _ or -");
        return false;
    }

    if (s.size()==0 && s.size()>20){
        messageBox.critical(0,"Error","The symlink name size must be between 1 and 20");
        return false;
    }
    if (p.size()>0){
        if (p.size()!=3)     {
            messageBox.critical(0,"Error","The permissions size must be between 4");
            return false;
        }
        if (!QRegExp("\\d{3}").exactMatch(p)){
            messageBox.critical(0,"Error","The permision name size must be e.g '777'");
            return false;
        }
        if (p.toInt()>777){
            messageBox.critical(0,"Error","The permision name size must be betwen 777 or less. E.g '777' or '545'");
            return false;
        }
    }
    return true;
}

void MainWindow::createSymlink(QString symlink,QString permisions,QString filename,Device dev){
    QFile fileW(UDEV_RULES_DIR+filename);
    if (!fileW.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
        QMessageBox messageBox;
        messageBox.critical(0,"Error","You do not have permissions to write to the file "+QString(UDEV_RULES_DIR)+filename+". Please run the program using kdesu or other similar commands");
        messageBox.setFixedSize(500,200);
        return;
    }
    QString newRule = "SUBSYSTEM==\"tty\", ATTRS{idVendor}==\""+dev.getIdVendor()+"\", ATTRS{idProduct}==\""+dev.getIdProduct()+"\", ATTR{serial}=\""+dev.getSerial()+"\", SYMLINK+=\""+symlink+"\"";
    if (permisions.size()>0){
        newRule+=+"\", MODE=\""+permisions;
    }
    fileW.write(newRule.toLatin1());
    fileW.close();
}

void MainWindow::on_b_apply_clicked()
{
    if (validate_apply() == false){
        return;
    }
    QModelIndex currentIndex = ui->table->currentIndex();
    QString serial = ui->table->item(currentIndex.row(), 0)->text();
    QString s= ui->field_symlink->text();
    QString f= ui->field_rule_file_name->text();
    QString p= ui->field_permissions->text();
    Device d;
    foreach (Device dd, deviceList) {
        if (serial == dd.getSerial()){
           d = dd;
        }
    }
    on_b_delete_clicked();
    createSymlink(s,p,f,d);
    on_refresh_clicked();
}

void MainWindow::on_field_rule_file_name_editingFinished()
{
    if (ui->b_generate->hasFocus()){
        return;
    }
    QString text =ui->field_rule_file_name->text();
    if (!text.endsWith(".rules")){
        ui->field_rule_file_name->setText(text+".rules");
    }
    if (!QRegExp("[\\w\\d-_]+\\.rules").exactMatch(ui->field_rule_file_name->text())  ){
        QMessageBox messageBox;
        messageBox.setFixedSize(500,200);
        messageBox.critical(0,"Error","The filename may be alphanumeric or contains the character _");
        ui->field_rule_file_name->setFocus();
    }
}

void MainWindow::on_b_generate_clicked()
{
    QString s =ui->field_symlink->text();
    if (s.size()!=0){
        QString newS;
        foreach(QChar c, s){
            if (!QRegExp("[\\w\\d-_]*").exactMatch(QString(c))){
                newS.push_back('_');
            }
            else{
                newS.push_back(c);
            }
            ui->field_rule_file_name->setText(newS+".rules");
        }
    }else{
        ui->field_rule_file_name->setText("60-portbinder.rules");
    }
}
