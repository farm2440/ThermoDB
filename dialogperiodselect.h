#ifndef DIALOGPERIODSELECT_H
#define DIALOGPERIODSELECT_H

#include <QDialog>
#include <QMessageBox>


namespace Ui {
    class DialogPeriodSelect;
}

class DialogPeriodSelect : public QDialog {
    Q_OBJECT
public:
    DialogPeriodSelect(QWidget *parent = 0);
    ~DialogPeriodSelect();

    bool showAll;
    QString startDate,endDate;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DialogPeriodSelect *ui;

private slots:
    void onShow();

    void onSelectStartDate();
    void onSelectEndDate();

    void onReportTypeChange(bool rt);

};

#endif // DIALOGPERIODSELECT_H
