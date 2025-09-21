#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFont>

class QUdpSocket;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    QUdpSocket * m_udpSocket;
    int m_maxLogItemsCount;
    int m_logFontSize;

    QFont m_logItemsFont;

    QStringList m_filtersList;

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
    void addMessage( const QString & );
    void createFilterList();
    bool checkFilterMatch( const QString & );

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
