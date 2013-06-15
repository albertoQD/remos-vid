#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <opencv2/core/core.hpp>
#include "controller.h"

namespace Ui {
class MainWindow;
}

using namespace std;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_saveAsAction_triggered();
    void on_aboutAction_triggered();
    void on_loadVideoAction_triggered();
    void on_playBtn_clicked();
    void updateVideoFrame(QImage);
    void updateMosaicFrame(QImage);
    void addThumbnail(QImage);
    void resetThumbsAndOutput();

private:
    Ui::MainWindow  *ui;
    Controller      *controller;
    int             gridCurrentRow;
    int             gridCurrentColumn;
    int             numberOfFrames;
    QString         loadedDir;
    QStringList     loadedFiles;
    QImage          generatedImg;
};

#endif // MAINWINDOW_H
