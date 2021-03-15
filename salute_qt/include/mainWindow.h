#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QCheckbox>
#include <QMessagebox>
#include "paintArea.h"

QT_BEGIN_NAMESPACE
class QGroupBox;
class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
QT_END_NAMESPACE

//! [0]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();


protected:

    void MainWindow::closeEvent(QCloseEvent* event) override;

private:

    void manageLayout();
    void manageConnections();

    PaintArea* paintArea;

    QPushButton* playBut;
    QPushButton* pauseBut;
    QPushButton* stopBut;
    QPushButton* exitBut;

    QSlider* freqSli; // salut frequency
    QLabel* freqLab;

    QSlider* expRadSli;
    QLabel* expRadLab;

    QCheckBox* infoCB;
    
    QGroupBox* gridGroupBox;

};


#endif