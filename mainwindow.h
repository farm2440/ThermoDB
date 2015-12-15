#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QTextCodec>
#include <QCloseEvent>
#include <QList>
#include <QHash>
#include <QStringList>
#include <QSignalMapper>
#include <QProgressBar>
#include <QDebug>

#include "channel.h"
#include "dialogperiodselect.h"
#include "dialogtimestamp.h"

namespace Ui {
    class MainWindow;
}

//C:\qt\tools\mingw48_32\lib\gcc\i686-w64-mingw32\4.8.0\include\c++\streambuf ???
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QAction *actionLoadDB, *actionCommitDB, *actionQuit;

private:
    Ui::MainWindow *ui;

    QSqlDatabase database;
    bool changesSaved;
    QHash<int,int> hashId2Col, hashCol2Id;
    QHash<int, double> hashId2Offset, hashId2Ratio;
    QList<Channel> channelsList;

    QSignalMapper *mapper;
    QString fileName;

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void onLoadDBAction();
    void onCommitDBAction();

    void onPlusMinus(int op);
    void onCellChanged(int row, int col);
    void on_btnInsertRecord_clicked();
    void on_btnDeleteRow_clicked();
};

#endif // MAINWINDOW_H
