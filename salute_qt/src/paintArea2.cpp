#include "paintArea.h"



PaintArea::PaintArea(QWidget* parent)
    : QWidget(parent), QObject(parent)
{
    // Roots the widget to the top left even if resized
    setAttribute(Qt::WA_StaticContents);

    // Set defaults for the monitored variables
    modified = false;
    scribbling = false;
    myPenWidth = 1;
    myPenColor = Qt::blue;
    scribblemodes = NONE; // test
    nNodes = 2;
    //setMouseTracking(true);
    updateShapes();

    //arrayOfPotencials.resize()
    temp_vector = QVector<float>(1920,0);
    //arrayOfPotencials = QVector<QVector<float>>(1080, temp_vector);

    QTimer* TESTtimer = new QTimer(this);
    TESTtimer = new QTimer(this);
    //QObject::connect(TESTtimer, &QTimer::timeout, this, [=]() {TESTtimer->start(1000); });
    //TESTtimer->start(1000);
    TEST_timerId = QObject::startTimer(50);

    //pArrayOfPotencials = &arrayOfPotencials;

}

void PaintArea::timerEvent(QTimerEvent* event)
{
    //simulateMovement();
    //clearImage();
    //drawText(QPoint(50, 50), "test " + QString::number(mousePoint.x()));
    if (scribblemodes == SIMULATE) startSimulateMovement();
    else stopSimulateMovement();

    //drawText(QPoint(50, 100), "test " + QString::number());
}





void PaintArea::updateShapes()
{
    clearImage();
    
    for (auto const& sh : shapes)
    {
        switch (sh->shapeType)
        {
        case sShape::ShapeType::POINT: {
            QColor chargeColor;
            chargeColor = (sh->charge > 0) ? Qt::red : Qt::blue;
            drawRectangle(sh->vecNodes[0].pos.toPoint(), chargeColor); break; }
        case sShape::ShapeType::LINE: {
            QColor chargeColor;
            chargeColor = (sh->charge > 0) ? Qt::red : Qt::blue;
            for (int i = 0; i < sh->vecNodes.size(); i++) drawRectangle(sh->vecNodes.at(i).pos.toPoint(), chargeColor);
            // "if" for checking only
            if (sh->vecNodes.size() > 1) drawLineBetween(sh->vecNodes.at(0).pos.toPoint(), sh->vecNodes.at(1).pos.toPoint(), Qt::black);
            break; }
        case sShape::ShapeType::MOVING_POINT: {
            drawRectangle(sh->vecNodes[0].pos.toPoint(), Qt::darkCyan); //draw start position
            
            sh->forceVec = summaryForceInPoint(*sh);

            drawArrow(sh->movingPos, sh->forceVec); // vec of force
            if (sh->interactable) {
                drawRectangle(sh->movingPos.toPoint(), sh->getColor()); // draw charge in it place
                drawArrow(sh->movingPos, sh->forceVec); // vec of force
                drawArrow(sh->movingPos, sh->speedVec, 10, 5, Qt::darkMagenta); // vec of speed
            }
            drawArrow(sh->vecNodes[0].pos, sh->vecNodes[1].pos); break; }
        }
    }
    // if something changed potencial should be recalculated
    if (potShouldReCalc && drawPotMap) calculatePotencial();
    // draw potencial heat map here
    if (!potShouldReCalc && drawPotMap) drawPotencialAllArea();
    // draw eq pot lines. near mouse and every stored point
    if (drawEqPotLines)
    {
        calcEqPot(mousePoint);
        for (auto &pts : storageEqPts) calcEqPot(pts);
    }
    
    // draw powerlines near mouse or from/to charge
    if (drawPowerLines) drawAllPowerLines();
        
    // draw electric field (lines)
    if (drawElField) drawElFieldAllArea();

    // then in simulating mode it will store points where charges were before to show trajectories
    if (showTrajectories) drawTrajectoriesAllArea();
        
    QString abraCADABRA;
    for (int i = 0; i < 16; i++)
        abraCADABRA += QString::number(pArrayPot[i]) + " ";
    drawText(QPoint(50, 50), abraCADABRA);
}


// If a mouse button is pressed check if it was the
// left button and if so store the current position
// Set that we are currently drawing
void PaintArea::mousePressEvent(QMouseEvent* event)
{
    //if (scribblemodes == SIMULATE) TESTtimer->start(50);
    //if (scribblemodes == SIMULATE) TESTtimer->start(1000);

    if (event->button() == Qt::LeftButton) {
        // Make sure user we destroy line if user want to switch another thing while LINE havent finished
        if (scribblemodes != LINE && scribblemodes != MOVING_POINT) nNodes = 2;


        switch (scribblemodes)
        {
        case PaintArea::NONE: {
            lastPoint = event->pos();
            scribbling = true;
            break; }
        case PaintArea::LINE:
        {
            if (nNodes == 2)
            {
                tempPoint = QVector2D(event->pos());// 
                drawRectangle(tempPoint.toPoint());
                nNodes--;
            }
            else if (nNodes == 1)
            {
                nNodes++;
                shapes.push_back(new sLine(tempPoint, QVector2D(event->pos()), chargeToAdd));
                potShouldReCalc = true;
                updateShapes();
            }
            break; 
        }
        case PaintArea::POINT: {
            shapes.push_back(new sPoint(QVector2D(event->pos()),chargeToAdd));
            potShouldReCalc = true;
            updateShapes();
            break; }
        case PaintArea::MOVING_POINT:
        {
            if (nNodes == 2)
            {
                tempPoint = QVector2D(event->pos());// 
                drawRectangle(tempPoint.toPoint());
                nNodes--;
                //setMouseTracking(true);
            }
            else if (nNodes == 1)
            {
                nNodes++;
                shapes.push_back(new sMovingPoint(tempPoint, QVector2D(event->pos()), chargeToAdd));
                potShouldReCalc = true;
                updateShapes();
            }
            break;
        }

        case PaintArea::CYLINDER:
            break;
        case PaintArea::EQPOTLINES: {storageEqPts.push_back(event->pos());; break; }
        case PaintArea::TRACKING: {
            if (clickNearShapeNode(QVector2D(event->pos()))) nodeHited = true;

            break;
        }
        case PaintArea::DIRECTIONS:
            break;
        }
        emit dataReady(shapes);
    }
}

