#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wiringPi.h"
#include <QTimer>
#include <QDebug>


//  TickCounter:
//	Global variable to count interrupts
//	Should be declared volatile to make sure the compiler doesn't cache it.
static volatile int TickCounter;


/* Kegbot Interrupt */
void FlowInterrupt (void)
{
    ++TickCounter;
    /* qDebug() << "Got One..."; */

}
/* End Kegbot Interrupt */

const int full_keg = 103;
const int sim_ticks_per_oz = 4;
const int ticks_per_oz = 166;
const int oz_per_pint = 20;

int pints_poured = 0;
int pints_left = full_keg;
int ozs_poured = 0;
int ozs_left = 0;


void Write_Data()
{
//OPEN DATA FILE IN OUR APPLICAITONS DIRECTORY OR CREATE IT IF IT DOESN'T EXIST
    FILE *file1;
    unsigned char file_data[10];
    const char *filename1 = "kegbot.dat";

    file1 = fopen(filename1, "wb");
    if (file1)
    {
        //----- FILE EXISTS -----
        file_data[0] = pints_poured;
        file_data[1] = ozs_poured;
        file_data[2] = pints_left;
        file_data[3] = ozs_left;

        fwrite(&file_data[0], sizeof(unsigned char), 10, file1) ;

        fclose(file1);
        file1 = NULL;
    }
    else
    {
        //----- FILE NOT FOUND -----
        qDebug() << "file not found";

        //Write new file
        file1 = fopen(filename1, "wb");
        if (file1)
        {
            qDebug() << "Writing new file";
            file_data[0] = pints_poured;
            file_data[1] = ozs_poured;
            file_data[2] = pints_left;
            file_data[3] = ozs_left;

            fwrite(&file_data[0], sizeof(unsigned char), 10, file1) ;

            fclose(file1);
            file1 = NULL;
        }
    }
}


void Read_Data()
{
    //OPEN DATA FILE IN OUR APPLICAITONS DIRECTORY
    FILE *file1;
    unsigned char file_data[10];
    const char *filename1 = "kegbot.dat";

    file1 = fopen(filename1, "rb");
    if (file1)
    {
        //----- FILE EXISTS -----
        fread(&file_data[0], sizeof(unsigned char), 10, file1);

        //qDebug() << "File opened, some byte values: %i %i %i %i\n", file_data[0], file_data[1], file_data[2], file_data[3]);
        qDebug() << file_data[0];
        pints_poured = file_data[0];
        ozs_poured = file_data[1];
        pints_left = file_data[2];
        ozs_left = file_data[3];

        fclose(file1);
        file1 = NULL;
        }
        else
        {
            //----- FILE NOT FOUND -----
            qDebug() << "file not found";
        }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setenv("WIRINGPI_GPIOMEM", "1", 1);
    system("gpio mode 0 out");
    system("gpio export 18 out");
    system("gpio export 22 in");

    wiringPiSetupSys();
    wiringPiISR (22, INT_EDGE_FALLING, &FlowInterrupt) ;

    timerId = startTimer(500);

    Read_Data();
    update_Screen_Counters();

    setStyleSheet(" .QGroupBox { background-color : white } ");
    //setStyleSheet(" .QLCDNumber{ background-color : black } ");
    ui->SimPourBtn->setAutoRepeat(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_WriteDataBtn_clicked()
{
    Write_Data();
}

void MainWindow::update_Screen_Counters()
{
    ui->lcdPintsPoured->display(pints_poured);
    ui->lcdPintsLeft->display(pints_left);
    ui->lcdOzsPoured->display(ozs_poured);
    ui->lcdOzsLeft->display(ozs_left);
    ui->BeerLevel->setValue(pints_left);
    ui->PintsPouredspinBox->setValue(pints_poured);
}

void MainWindow::on_SimPourBtn_clicked()
{
//   ++TickCounter;
//    if (TickCounter == ticks_per_oz) {
//        ++ozs_poured;
//        ozs_left = (oz_per_pint - ozs_poured);
//        TickCounter = 0;
//        if (pints_left == full_keg) {
//            --pints_left;
//        }

//        if (ozs_poured == oz_per_pint) {
//            ++pints_poured;
//            --pints_left;
//            ozs_poured = 0;
//            ozs_left = 0;
//        }
//    }

//    update_Screen_Counters();
//    qDebug() << TickCounter;

//    ozs_poured = (TickCounter / sim_ticks_per_oz);
//    pints_poured = (ozs_poured / oz_per_pint);
//    ozs_poured = (ozs_poured % oz_per_pint);
//    ozs_left = (oz_per_pint - ozs_poured);
//    pints_left = ((full_keg - 1) - pints_poured);

//    update_Screen_Counters();
//    qDebug() << TickCounter;

}

void MainWindow::on_SimPourBtn_pressed()
{
    ++TickCounter;
//    if (TickCounter == ticks_per_oz) {
//        ++ozs_poured;
//        ozs_left = (oz_per_pint - ozs_poured);
//        TickCounter = 0;
//        if (pints_left == full_keg) {
//            --pints_left;
//        }

//        if (ozs_poured == oz_per_pint) {
//            ++pints_poured;
//            --pints_left;
//            ozs_poured = 0;
//            ozs_left = 0;
//        }
//    }

//    ozs_poured = (TickCounter / sim_ticks_per_oz);
//    pints_poured = (ozs_poured / oz_per_pint);
//    ozs_poured = (ozs_poured % oz_per_pint);
//    ozs_left = (oz_per_pint - ozs_poured);
//    pints_left = ((full_keg - 1) - pints_poured);

//    update_Screen_Counters();
//    qDebug() << TickCounter;
}

void MainWindow::on_ResetBtn_clicked()
{
    TickCounter = 0;
    pints_poured = 0;
    pints_left = full_keg;
    ozs_poured = 0;
    ozs_left = 0;

    Write_Data();
    update_Screen_Counters();

}

void MainWindow::on_PintsPouredspinBox_valueChanged(int arg1)
{
    TickCounter = ((arg1 * oz_per_pint) * ticks_per_oz);
    pints_poured = arg1;
    if (ozs_poured > 0) {
        pints_left = ((full_keg - 1) - pints_poured);
    }
    update_Screen_Counters();
}

void MainWindow::on_ExitButton_clicked()
{
    Write_Data();
    close();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    qDebug() << "Update...";

    ozs_poured = (TickCounter / ticks_per_oz);
    pints_poured = (ozs_poured / oz_per_pint);
    ozs_poured = (ozs_poured % oz_per_pint);
    ozs_left = (oz_per_pint - ozs_poured);
    pints_left = ((full_keg - 1) - pints_poured);

    update_Screen_Counters();
    qDebug() << TickCounter;

}

