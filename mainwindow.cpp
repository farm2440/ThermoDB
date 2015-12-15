#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :    QMainWindow(parent),    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //главно меню
    QMenu *menuDB = new QMenu(tr("База данни..."),this);
    ui->menuBar->addMenu(menuDB);
    actionLoadDB = menuDB->addAction(tr("Зареждане от файл"));
    connect (actionLoadDB, SIGNAL(triggered()), this, SLOT(onLoadDBAction()));
    actionCommitDB = menuDB->addAction(tr("Запази промените"));
    connect (actionCommitDB, SIGNAL(triggered()), this, SLOT(onCommitDBAction()));
    actionCommitDB->setEnabled(false);
    menuDB->addSeparator();
    QAction *quitAction = menuDB->addAction(tr("Изход"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    changesSaved = true;

    mapper = new QSignalMapper(this);
    mapper->setMapping(ui->btnMinus,1);
    mapper->setMapping(ui->btnPlus,2);
    connect(ui->btnMinus, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(ui->btnPlus, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(onPlusMinus(int)));

    showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{//override , потребителя се опитва да излезе от програмата
    if(changesSaved) event->accept();
    else
    {
        if(QMessageBox::Yes == QMessageBox::warning(this,tr("ВАЖНО!"),
                                                    tr("Промените не са записани.\nСигурни ли сте че искате да излезете от програмата?"),
                                                    QMessageBox::Yes,QMessageBox::No))
            event->accept();
        else
            event->ignore();
    }
}

void MainWindow::onLoadDBAction()
{
    QString qryStr;
    bool ok;
    int col;

    QString fn = QFileDialog::getOpenFileName(this,tr("Open DB file"), "C:\\", tr("database (*.db)"));
    if(fn!="") fileName=fn;
    else return;

    this->setWindowTitle("ThermoDB: " + fileName);
    //отваряне на базата данни
    QFile dbFile(fileName);
    if(!dbFile.exists())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Файлът с БД не съществува"));
        return;
    }
    else
    {   database = QSqlDatabase::addDatabase("QSQLITE"); //QSQLITE е за версия 3 и нагоре, QSQLITE2 e за версия 2
        database.setDatabaseName(fileName);
        if(!database.open())
        {
            QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да отворя БД"));
            return;
        }
    }
    //Зареждане на текущите настройки от БД:
    QSqlQuery qry; //ВАЖНО!!!! QSqlQuery трябва да се създаде СЛЕД КАТО БД е отворена. Иначе няма да работи !!!

    qryStr = "SELECT * FROM tableChannels;";
    ok = qry.prepare(qryStr);
    if(!qry.exec())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SELECT заявка!"));
        return;
    }

    Channel chan;
    channelsList.clear();
    hashId2Col.clear();
    hashCol2Id.clear();
    hashId2Offset.clear();
    hashId2Ratio.clear();

    col = 1; //В колона 0 е дата/час.
    while(qry.next())
    {   //ID в базата данни
        chan.setID( qry.value(0).toInt(&ok));
        //Име на канал
        chan.setName(qry.value(1).toString());
        //Адрес в RS-485
        int node = qry.value(2).toInt(&ok);
        chan.setNode( node);
        // 1-wire адрес
        chan.setAddress(qry.value(3).toString());
        //Офсет
        chan.setOffset(qry.value(5).toDouble());
        //Коефициент
        chan.setRatio(qry.value(6).toDouble());

        //Записваат се данни само за канали с коректен адрес и номер на контролер
        if((chan.node()<0) || (chan.node()>15)) continue;
        if(chan.address().length() !=16 ) continue;

        channelsList.append(chan);
        hashId2Col.insert(chan.id(), col);//Запазвам кой id канал в коя колона ще отиде
        hashCol2Id.insert(col,chan.id());
        hashId2Offset.insert(chan.id(), chan.offset());
        hashId2Ratio.insert(chan.id(), chan.ratio());
        col++;
    }
    //Оразмеряване на колоните в таблицата
    disconnect(ui->tableWidget,SIGNAL(cellChanged(int,int)),this, SLOT(onCellChanged(int,int)));

    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(channelsList.count()+1);
    //Заглавия на колоните
    QStringList headers;
    headers.append(tr("Дата\nЧас"));
    foreach(Channel ch, channelsList) headers.append(ch.name());
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    //Избор дали да се заредят данни за период или да се зареди всичко
    DialogPeriodSelect dlg(this);
    dlg.exec();
    //Зареждане на събраните данни
    if(dlg.showAll) qryStr = "SELECT * FROM tableLog ORDER BY Timestamp DESC;";
    else qryStr = QString("SELECT * FROM tableLog WHERE Timestamp>='%1' AND Timestamp<='%2';")
                                 .arg(dlg.startDate)
                                 .arg(dlg.endDate);

    qry.setForwardOnly(true);//за по-бързо
    ok = qry.prepare(qryStr);
    if(!qry.exec())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да изпълня SELECT заявка!"));
        return;
    }

    int row=0,chId;
    QString pTimestamp = "", timestamp;
    int nTmp;
    double dTemp;
    QTableWidgetItem *itm;

    while(qry.next())
    {
        timestamp = qry.value(0).toString();
        if(pTimestamp != timestamp)
        {//Ако Това е нова стойност за дата час. Добавя се нов ред а датата/час се слага в първата колона.
            pTimestamp=timestamp;
            row=ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);
            itm = new QTableWidgetItem( timestamp );
            itm->setFlags(0);
            ui->tableWidget->setItem(row,0,itm);
        }
        //извличаме температурата и я слагаме в колоната за съответния канал
        chId = qry.value(1).toInt(&ok);
        nTmp = qry.value(2).toInt(&ok);
        dTemp = (double)nTmp/100;
        dTemp *= hashId2Ratio[chId];
        dTemp += hashId2Offset[chId];
        itm = new QTableWidgetItem(QString::number(dTemp,'f',1)); //температура
        itm->setTextAlignment(Qt::AlignCenter);
        //по ID на канала се разбира в коя колона да се сложи температурата
        col = hashId2Col[chId];
        ui->tableWidget->setItem(row,col,itm);
    }
    database.close();
    changesSaved=true;
    actionCommitDB->setEnabled(false);
    connect(ui->tableWidget,SIGNAL(cellChanged(int,int)),this, SLOT(onCellChanged(int,int)));

}

