#include "gui.h"
#include "ui_gui.h"

GUI::GUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GUI)
{
    ui->setupUi(this);
    this->rc522 = new QtRC522(this);
}

GUI::~GUI()
{
    delete ui;
}