// When the mouse moves if the left button is clicked
// we call the drawline function which draws a line
// from the last position to the current
void PaintArea::mouseMoveEvent(QMouseEvent* event)
{
    
    //drawText(QPoint(50, 50), "width = " + QString::number(width()) + "\nheight = " + QString::number(height()));
    //drawText(QPoint(50, 50), "N = " + QString::number(scribblemodes));
    //mousePoint = event->pos();
    mouseVector = inboundVector( QVector2D(event->pos()));
    mousePoint = mouseVector.toPoint();

    if (!vectorInbound(QVector2D(event->pos()))) updateShapes();

    switch (scribblemodes)
    {
    case PaintArea::NONE: {
        if ((event->buttons() & Qt::LeftButton) && scribbling) drawLineTo(event->pos());
        break; }
    case PaintArea::LINE: {
        if (nNodes == 1)
        {
            updateShapes();
            drawRectangle(tempPoint.toPoint());
            drawLineBetween(tempPoint.toPoint(), mousePoint, Qt::blue);
            drawRectangle(mousePoint);
        }
        else {
            updateShapes(); 
            drawRectangle(mousePoint);
        }
        break; }
    case PaintArea::POINT: {updateShapes(); drawRectangle(mousePoint); break; }
    case PaintArea::MOVING_POINT: {
        if (nNodes == 1)
        {
            updateShapes();
            drawRectangle(tempPoint.toPoint());
            drawArrow(tempPoint, mouseVector);
        }
        else {
            updateShapes();
            drawRectangle(mousePoint);
        }
        break; }
    case PaintArea::CYLINDER:
        break;
    case PaintArea::EQPOTLINES: { storePtsEqPotLines = true; updateShapes(); break; }
    case PaintArea::DIRECTIONS: break;
    case PaintArea::SIMULATE: { 
        if (mouseInboundArea()) drawToolTip(event);
        break; }
    case PaintArea::TRACKING: {
        if (nodeHited)
        {
            for (auto& sh : shapes)
                for (auto& node : sh->vecNodes)
                    if (node.attachedToCursor) {
                         
                        node.pos =  mouseVector;
                        
                        potShouldReCalc = true;
                        updateShapes();
                    }
        }

        //setMouseTracking(true);
        if (event->pos() == mousePoint) drawToolTip(event);
        break;
    }
    }
    storePtsEqPotLines = false;
}

// If the button is released we set variables to stop drawing
void PaintArea::mouseReleaseEvent(QMouseEvent* event)
{
    if (scribblemodes == NONE)
    {
        if (event->button() == Qt::LeftButton && scribbling) {
            drawLineTo(event->pos());
            scribbling = false;
        }
    }
    if (nodeHited)
        for (auto& sh : shapes)
            for (auto& node : sh->vecNodes)
                node.attachedToCursor = false;
    nodeHited = false;
}

// QPainter provides functions to draw on the widget
// The QPaintEvent is sent to widgets that need to
// update themselves
void PaintArea::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    // Returns the rectangle that needs to be updated
    QRect dirtyRect = event->rect();

    // Draws the rectangle where the image needs to
    // be updated
    painter.drawImage(dirtyRect, image, dirtyRect);
}

// Resize the image to slightly larger then the main window
// to cut down on the need to resize the image
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



void PaintArea::drawCylTo(QPoint &point, qreal WIDTH = 10)
{
    QPainter painter(&image);
    painter.setPen(QPen(myPenColor, WIDTH, Qt::SolidLine, Qt::RoundCap,
        Qt::RoundJoin));
    QPoint m_point = point;

    painter.drawRoundRect(QRect(point,point+point),50,50);
    //painter.drawRoundRect()
    int rad = (myPenWidth / 2) + 2;

    // Call to update the rectangular space where we drew
    update();
}


// When the app is resized create a new image using
// the changes made to the image
void PaintArea::resizeImage(QImage* image, const QSize& newSize)
{
    // Check if we need to redraw the image
    if (image->size() == newSize)
        return;

    //temp_vector = QVector<float>(width(), 0);
    //arrayOfPotencials = QVector<QVector<float>>(height(), temp_vector);

    // Create a new image to display and fill it with white
    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(qRgb(255, 255, 255));

    // Draw the image
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}


