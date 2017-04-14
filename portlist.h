#ifndef PORTLIST_H
#define PORTLIST_H

#include <QDialog>

namespace Ui {
class PortList;
}

class PortList : public QDialog
{
    Q_OBJECT

public:
    explicit PortList(QWidget *parent = 0);
    ~PortList();

private:
    Ui::PortList *ui;
};

#endif // PORTLIST_H
