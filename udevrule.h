#ifndef UDEVRULE_H
#define UDEVRULE_H

#include <QList>



class UdevRule
{
public:
    // e.g 'DEVICE="/dev/ttyUSB0"
    class Component{
    public:
        QString key;    // 'DEVICE'
        QString op;     // '='
        QString value;  // '/dev/ttyUSB0'
        bool operator==(const Component& other) {
          return key == other.key && op == other.op && value==other.value;
        }
        operator QString() const{
          return QString(key+op+"\""+value+"\"");
        }
        bool isEmpty() const{
            return key=="";
        }
    };

    UdevRule();
    void addComponent(Component c);
    void addComponent(QString key, QString op, QString value);
    Component getComponent(QString key,bool isAttr=false) const;
    bool isEmpty() const;
    bool hasSymlink() const;
    QString toString() const;
    operator QString() const {return toString();}


    QString getFile() const;
    void setFile(const QString &value);

private:
    QList<Component> components;
    QString file;
};

#endif // UDEVRULE_H
