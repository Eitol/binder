#include "device.h"

Device::Device()
{

}

Device::Device(QString serial, QString idVendor, QString idProduct, QString symlink)
{
    this->idProduct=idProduct;
    this->idVendor=idVendor;
    this->serial=serial;
    this->symlink=symlink;

}

QString Device::getSymlink() const
{
    return symlink;
}

void Device::setSymlink(const QString &value)
{
    symlink = value;
}

bool Device::isVoid()
{
    return this->getIdVendor().size()==0;
}

QString Device::getIdProduct() const
{
    return idProduct;
}

void Device::setIdProduct(const QString &value)
{
    idProduct = value;
}

QString Device::getIdVendor() const
{
    return idVendor;
}

void Device::setIdVendor(const QString &value)
{
    idVendor = value;
}


QString Device::getSerial() const
{
    return serial;
}

void Device::setSerial(const QString &value)
{
    serial = value;
}
