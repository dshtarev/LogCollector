#include <QUdpSocket>
#include <QDebug>
#include <QApplication>
#include <QtQuick/QQuickItem>
#include <QtQuickWidgets/QQuickWidget>
#include <QtQuick/QQuickView>
#include <QRegularExpression>
#include <QCheckBox>
#include <QScrollArea>
#include <QProcess>
#include "mainwindow.h"
#include "ui_mainwindow.h"

ProcessItemWidget::ProcessItemWidget(QWidget * parent, const QString & processName):
    QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(3, 3, 3, 3);
    QFrame * frm = new QFrame(this);
    layout->addWidget( frm );
    setLayout(layout);

    QVBoxLayout *frmlayout = new QVBoxLayout(frm);

    cbProcessName = new QCheckBox(processName);
    lbInfo = new QLabel("CPU:   MEM: ");

    frmlayout->addWidget(cbProcessName);
    frmlayout->addWidget(lbInfo);
    frmlayout->setContentsMargins(0, 0, 0, 0);

}

ProcessItemWidget::~ProcessItemWidget()
{

}


void ProcessItemWidget::setInfo(const ProcessInfo & info)
{
    QString infoStr = "CPU:" + QString::number(info.cpuUsage,'f', 2 ) + " MEM:"+QString::number(info.mem/1000.0,'f',2 )+"MB";
    lbInfo->setText( infoStr );

    if( info.process_state == QProcess::Running )
    {
        setStyleSheet("background-color: lightgreen;");
    }

    if( info.process_state == QProcess::NotRunning )
    {
        setStyleSheet("background-color: red;");
    }

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_maxLogItemsCount = 100;
    m_logFontSize = 18;
    m_logItemsFont = QFont("arial", m_logFontSize );

    ui->sbItemsCount->setValue( m_maxLogItemsCount );
    ui->sbFontSize->setValue( m_logFontSize );

    connect( ui->sbItemsCount, SIGNAL(valueChanged(int)), this, SLOT(itemsCountChanged(int)) );
    connect( ui->sbFontSize, SIGNAL(valueChanged(int)), this, SLOT(fontSizeChanged(int)) );
    connect( ui->pbClear, SIGNAL(clicked())  , this, SLOT(clearLog()) );
    connect( ui->pbSearch, SIGNAL(clicked()), this, SLOT(searchClicked()) );

    connect( ui->pbFilterAdd, SIGNAL(clicked()), this, SLOT(filterAddClicked()) );
    connect( ui->pbFilterDel, SIGNAL(clicked()), this, SLOT(filterDelClicked()) );

    ui->actionShowFilter->setChecked(true);
    ui->actionShowProcesses->setChecked(true);
    connect( ui->actionShowProcesses, SIGNAL(triggered()), this, SLOT(showProcessTriggered()) );
    connect( ui->actionShowFilter, SIGNAL(triggered()), this, SLOT(showFiltersTriggered()) );
    connect( ui->actionExit, SIGNAL(triggered()), this, SLOT(exitTriggered()) );


    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(54321);
    connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

    updateProcessList();

    createDashboard();

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createDashboard()
{
    ui->quickWidget->setSource(QUrl(QStringLiteral("../dashboard_1.qml")));

    // Получаем rootObject (QML корень)
    QQuickItem *rootObject = ui->quickWidget->rootObject();

    // Подготовим QVariantMap
    QVariantMap map;
    map["name"] = "TestName";
    map["value"] = 42;

    // Вызовем QML функцию processData(map)
    if (rootObject) {
        QVariant returnedValue;
        QMetaObject::invokeMethod(
            rootObject,
            "processData",
            Q_RETURN_ARG(QVariant, returnedValue),
            Q_ARG(QVariant, map)
        );
    }


}

void MainWindow::showProcessTriggered()
{
    ui->frmProcesses->setVisible( ui->actionShowProcesses->isChecked() );
}


void MainWindow::showFiltersTriggered()
{
    ui->frmFilters->setVisible( ui->actionShowFilter->isChecked() );
}

void MainWindow::exitTriggered()
{
    qApp->exit(0);
}

void MainWindow::filterAddClicked()
{
    ui->lwFilters->addItem( ui->leFilterText->text() );
    createFilterList();
}


void MainWindow::filterDelClicked()
{
    QListWidgetItem * item = ui->lwFilters->takeItem( ui->lwFilters->currentRow() );
    if( item )
        delete item;

    createFilterList();
}

void MainWindow::createFilterList()
{
    m_filtersList.clear();
    for( int i=0;i< ui->lwFilters->count() ; i++ )
    {
        m_filtersList.append( ui->lwFilters->item(i)->text() );
    }
}

bool MainWindow::checkFilterMatch(const QString & str)
{
    QRegularExpression rx;

    bool matched = false;

    if( m_filtersList.count() == 0 )
        matched = true;

    foreach( QString filter, m_filtersList )
    {
        rx.setPattern( filter );

        if( rx.match(str).hasMatch() )
        {
            matched = true;
            break;
        }
    }

    return matched;
}

void MainWindow::searchClicked()
{
    int cI = ui->lwLog->currentRow();
    bool nextSelected = false;

    QRegularExpression rx;
    rx.setPattern( ui->leSearch->text() );

    for( int i=0; i< ui->lwLog->count();i++ )
    {
        QListWidgetItem * item = ui->lwLog->item( i );

        if( rx.match( item->text() ).hasMatch() )
        {
            item->setBackground( QBrush( Qt::yellow,Qt::SolidPattern ) );

            if(!nextSelected && i > cI )
            {
                nextSelected = true;
                ui->lwLog->setCurrentItem( item );
            }

        }
        else
            item->setBackground( Qt::white );
    }

}

void MainWindow::itemsCountChanged(int val)
{
    m_maxLogItemsCount = val;
}

void MainWindow::fontSizeChanged(int val)
{
    m_logFontSize = val;
    m_logItemsFont = QFont( "arial", val );


    for( int i=0;i<ui->lwLog->count();i++ )
    {
        ui->lwLog->item(i)->setFont( m_logItemsFont );
    }
}

void MainWindow::clearLog()
{
    ui->lwLog->clear();
}

void MainWindow::readPendingDatagrams() {
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(m_udpSocket->pendingDatagramSize()));
        m_udpSocket->readDatagram(datagram.data(), datagram.size());

        Msg * msg = (Msg *)datagram.data();

        if( msg->type == ETextMsg )
        {
            QString msg = createStringMessage( datagram );

            if( ui->pbCapture->isChecked() )
                if( checkFilterMatch(msg) )
                    addMessage( msg );
        }

        if( msg->type == EProcessInfoMsg )
        {
            ProcessInfo * processInfo = (ProcessInfo *)msg->payload;

            qDebug() << "CPU" << QString().setNum( processInfo->cpuUsage );
            QQuickItem *rootObject = ui->quickWidget->rootObject();


            // Вызовем QML функцию processData(map)
            if (rootObject) {
                QVariant returnedValue;
                QMetaObject::invokeMethod(
                    rootObject,
                    "appendValue",
                    Q_RETURN_ARG(QVariant, returnedValue),
                    Q_ARG(QVariant,  processInfo->cpuUsage )
                );
            }

            QString key = QString(msg->prefix) + QString( msg->processName );
            if( m_processMap.contains( key ) == false )
            {
                ProcessWidgetDescriptor wd;
                wd.widget = NULL;
                m_processMap[key] = wd;
                updateProcessList();
            }

            ProcessItemWidget * wgt = m_processMap[key].widget;

            if( wgt )
                wgt->setInfo( *processInfo );
            m_processMap[key].lifeTime++;

        }

    }
}

