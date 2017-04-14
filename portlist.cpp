#include "portlist.h"
#include "ui_portlist.h"

PortList::PortList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PortList)
{
    ui->setupUi(this);
}

PortList::~PortList()
{
    delete ui;
}