void MainWindow::onCommitDBAction()
{
    double dTemp;
    int nTemp;
    bool ok;
    int row, col, chanId;
    QString timestamp, str;
    bool incorrectValuesFound = false;
    bool dbWriteFail = false;
    double offset, ratio;

    //отваряне на базата данни
    QFile dbFile(fileName);
    if(!dbFile.exists())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Файлът с БД не съществува"));
        return;
    }
    else
    {   database = QSqlDatabase::addDatabase("QSQLITE"); //QSQLITE е за версия 3 и нагоре, QSQLITE2 e за версия 2
        database.setDatabaseName(fileName);
        if(!database.open())
        {
            QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да отворя БД"));
            return;
        }
    }

    QProgressBar *pbar = new QProgressBar(this);
    pbar->setValue(0);
    pbar->setMaximum(ui->tableWidget->rowCount()+1);
    ui->statusBar->addWidget(pbar);
    QSqlQuery qry;
    QString sql;
    for(row=0 ; row!=ui->tableWidget->rowCount() ; row++)
    {
        pbar->setValue(row);
        timestamp = ui->tableWidget->item(row,0)->text();
        for(col=1 ; col!=ui->tableWidget->columnCount() ; col++)
        {
            if(ui->tableWidget->item(row,col) == NULL) continue;
            if(ui->tableWidget->item(row,col)->toolTip()!="1") continue;
            str = ui->tableWidget->item(row,col)->text();
            dTemp = str.toDouble(&ok);
            if(!ok)
            {
                ui->tableWidget->item(row,col)->setBackgroundColor(Qt::red);
                incorrectValuesFound = true;
                continue;
            }
            else   ui->tableWidget->item(row,col)->setBackgroundColor(Qt::white);

            chanId = hashCol2Id[col];
            offset = hashId2Offset[chanId];
            ratio = hashId2Ratio[chanId];
            dTemp -= offset;
            dTemp /= ratio;
            dTemp *= 100;
            nTemp = (int) dTemp;


            //Проверява се дали има запис с този Тimestamp и ChannelID. Ако има, то се ползва UPDATE
            //Иначе се ползва INSERT
            sql = QString("SELECT * FROM tableLog WHERE Timestamp=%1 AND ChannelID=%2;").arg(timestamp).arg(chanId);
            qry.prepare(sql);

            if(qry.next()) sql = QString("UPDATE tableLog SET Temp=%1 WHERE ChannelID=%2 AND Timestamp='%3';").arg(nTemp).arg(chanId).arg(timestamp);
            else sql = QString("INSERT INTO tableLog VALUES ('%1',%2,%3);").arg(timestamp).arg(chanId).arg(nTemp);

            qry.prepare(sql);
            if (!qry.exec())
            {
                dbWriteFail=true;
                ui->tableWidget->item(row,0)->setBackgroundColor(Qt::red);
            }
            else
            {
                ui->tableWidget->item(row,col)->setToolTip("");
                ui->tableWidget->item(row,0)->setBackgroundColor(Qt::white);
            }

        }
    }

    database.close();
    delete pbar;
    if(incorrectValuesFound) QMessageBox::warning(this,tr("ГРЕШКА!"), tr("Таблицата съдържа некоректна стойност!"));
    if(dbWriteFail) QMessageBox::warning(this,tr("ГРЕШКА!"), tr("Грешка при запис в БД!"));
    actionCommitDB->setEnabled(false);
    changesSaved=true;
}

