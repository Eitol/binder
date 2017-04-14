#ifndef DEVICE_H
#define DEVICE_H

#include <QString>



class Device
{
public:
    Device();
    Device(QString serial, QString idVendor, QString idProduct, QString symlink="");
    QString getSerial() const;
    void setSerial(const QString &value);
    QString getIdVendor() const;
    void setIdVendor(const QString &value);
    QString getIdProduct() const;
    void setIdProduct(const QString &value);
    QString getSymlink() const;
    void setSymlink(const QString &value);
    bool isVoid();

private:
    QString serial;
    QString idVendor;
    QString idProduct;
    QString symlink;
};

#endif // DEVICE_H
