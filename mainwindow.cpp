#include <QUdpSocket>
#include <QDebug>
#include <QApplication>
#include <QRegularExpression>
#include "mainwindow.h"
#include "ui_mainwindow.h"

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


}


MainWindow::~MainWindow()
{
    delete ui;
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
            item->setBackground( QBrush( Qt::yellow,Qt::HorPattern ) );

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
        QString msg = QString::fromUtf8(datagram);

        if( ui->pbCapture->isChecked() )
            if( checkFilterMatch(msg) )
                addMessage( msg );
    }
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

