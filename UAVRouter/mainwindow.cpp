﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qmessagebox.h"

#include "cogrgeometryfilereader.h"
#include "flightroutedesign.h"
#include "designtaskfactory.h"

#include <qfiledialog.h>
#include "gomologging.h"

#include <QDebug>

#include <QFile>

#include <QException>

#include <QXmlStreamReader>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //ui->btn_unittest->hide();

    ui->editFocus->setReadOnly(true);
    ui->editCamHeight->setReadOnly(true);
    ui->editCamWidth->setReadOnly(true);
    ui->editPixelsize->setReadOnly(true);

    //load the default Camera parameters
    QString focus("55");
    QString h("7760");
    QString w("10328");
    QString pixel("5.2");
    ui->editFocus->setText(focus);
    ui->editCamHeight->setText(h);
    ui->editCamWidth->setText(w);
    ui->editPixelsize->setText(pixel);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_toolButtonAirport_clicked()
{


        QString s = QFileDialog::getOpenFileName(this,"Choose a Airport Location File...",
                            ".",
                            "KML (*.kml *.KML)"
                            );

        if(s.length() == 0)
        {
            QMessageBox::information(NULL, tr("Path error!"), tr("You didn't select any files."));
            ui->labelAirportLocation->setText(s);
        }
        else
        {
            std::auto_ptr<OGRGeometry> airport_geom_ptr =COGRGeometryFileReader::GetFirstOGRPointFromFile(s.toStdString());

            OGRPoint * pPt=dynamic_cast<OGRPoint *>(airport_geom_ptr.get());

            OGRPoint airportLoc(pPt->getX(), pPt->getY());

            //m_flight_param.AirportLocation=airportLoc;
            m_flight_param.airport.SetLocation(airportLoc);

            QFileInfo fi(s);
            QString airportfilebasename=fi.baseName();
            m_flight_param.airport.SetName(airportfilebasename.toStdString());

            char *location=NULL;
            airportLoc.exportToWkt(&location);
            QString strloc(location);
            ui->labelAirportLocation->setText(strloc);
            //QMessageBox::information(NULL, tr("Path"), tr("You selected ") + s);
        }






}

void MainWindow::on_toolButton_Region_clicked()
{

    m_flight_param.ClearFlightRegions();

    if(ui->radioSinglePolygon->isChecked())
    {
        QString s = QFileDialog::getOpenFileName(this,"Choose a Flight Region Defination File...",
                            ".",
                            "KML (*.kml *.KML)"
                            );

        m_flight_param.FightRegion=COGRGeometryFileReader::GetFirstOGRGeometryFromFile(s.toStdString());

        if(s.length() == 0)
        {
            QMessageBox::information(NULL, tr("Path Error"), tr("You didn't select any files."));
        }
        else
        {
            GomoLogging * pLog = GomoLogging::GetInstancePtr();
            std::string logstr("hello!");
            pLog->logging(logstr);
        }

        ui->textRegionFile->setText(s);

    }
    else
    {

        QStringList slist = QFileDialog::getOpenFileNames(this,"Choose Flight Region Files(Use Ctrl+)...",
                            ".",
                            "KML (*.kml *.KML)"
                            );

        if(slist.size()>0)
        {
            GomoLogging * pLog = GomoLogging::GetInstancePtr();

            for(int i=0; i< slist.size(); i++)
            {
                pLog->logging(slist.at(i).toStdString());

                m_flight_param.AddFlightRegionGeometry(
                            COGRGeometryFileReader::GetFirstOGRGeometryFromFile(slist.at(i).toStdString()));
            }

        }
        else
        {
            QMessageBox::information(NULL, tr("Path Error"), tr("You didn't select any files."));
        }



    }




}

