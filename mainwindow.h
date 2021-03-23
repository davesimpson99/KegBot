#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateClock();

private slots:
    void on_WriteDataBtn_clicked();

    void on_SimPourBtn_clicked();

    void on_SimPourBtn_pressed();

    void on_ResetBtn_clicked();

    void on_PintsPouredspinBox_valueChanged(int arg1);

    void on_ExitButton_clicked();

    void on_CB_KegSelect_currentIndexChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    int timerId;
    QTimer *clocktimer;

    void update_Screen_Counters();

protected:
    void timerEvent(QTimerEvent *event);


};

#endif // MAINWINDOW_H
