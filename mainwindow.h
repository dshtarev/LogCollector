#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariantMap>
#include <QCheckBox>
#include <QFont>
#include <QLabel>

class QUdpSocket;

namespace Ui {
class MainWindow;
}

enum MsgType
{
    ETextMsg,
    EProcessInfoMsg
};

enum ChannelType
{
    EStdio,
    EStderr
};

struct ProcessInfo
{
    double  cpuUsage;
    int     mem;
    uint8_t process_state;

};


struct Msg
{
    uint8_t type;
    uint8_t channelType;
    char    processName[32];
    char    prefix[16];
    char    payload[];
};


class ProcessItemWidget: public QWidget
{
    Q_OBJECT

    QCheckBox * cbProcessName;
    QLabel * lbInfo;

public:
    ProcessItemWidget(QWidget * parent = nullptr, const QString & processName = QString() );
    ~ProcessItemWidget();

    void setInfo( const ProcessInfo & );
};


struct ProcessWidgetDescriptor
{
    int lifeTime;
    ProcessItemWidget * widget;

    ProcessWidgetDescriptor()
    {
        lifeTime = 0;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    QUdpSocket * m_udpSocket;
    int m_maxLogItemsCount;
    int m_logFontSize;
    QFont m_logItemsFont;
    QStringList m_filtersList;
    QMap<QString, ProcessWidgetDescriptor> m_processMap;


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
public slots:
    void readPendingDatagrams();
    void itemsCountChanged(int);
    void fontSizeChanged(int);
    void clearLog();
    void searchClicked();
    void filterAddClicked();
    void filterDelClicked();

    void showProcessTriggered();
    void showFiltersTriggered();
    void exitTriggered();

protected:
    QString createStringMessage( const QByteArray & );
    void addMessage( const QString & );
    void createFilterList();
    bool checkFilterMatch( const QString & );

    void updateProcessList();

    void getProcessInfo( int pid );
    void createDashboard();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
