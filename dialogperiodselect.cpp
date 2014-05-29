#include "dialogperiodselect.h"
#include "ui_dialogperiodselect.h"

DialogPeriodSelect::DialogPeriodSelect(QWidget *parent) : QDialog(parent), ui(new Ui::DialogPeriodSelect)
{
    ui->setupUi(this);

    connect(ui->btnShow,SIGNAL(clicked()),this,SLOT(onShow()));

    connect(ui->calendarWidgetEnd,SIGNAL(selectionChanged()),this,SLOT(onSelectEndDate()));
    connect(ui->calendarWidgetStart,SIGNAL(selectionChanged()),this,SLOT(onSelectStartDate()));

    connect(ui->rbReportPeriod,SIGNAL(toggled(bool)),this,SLOT(onReportTypeChange(bool)));

    ui->lblStart->setText(tr("От:") + ui->calendarWidgetStart->selectedDate().toString("yyyy-MM-dd"));
    ui->lblEnd->setText(tr("До:") + ui->calendarWidgetEnd->selectedDate().toString("yyyy-MM-dd"));

    startDate = ui->calendarWidgetStart->selectedDate().toString("yyyy-MM-dd")  + " 00:00:00";
    endDate = ui->calendarWidgetEnd->selectedDate().toString("yyyy-MM-dd")  + " 24:00:00";

    ui->calendarWidgetStart->setEnabled(false);
    ui->calendarWidgetEnd->setEnabled(false);
    showAll=true;
}

DialogPeriodSelect::~DialogPeriodSelect()
{
    delete ui;
}

void DialogPeriodSelect::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void DialogPeriodSelect::onShow()
{
    this->accept();
}

void DialogPeriodSelect::onSelectStartDate()
{
    ui->lblStart->setText(tr("От:") + ui->calendarWidgetStart->selectedDate().toString("yyyy-MM-dd"));
    ui->calendarWidgetEnd->setMinimumDate(ui->calendarWidgetStart->selectedDate());;

    startDate = ui->calendarWidgetStart->selectedDate().toString("yyyy-MM-dd") + " 00:00:00";
    endDate = ui->calendarWidgetEnd->selectedDate().toString("yyyy-MM-dd") + " 24:00:00";

}

void DialogPeriodSelect::onSelectEndDate()
{
    ui->lblEnd->setText(tr("До:") + ui->calendarWidgetEnd->selectedDate().toString("yyyy-MM-dd"));
    ui->calendarWidgetStart->setMaximumDate(ui->calendarWidgetEnd->selectedDate());

    startDate = ui->calendarWidgetStart->selectedDate().toString("yyyy-MM-dd") + " 00:00:00";
    endDate = ui->calendarWidgetEnd->selectedDate().toString("yyyy-MM-dd")  + " 24:00:00";
}

void DialogPeriodSelect::onReportTypeChange(bool rt)
{
    ui->calendarWidgetEnd->setEnabled(rt);
    ui->calendarWidgetStart->setEnabled(rt);
    showAll = !rt;
}


