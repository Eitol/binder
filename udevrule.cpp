#include "udevrule.h"

UdevRule::UdevRule()
{

}

void UdevRule::addComponent(Component c)
{
    addComponent(c.key,c.op,c.value);
}

void UdevRule::addComponent(QString key, QString op, QString value)
{
    Component component;
    component.key = key;
    component.op = op;
    component.value = value;
    components.append(component);
}

bool UdevRule::isEmpty() const
{
    return !components.size();
}

bool UdevRule::hasSymlink() const
{
    foreach (Component c, components) {
        if (c.key.toUpper()=="SYMLINK"){
            return true;
        }
    }
    return false;
}

QString UdevRule::toString() const
{
    QString r = "";
    foreach (Component c, components) {
        r+=c.key+c.op+"\""+c.value+"\"";
        if (components.indexOf(c)<components.size()-1){
            r+=",";
        }
    }
    return r;
}

QString UdevRule::getFile() const
{
    return file;
}

void UdevRule::setFile(const QString &value)
{
    file = value;
}

UdevRule::Component UdevRule::getComponent(QString key,bool isAttr) const
{
    if (isAttr){
        foreach (Component c, components) {
            if ("ATTRS{"+key+"}" == c.key || "ATTR{"+key+"}" == c.key){
                return c;
            }
        }
    }else{
        foreach (Component c, components) {
            if (c.key == key){
                return c;
            }
        }
    }
    return Component();
}


