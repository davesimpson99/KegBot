#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wiringPi.h"
#include <QTimer>
#include <QDateTime>
#include <QDebug>

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <QFile>

//  TickCounter:
//	Global variable to count interrupts
//	Should be declared volatile to make sure the compiler doesn't cache it.
static volatile int TickCounter;
static volatile int SessionCount;



/* Kegbot Interrupt */
void FlowInterrupt (void)
{
    ++TickCounter;
    ++SessionCount;
    /* qDebug() << "Got One..."; */

}
/* End Kegbot Interrupt */

const int half_barrel = 103;
const int sixth = 34;
const int sim_ticks_per_oz = 4;
const int ticks_per_oz = 166;
const int oz_per_pint = 20;

int full_keg = half_barrel;
int pints_poured = 0;
int pints_left = full_keg;
int total_ozs_poured = 0;
int last_ozs_poured = 0;
int ozs_poured = 0;
int ozs_left = 0;
int tempF = 0;
int thermocount = 0;
int SessionPints = 0;
int SessionOzs = 0;
int keg_type = 0; //0 = Full 1 = Sixth

QDate thedate;


void Thermo () {
 DIR *dir;
 struct dirent *dirent;
 char dev[16];      // Dev ID
 char devPath[128]; // Path to device
 char buf[256];     // Data from device
 char tmpData[6];   // Temp C * 1000 reported by device
 char path[] = "/sys/bus/w1/devices";
 ssize_t numRead;

 dir = opendir (path);
 if (dir != NULL)
 {
  while ((dirent = readdir (dir)))
   // 1-wire devices are links beginning with 28-
   if (dirent->d_type == DT_LNK &&
     strstr(dirent->d_name, "28-") != NULL) {
    strcpy(dev, dirent->d_name);
    //printf("\nDevice: %s\n", dev);
   }
        (void) closedir (dir);
        }
 else
 {
  qDebug() << "Couldn't open the w1 devices directory";
  return; //return 1;
 }

        // Assemble path to OneWire device
 sprintf(devPath, "%s/%s/w1_slave", path, dev);
 // Read temp continuously
 // Opening the device's file triggers new reading
 //while(1) {
  int fd = open(devPath, O_RDONLY);
  if(fd == -1)
  {
   qDebug() << "Couldn't open the w1 device.";
   return; //return 1;
  }
  while((numRead = read(fd, buf, 256)) > 0)
  {
   strncpy(tmpData, strstr(buf, "t=") + 2, 5);
   float tempC = strtof(tmpData, NULL);
   //printf("Device: %s  - ", dev);
   //printf("Temp: %.3f C  ", tempC / 1000);
   //printf("%.3f F\n\n", (tempC / 1000) * 9 / 5 + 32);

   tempF = (tempC / 1000) * 9 / 5 + 32;

  }
  close(fd);
 //}
}

void Write_Data()
{
//OPEN DATA FILE IN OUR APPLICAITONS DIRECTORY OR CREATE IT IF IT DOESN'T EXIST
    FILE *file1;
    unsigned char file_data[5];
    const char *filename1 = "kegbot.dat";

    file1 = fopen(filename1, "wb");
    if (file1)
    {
        //----- FILE EXISTS -----
        file_data[0] = pints_poured;
        file_data[1] = ozs_poured;
        file_data[2] = pints_left;
        file_data[3] = ozs_left;
        file_data[4] = keg_type;


        fwrite(&file_data[0], sizeof(unsigned char), 5, file1) ;

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
            file_data[4] = keg_type;

            fwrite(&file_data[0], sizeof(unsigned char), 5, file1) ;

            fclose(file1);
            file1 = NULL;
        }
    }
}