QString MainWindow::createStringMessage(const QByteArray & ba )
{
    QString str;
    Msg * msg = (Msg *)ba.data();

    str += "[" + QString(msg->prefix) + "*" + QString(msg->processName ) + "]";
    QString channelName = "stdio";
    if( msg->channelType == EStderr )
        channelName = "stderr";
    str += "[" + channelName + "]";

    str += QString( msg->payload );

    return str;
}

void MainWindow::addMessage(const QString & msg)
{



    QListWidgetItem * item = new QListWidgetItem( msg, ui->lwLog );
    item->setFont( m_logItemsFont );
    ui->lwLog->addItem( item );

    if( ui->lwLog->count() > m_maxLogItemsCount )
    {
        for(int i=0;i<(ui->lwLog->count()-m_maxLogItemsCount);i++ )
        {
            delete ui->lwLog->takeItem(i);
        }
    }

    //ui->lwLog->scrollToBottom();

}


void MainWindow::updateProcessList()
{
    ui->lwProcesses->clear();

    for (auto it = m_processMap.begin(); it != m_processMap.end(); ++it) {
        const QString &key = it.key();
        ProcessWidgetDescriptor value = it.value();

        QListWidgetItem *item = new QListWidgetItem(ui->lwProcesses);
        ProcessItemWidget * widget = new ProcessItemWidget(nullptr, key );
        widget->show();

        it.value().widget = widget;
        // привязываем виджет к item
        ui->lwProcesses->addItem(item);
        ui->lwProcesses->setItemWidget(item, widget);

        item->setSizeHint(widget->sizeHint());
    }
}



