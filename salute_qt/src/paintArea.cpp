#include "paintArea.h"

#include <QMouseEvent>
#include <QPainter>



//! [0]
PaintArea::PaintArea(QWidget* parent)
    : QWidget(parent)
{
    clearImage();
    // make loop inside timerEvent method
    timer = QObject::startTimer(50);

}


void PaintArea::clearImage()
{
    image.fill(Qt::black);
    update();
}

void PaintArea::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && state == State::PLAY) {
        QVector2D toAdd(event->pos());
        if (toAdd.x() > 0 && toAdd.x() < width() && toAdd.y() > 0 && toAdd.y() < height())
            userAdd.push_back(QVector2D(event->pos()));
    }
    
}



void PaintArea::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QRect dirtyRect = event->rect();
    painter.drawImage(dirtyRect, image, dirtyRect);
}

void PaintArea::resizeEvent(QResizeEvent* event)
{
    if (width() > image.width() || height() > image.height()) {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
        update();
    }
    QWidget::resizeEvent(event);
}



void PaintArea::clearDeadLines()
{
   
    //if (!painter.isActive())

    /*for (auto l = lines.begin(); l != lines.end();)
    {
        if ((*l)->life == false)
        {
            delete (*l);
            lines.erase(l);
        }
        else l++;
    }*/
    if (lines.empty()) return;

    //mutex.lock();

    QList<FireLine*>::iterator end = lines.end();
    QList<FireLine*>::iterator it = lines.begin();
    while (it != end)
    {
        //if ((*it)->line.last()->isAlive() == false)
        if ((*it)->readyToDelete())
        {
            //for (auto it2 = (*it)->line.begin(); it2 != (*it)->line.end(); it2++) delete* it2;
            delete* it;
            it = lines.erase(it);
        }
        else ++it;
    }

    //mutex.unlock();
}

void PaintArea::resizeImage(QImage* image, const QSize& newSize)
{
    if (image->size() == newSize)
        return;

    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(Qt::black);
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}



void PaintArea::timerEvent(QTimerEvent* event)
{
    switch (state)
    {
    case PaintArea::PLAY:
    {
        auto start = std::chrono::high_resolution_clock::now();

        // random generator
        QRandomGenerator* rg = QRandomGenerator::global();

        

        nPart = 0; // calculate number of particles

        for (auto& l : lines)
        {
            l->update(); // update all math
            if (l->isAlive()) nPart += l->line.size(); // not exact number if not all particles life but faster
        }
        // generate random lines
        if (rg->generateDouble() > 0.2f + saluteRate * 0.1f)
        {
            lines.push_back(new FireLine(
                QVector2D(width() * (0.25 + 0.5 * rg->generateDouble()), height()), // start pos

                QVector2D(-2 + QRandomGenerator::global()->bounded(0, 4), // speed x
                    -3 + -1 * QRandomGenerator::global()->bounded(1, 5)), // speed y
                QRandomGenerator::global()->bounded(40, 80), // health
                QRandomGenerator::global()->bounded(5, 20),  // length
                true,   // first?
                QColor::fromRgb(QRandomGenerator::global()->generate()), // color
                QRandomGenerator::global()->bounded(4, 7) // size
            ));
            lines.back()->setWind(windX);
            nSalutes++;
        }
        // generate user lines
        for (auto& ua : userAdd)
        {
            QVector2D startPos(width() * (0.25 + 0.5 * rg->generateDouble()), height());
            QVector2D speedVec(ua - startPos);
            speedVec /= speedVec.length();
            speedVec *= QRandomGenerator::global()->bounded(5, 10);
            lines.push_back(new FireLine(
                startPos,
                speedVec,
                1000, // health, should explode at height 
                QRandomGenerator::global()->bounded(5, 20),  // length
                true,   // parent
                Qt::green, // fixed color to see that is made by user
                7 // bigger size
            ));
            lines.back()->setExplodedHeight(ua.y());
            lines.back()->setWind(windX);
            nSalutes++;
        }
        userAdd.clear();

        // store explode positions and parent valus
        QList<Particle> expPos;
        for (auto& l : lines)
        {
            if (l->isExploded() && l->isParent())
            {
                l->setParentAtribute(false); // now that line cant generate new lines
                Particle p = *l->line.front();
                expPos.push_back(p);
            }
        }
        // add child lines
        for (auto& ep : expPos)
        {
            //vector rounding around (0,0) with start coord (x,0)
            //also its speed of 2nd generated lines of salut
            const float roundedVec = QRandomGenerator::global()->bounded(2, 5);

            const int numberOfLines = QRandomGenerator::global()->bounded(5, 15);
            const int lengthL = QRandomGenerator::global()->bounded(4, 12);
            //const int healthL = QRandomGenerator::global()->bounded(expRadMod, 2*expRadMod);
            int healthL = 3*expRadMod;
            if (expRadMod == 1)
                healthL = QRandomGenerator::global()->bounded(8, 20);
            for (float phi = 0; phi <= 2 * PI; phi += PI / (1.f * numberOfLines))
            {
                lines.push_back(new FireLine(
                    ep.getPos(), // stored pos of exloded
                    QVector2D(roundedVec * cos(phi), roundedVec * sin(phi)), // speed
                    healthL, // health
                    lengthL,  // length
                    false,    // cant generate child lines
                    ep.getColor(), // same color as parent
                    ep.getSize() // same size as parent
                ));
            }
        }
        //expPos.clear();

        auto finishMath = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedMath = finishMath - start;



        // draw everything
        draw();

        auto finishPaint = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedPaint = finishPaint - finishMath;

        infoStr = " FPS: " + QString::number(1 / (elapsedMath + elapsedPaint).count()) +
            "\n Particles: " + QString::number(nPart) +
            "\n math time: " + QString::number(elapsedMath.count() * 1000) + " ms" +
            "\n paint time: " + QString::number(elapsedPaint.count() * 1000) + " ms" +
            "\n salutes: " + QString::number(nSalutes);

        // delete "dead" lines
        clearDeadLines();

        break;
    }
        
    case PaintArea::PAUSE:
    {
        draw();
        break;
    }  
    case PaintArea::STOP:
    {
        qDeleteAll(lines.begin(), lines.end());
        lines.clear();
        userAdd.clear();
        clearImage();
        nSalutes = 0;
        break;
    }
    }


}



void PaintArea::draw()
{
    //mutex.lock();
    QPainter painter(&image);
    image.fill(Qt::black);
    
    painter.setRenderHint(QPainter::Antialiasing, true);

   for (auto& l : lines)
    {
        for (auto& p : l->line)
        {
            
            painter.setPen(QPen(p->getColor(), p->getSize(), Qt::SolidLine, Qt::RoundCap,
                Qt::RoundJoin));
            if (p->isAlive()) painter.drawPoint(p->getPos().toPoint());

        }
    }
   if (showInfo)
   {
       painter.setPen(QPen(Qt::white, 5, Qt::SolidLine, Qt::RoundCap,
           Qt::RoundJoin));
       painter.drawText(this->rect(), Qt::AlignLeft, infoStr);
   }
   
    update();
   // mutex.unlock();

}



