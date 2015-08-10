#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include "qtrc522.h"

namespace Ui {
class GUI;
}

class GUI : public QMainWindow
{
    Q_OBJECT
private:
    QtRC522 * rc522;
public:
    explicit GUI(QWidget *parent = 0);
    ~GUI();
    
private:
    Ui::GUI *ui;
};

#endif // GUI_H
