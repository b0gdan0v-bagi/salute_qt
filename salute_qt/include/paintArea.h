#pragma once

#include <QMutex>
#include <QWidget>
#include <QImage>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QRandomGenerator>

#include "particle.h"
#include <QElapsedTimer>
#include <chrono>
#include <list>

class PaintArea : public QWidget
{
    // Declares our class as a QObject which is the base class
    // for all Qt objects
    // QObjects handle events
    Q_OBJECT

public:
    PaintArea(QWidget* parent = nullptr);

    enum State {PLAY, PAUSE, STOP} state = STOP;

    void setShowInfo(bool show) { showInfo = show; }
    void setSaluteRate(int rate) { saluteRate = rate; }
    void setExpRadius(int r) { expRadMod = r; }
    void setWind(int w) { windX = w; }

public slots:
    void clearImage();


protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

private:
    void clearDeadLines();
    void resizeImage(QImage* image, const QSize& newSize);
    void draw();
    


    bool showInfo = { true };
    int saluteRate;
    int expRadMod;
    int windX = { 0 };

    QList<FireLine*> lines;
    QList<QVector2D> userAdd;

    int timer;

    QMutex mutex;
    QImage image;

    QString infoStr;
    uint32_t nPart;
    uint32_t nSalutes = { 0 };

    const float PI = 3.14159265358f;

};