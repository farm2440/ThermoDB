#ifndef DIALOGTIMESTAMP_H
#define DIALOGTIMESTAMP_H

#include <QDialog>

namespace Ui {
class DialogTimestamp;
}

class DialogTimestamp : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTimestamp(QWidget *parent = 0);
    ~DialogTimestamp();

    QString getTimestamp();
private:
    Ui::DialogTimestamp *ui;
};

#endif // DIALOGTIMESTAMP_H