void MainWindow::onPlusMinus(int op)
{
    QList<QTableWidgetItem*> selection = ui->tableWidget->selectedItems();
    double dTemp;

    bool ok;
    bool incorrectValuesFound = false;

    double v=ui->doubleSpinBox_val->value();
    if(v == 0.0) return;

    int criteria = 0;
    if(ui->rbGreatherThan->isChecked()) criteria=1;
    if(ui->rbLessThan->isChecked()) criteria=2;

    double threshold = ui->doubleSpinBox_threshold->value();

    foreach(QTableWidgetItem* itm , selection)
    {
        if(itm->column()==0) continue; //в първа колона е дата/час

        dTemp = itm->text().toDouble(&ok);
        if(!ok)
        {
            itm->setBackgroundColor(Qt::red);
            incorrectValuesFound = true;
            continue;
        }
        else  itm->setBackgroundColor(Qt::white);

        switch(criteria)
        {
        case 0:
            break;
        case 1://greather than only
            if(dTemp<threshold) continue;
            break;
        case 2://less than only
            if(dTemp>threshold) continue;
            break;
        }

        changesSaved = false;
        if(op==2) dTemp+=v;
        else dTemp-=v;

        itm->setText(QString::number(dTemp,'f',1));
    }//foreach

    if(!changesSaved) actionCommitDB->setEnabled(true);
    if(incorrectValuesFound) QMessageBox::warning(this,tr("ГРЕШКА!"), tr("Таблицата съдържа некоректна стойност!"));
}

void MainWindow::onCellChanged(int row, int col)
{
    ui->tableWidget->item(row,col)->setToolTip("1"); //променените се маркират. само те ще бъдат записани
    actionCommitDB->setEnabled(true);
    changesSaved = false;
}

void MainWindow::on_btnInsertRecord_clicked()
{
    QFile dbFile(fileName);
    if(!dbFile.exists())
    {
        QMessageBox::critical(this, tr("Грешка база данни"), tr("Файлът с БД не съществува\nили няма заредена БД"));
        return;
    }

    QTableWidgetItem *itm;
    DialogTimestamp dlg;
    if(dlg.exec() == DialogTimestamp::Accepted)
    {
        qDebug() << "New record timestamp is " << dlg.getTimestamp();
        int row = ui->tableWidget->currentRow() + 1;
        ui->tableWidget->insertRow(row);
        itm = new QTableWidgetItem(dlg.getTimestamp());
        itm->setFlags(0);
        ui->tableWidget->setItem(row,0,itm);

        //Вмъкване на температури:
        for(int i=1 ; i!=ui->tableWidget->columnCount() ; i++)
        {
            itm = new QTableWidgetItem("0");
            itm->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(row,i,itm);
        }
    }
}

void MainWindow::on_btnDeleteRow_clicked()
{
       int row = ui->tableWidget->currentRow();
       qDebug() << "Delete record on row " << row;
       if(row==-1)
       {
           QMessageBox::critical(this, "ГРЕШКА", "Няма избран ред!");
           return;
       }

       QFile dbFile(fileName);
       if(!dbFile.exists())
       {
           QMessageBox::critical(this, tr("Грешка база данни"), tr("Файлът с БД не съществува\nили няма заредена БД"));
           return;
       }else
       {   database = QSqlDatabase::addDatabase("QSQLITE"); //QSQLITE е за версия 3 и нагоре, QSQLITE2 e за версия 2
           database.setDatabaseName(fileName);
           if(!database.open())
           {
               QMessageBox::critical(this, tr("Грешка база данни"), tr("Не мога да отворя БД"));
               return;
           }
       }

       QString timestamp = ui->tableWidget->item(row,0)->text();
       QString warning = QString("Записът от ред %1 за %2 ще бъде изтрит от БД!").arg(row+1).arg(timestamp);
       warning += "\nПромяната не може да бъде отменена. Сигурни ли сте?";
       int res = QMessageBox::warning(this,"ПРЕДУПРЕЖДЕНИЕ", warning, QMessageBox::Ok, QMessageBox::Cancel);

       QSqlQuery qry;
       if(res==QMessageBox::Ok)
       {
           QString sql = QString("DELETE FROM tableLog WHERE Timestamp='%1';").arg(timestamp);
           qry.prepare(sql);
           if(qry.exec()) ui->tableWidget->removeRow(row);
           else QMessageBox::critical(this, tr("Грешка база данни"), tr("SQL DELETE заявката не може да бъде изпълнена!"));
       }

       database.close();
}
