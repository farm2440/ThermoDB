#include "dialogtimestamp.h"
#include "ui_dialogtimestamp.h"

DialogTimestamp::DialogTimestamp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTimestamp)
{
    ui->setupUi(this);
}

DialogTimestamp::~DialogTimestamp()
{
    delete ui;
}

QString DialogTimestamp::getTimestamp()
{
    return ui->lineEdit->text();
}
