#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QImageReader>
#include <QDebug>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "mattoqimage.h"
#include <vector>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , gridCurrentRow(0)
    , gridCurrentColumn(0)
    , numberOfFrames(0)
{
    controller = new Controller;
    generatedImg = QImage();
    ui->setupUi(this);
    connect(ui->exitAction, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow()
{
    controller->stopThreads();
    controller->deleteThreads();

    delete ui;
}

void MainWindow::on_aboutAction_triggered()
{
    QMessageBox::about(this,
                       "About MOS",
                        QString(" Developed by:\n\n"
                                " * Alberto QUINTERO DELGADO \n"
                                "   <ajquinterod@gmail.com>\n"));
}

void MainWindow::on_saveAsAction_triggered()
{
    if (generatedImg.isNull())
    {
        statusBar()->showMessage(tr("Must generate a Mosaiced Image first"));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
            this,
            tr("Save Image"),
            QDir::toNativeSeparators(QDir::homePath()),
            tr("PNG Files (*.png)") );

    if (filename.isEmpty())
        return;

    if (filename.mid(filename.size()-4) != ".png")
        filename += ".png";

    // saving image
    if (generatedImg.save(filename))
    {
        statusBar()->showMessage(tr("Image successfully saved in ") + filename);
    }
    else
    {
        statusBar()->showMessage(tr("Error saving image"));
    }

}

void MainWindow::on_loadVideoAction_triggered()
{
    QString filename = QFileDialog::getOpenFileName(
            this,
            tr("Select Video to Open"),
            QDir::toNativeSeparators(QDir::homePath()),
            tr("Videos (*.avi)"));

    if (!filename.isEmpty())
    {
        resetThumbsAndOutput();

        if (controller->readVideo(filename))
        {
            // CLEAN THE MOSAICED IMAGE

            connect(controller, SIGNAL(newInputFrame(QImage)), this, SLOT(updateVideoFrame(QImage)));
            connect(controller, SIGNAL(newInputFrame(QImage)), this, SLOT(addThumbnail(QImage)));
            connect(controller->processingThread, SIGNAL(newProcessedFrame(QImage)), this, SLOT(updateMosaicFrame(QImage)));

            // start already reading the video and pause it - TO DO
            ui->playBtn->setEnabled(true);
            statusBar()->showMessage(tr(QString("Loaded video: " + filename).toStdString().data()));
        }
    }
}

void MainWindow::addThumbnail(QImage frame)
{
    QLabel *img = new QLabel();
    QImage preview = frame.scaled(110, 94);

    img->setScaledContents(true);
    img->setPixmap(QPixmap::fromImage(preview));
    img->setFrameStyle(QFrame::StyledPanel);

    // insert the qlabel in the grid
    ui->imagesGrid->addWidget(img, gridCurrentRow, gridCurrentColumn);
    numberOfFrames += 1;
    ui->informationLabel->setText(QString("Number of images used: ") + QString().setNum(numberOfFrames));

    // update r & c
    if (gridCurrentColumn == 2) gridCurrentRow += 1, gridCurrentColumn = 0;
    else gridCurrentColumn+=1;
}

void MainWindow::updateVideoFrame(QImage frame)
{
    ui->videoLabel->setPixmap(QPixmap::fromImage(frame));
}

void MainWindow::updateMosaicFrame(QImage mos)
{
    generatedImg = mos;
    ui->outputLabel->setPixmap(QPixmap::fromImage(mos));
    statusBar()->showMessage(tr("Mosaic generated ..."));
}

void MainWindow::on_playBtn_clicked()
{
    if (!controller->captureThread->isPaused() && !controller->captureThread->isStopped())
    { // pause it
        controller->captureThread->pause();
        controller->processingThread->pause();
        ui->playBtn->setStyleSheet(QString("QPushButton#playBtn {"
                                           "border: none;"
                                           "background: url(:/imgs/pause.png) no-repeat transparent;"
                                           "background-position: center center;"
                                           "background-color: qlineargradient(spread:reflect, x1:0, y1:1, x2:1, y2:0, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(192, 192, 192, 255));"
                                           "}"

                                           "QPushButton#playBtn:pressed {"
                                           "border: none;"
                                           "background: url(:/imgs/pause.png) no-repeat transparent;"
                                           "background-position: center center;"
                                           "background-color: qlineargradient(spread:reflect, x1:1, y1:0, x2:0, y2:1, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(255, 255, 255, 255));"
                                           "}"
                                           "QPushButton#playBtn:focus {"
                                           "border: none;"
                                           "outline: none;"
                                           "}"));
    }
    else
    { // run it
        controller->captureThread->play();
        controller->processingThread->play();
        ui->playBtn->setStyleSheet(QString("QPushButton#playBtn {"
                                           "border: none;"
                                           "background: url(:/imgs/play.png) no-repeat transparent;"
                                           "background-position: center center;"
                                           "background-color: qlineargradient(spread:reflect, x1:0, y1:1, x2:1, y2:0, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(192, 192, 192, 255));"
                                           "}"

                                           "QPushButton#playBtn:pressed {"
                                           "border: none;"
                                           "background: url(:/imgs/play.png) no-repeat transparent;"
                                           "background-position: center center;"
                                           "background-color: qlineargradient(spread:reflect, x1:1, y1:0, x2:0, y2:1, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(255, 255, 255, 255));"
                                           "}"
                                           "QPushButton#playBtn:focus {"
                                           "border: none;"
                                           "outline: none;"
                                           "}"));
    }
}

void MainWindow::resetThumbsAndOutput()
{
    // Cleaning the grid
    qDeleteAll(ui->widget->children());
    ui->imagesGrid = new QGridLayout;
    ui->imagesGrid->setObjectName(QString::fromUtf8("imagesGrid"));
    ui->widget->setLayout(ui->imagesGrid);
    delete controller;
    controller = new Controller;

    // cleaning the information label
    numberOfFrames = 0;
    gridCurrentColumn = 0;
    gridCurrentRow = 0;
}