void Read_Data()
{
    //OPEN DATA FILE IN OUR APPLICAITONS DIRECTORY
    FILE *file1;
    unsigned char file_data[5];
    const char *filename1 = "kegbot.dat";

    file1 = fopen(filename1, "rb");
    if (file1)
    {
        //----- FILE EXISTS -----
        fread(&file_data[0], sizeof(unsigned char), 5, file1);

        //qDebug() << "File opened, some byte values: %i %i %i %i\n", file_data[0], file_data[1], file_data[2], file_data[3]);
        qDebug() << file_data[0];
        qDebug() << file_data[1];
        qDebug() << file_data[2];
        qDebug() << file_data[3];
        qDebug() << file_data[4];

        pints_poured = file_data[0];
        ozs_poured = file_data[1];
        pints_left = file_data[2];
        ozs_left = file_data[3];
        keg_type = file_data[4];

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

    //Set up Timers and Clocks
    clocktimer = new QTimer(this);
    connect(clocktimer, SIGNAL(timeout()),this,SLOT(updateClock()));
    timerId = startTimer(500);
    clocktimer->start(1000);

    thedate=QDate::currentDate();
    QString datetext=thedate.toString();
    ui->label_dateTime->setText(datetext);

    //Set up WiringPI
    setenv("WIRINGPI_GPIOMEM", "1", 1);
    system("gpio mode 0 out");
    // LED
    system("gpio export 18 out");    
    // Temp Sensor
    system("gpio export 17 in");
    // Flow Meter
    system("gpio export 22 in");

    wiringPiSetupSys();
    wiringPiISR (22, INT_EDGE_FALLING, &FlowInterrupt) ;

    //Make a copy of the Kegbot.dat file
    if (QFile::exists("kegbot.bak"))
    {
        QFile::remove("kegbot.bak");
    }

    QFile::copy("kegbot.dat", "kegbot.bak");

    Read_Data();
    if (keg_type == 1) //Sixth Keg
    {
        ui->FullKeg->setVisible(false);
        ui->SixthKeg->setVisible(true);
        ui->BeerLevel->setMaximum(34);
        full_keg = sixth;
    }
    else if (keg_type == 0) //Full Keg
    {
        ui->FullKeg->setVisible(true);
        ui->SixthKeg->setVisible(false);
        ui->BeerLevel->setMaximum(103);
        full_keg = half_barrel;
    }
    TickCounter = (((pints_poured * oz_per_pint) * ticks_per_oz) + (ozs_poured * ticks_per_oz));

    Thermo();
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
    //ui->PintsPouredspinBox->setValue(pints_poured);
    ui->TemplcdNumber->display(tempF);
    ui->ThislcdNumber->display(SessionPints);
    ui->SessionOzsPoured->display(SessionOzs);
}

void MainWindow::on_SimPourBtn_clicked()
{
   ++TickCounter;
   ++SessionCount;

}

void MainWindow::on_SimPourBtn_pressed()
{
    //++TickCounter;
    TickCounter = (TickCounter + 100);
    ++SessionCount;

}

void MainWindow::on_ResetBtn_clicked()
{
    TickCounter = 0;
    SessionCount = 0;
    pints_poured = 0;
    pints_left = full_keg;
    ozs_poured = 0;
    ozs_left = 0;

    Write_Data();
    update_Screen_Counters();

}

void MainWindow::on_PintsPouredspinBox_valueChanged(int arg1)
{
    pints_poured = arg1;

    TickCounter = (((pints_poured * oz_per_pint) * ticks_per_oz) + (ozs_poured * ticks_per_oz));

    if (ozs_poured > 0) {
        pints_left = (full_keg - pints_poured);
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
    //qDebug() << "Update...";

    total_ozs_poured = (TickCounter / ticks_per_oz);
    pints_poured = (total_ozs_poured / oz_per_pint);
    ozs_poured = (total_ozs_poured % oz_per_pint);
    if (ozs_poured > 0) {
        ozs_left = (oz_per_pint - ozs_poured);
        pints_left = ((full_keg -1) - pints_poured);
    }
    else {
        pints_left = (full_keg - pints_poured);
    }


    if (total_ozs_poured != last_ozs_poured) {
        Write_Data();
    }

    SessionPints = (SessionCount / (ticks_per_oz * oz_per_pint));
    SessionOzs = ((SessionCount/ticks_per_oz) % oz_per_pint);

    ++thermocount;

    if (thermocount == 40) //every 20 seconds
    {
        Thermo();
        thermocount = 0;
    }

    update_Screen_Counters();
    //qDebug() << TickCounter;

}
void MainWindow::updateClock()
{
    QDate todaysdate=QDate::currentDate();
    QTime time = QTime::currentTime();
    QString time_text = time.toString("hh : mm : ss");
    ui->label_date_time->setText(time_text);

    if (thedate != todaysdate) {
        QString datetext=thedate.toString();
        ui->label_dateTime->setText(datetext);
    }

}

void MainWindow::on_CB_KegSelect_currentIndexChanged(const QString &arg1)
{
    if (arg1 == "Sixth Keg") {
        ui->FullKeg->setVisible(false);
        ui->SixthKeg->setVisible(true);
        ui->BeerLevel->setMaximum(34);
        full_keg = sixth;
        keg_type = 1;
    }
    else if (arg1 == "Full Keg") {
        ui->FullKeg->setVisible(true);
        ui->SixthKeg->setVisible(false);
        ui->BeerLevel->setMaximum(103);
        full_keg = half_barrel;
        keg_type = 0;
    }
}