void MainWindow::on_cmdDesignStart_clicked()
{

    DigitalCameraInfo cam;

    cam.f=ui->editFocus->text().toDouble();
    cam.width = ui->editCamWidth->text().toLong();
    cam.height = ui->editCamHeight->text().toLong();
    cam.pixelsize = ui->editPixelsize->text().toDouble()/1000.0; //um => mm


    m_flight_param.CameraInfo=cam;

    m_flight_param.AverageElevation= ui->editDatumHeight->text().toDouble();
    m_flight_param.FightHeight=ui->editFlightHeight->text().toDouble();
    m_flight_param.GuidanceEntrancePointsDistance=ui->editGEDist->text().toDouble();
    m_flight_param.overlap=ui->editoverlaprate->text().toDouble()/100.0;
    m_flight_param.overlap_crossStrip=ui->editOverlapSide->text().toDouble()/100.0;
    m_flight_param.RedudantBaselines=ui->editRedudantBaseline->text().toInt();


    //m_flight_param.OutputFilePathname=ui->textOutputFile->toPlainText().toStdString();

    FlightRouteDesign * route_desinger = DesignTaskFactory::CreateFlightRouteDeigner(m_flight_param);
    QFileInfo fi(ui->textOutputFile->toPlainText());
    QString outputbasename=fi.absolutePath()+"/"+fi.baseName();
    QString outputBinary = outputbasename +".bht";
    QString outputText = outputbasename +".ght";
    QString outputKML = outputbasename +".kml";
    QString outputGST = outputbasename +".gst";

    route_desinger->AddOutPutFileName(outputBinary.toStdString());
    route_desinger->AddOutPutFileName(outputText.toStdString());
    route_desinger->AddOutPutFileName(outputKML.toStdString());
    route_desinger->AddOutPutFileName(outputGST.toStdString());
    route_desinger->PerformRouteDesign();
    route_desinger->OutputRouteFile();

    delete route_desinger;
    route_desinger=NULL;

}

void MainWindow::on_toolButtonOutputSelect_clicked()
{
    QString s = QFileDialog::getSaveFileName(this,"Choose a Flight Route Design File...",
                        ".",
                        "ght, bht(*.ght(text file) *.bht(binary file))"
                        );

    ui->textOutputFile->setText(s);


}


void MainWindow::on_btn_unittest_clicked()
{

    DigitalCameraInfo cam;
    cam.f= 50.678; //mm
    cam.width = 20000;
    cam.height = 10000;
    cam.pixelsize = 0.005; //mm

    m_flight_param.CameraInfo=cam;
    m_flight_param.AverageElevation= 0;
    m_flight_param.FightHeight=300; //flight height 300m in default
    m_flight_param.GuidanceEntrancePointsDistance=200;// 200m
    m_flight_param.overlap=0.6;
    m_flight_param.overlap_crossStrip=0.2;
    m_flight_param.RedudantBaselines=2;
    std::string region_filename("G:/Gomo/Data/0209/area.kml");
    m_flight_param.FightRegion=COGRGeometryFileReader::GetFirstOGRGeometryFromFile(region_filename);

    std::string airport_filename("G:/Gomo/Data/0209/bjairport.kml");
    std::auto_ptr<OGRGeometry> airport_geom_ptr =COGRGeometryFileReader::GetFirstOGRPointFromFile(airport_filename);
    OGRPoint * pPt=dynamic_cast<OGRPoint *>(airport_geom_ptr.get());
    OGRPoint airportLoc(pPt->getX(), pPt->getY());
    m_flight_param.airport.SetLocation(airportLoc);
    QString fap(airport_filename.c_str());
    QFileInfo fiairport(fap);
    m_flight_param.airport.SetName(fiairport.baseName().toStdString());

    FlightRouteDesign * route_desinger = DesignTaskFactory::CreateFlightRouteDeigner(m_flight_param);
    QString output("G:/Gomo/Data/0420/testoutput.txt");
    QFileInfo fi(output);
    QString outputbasename=fi.absolutePath()+"/"+fi.baseName();
    QString outputBinary = outputbasename +".bht";
    QString outputText = outputbasename +".ght";
    QString outputKML = outputbasename +".kml";
    QString outputGST = outputbasename +".gst";

    route_desinger->AddOutPutFileName(outputBinary.toStdString());
    route_desinger->AddOutPutFileName(outputText.toStdString());
    route_desinger->AddOutPutFileName(outputKML.toStdString());
    route_desinger->AddOutPutFileName(outputGST.toStdString());
    route_desinger->PerformRouteDesign();
    route_desinger->OutputRouteFile();

    delete route_desinger;
    route_desinger=NULL;




}

void MainWindow::on_MainWindow_iconSizeChanged(const QSize &iconSize)
{

}

void MainWindow::on_radioSinglePolygon_clicked()
{

}

void MainWindow::on_radioMultiPolygon_clicked()
{

}

void MainWindow::on_radioMultiPolygon_toggled(bool checked)
{


}

void MainWindow::on_radioSinglePolygon_toggled(bool checked)
{

}
