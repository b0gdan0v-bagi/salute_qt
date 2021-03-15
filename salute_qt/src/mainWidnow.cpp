#include <QtWidgets>
#include "mainWindow.h"

//! [1]
MainWindow::MainWindow()
{
	paintArea = new PaintArea(this);

	playBut = new QPushButton("play", this);
	pauseBut = new QPushButton("pause", this);
	stopBut = new QPushButton("stop", this);
	exitBut = new QPushButton("quit", this);

	freqSli = new QSlider(Qt::Horizontal,this);
	freqSli->setMaximumWidth(200);
	freqSli->setMinimum(1);
	freqSli->setMaximum(5);
	freqSli->setValue(3);
	freqLab = new QLabel("Salute rate " + QString::number(freqSli->value()), this);
	paintArea->setSaluteRate(freqSli->value());

	expRadSli = new QSlider(Qt::Horizontal, this);
	expRadSli->setMaximumWidth(200);
	expRadSli->setMinimum(1);
	expRadSli->setMaximum(10);
	expRadSli->setValue(3);
	expRadLab = new QLabel("Exp radius " + QString::number(expRadSli->value()), this);
	paintArea->setExpRadius(expRadSli->value());

	windSli = new QSlider(Qt::Horizontal, this);
	windSli->setMaximumWidth(200);
	windSli->setMinimum(-3);
	windSli->setMaximum(3);
	windSli->setValue(0);
	windLab = new QLabel(windIs(windSli->value()), this);
	paintArea->setWind(windSli->value());

	infoCB = new QCheckBox("Show info", this);
	infoCB->setChecked(true);
	paintArea->setShowInfo(infoCB->isChecked());

	manageLayout();
	manageConnections();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	PaintArea::State state = paintArea->state;
	paintArea->state = PaintArea::State::PAUSE;

	QMessageBox::StandardButton ret;
	ret = QMessageBox::warning(this, tr("Salut"),
		tr("Do you want to close app?"),
		QMessageBox::Yes | QMessageBox::No);
	if (ret == QMessageBox::Yes)
		event->accept();
	else
	{
		paintArea->state = state;
		event->ignore();
	}
}

void MainWindow::manageLayout()
{
	QGridLayout* gridLayout = new QGridLayout;

	gridLayout->addWidget(paintArea, 0, 0, 1, 4);
	gridLayout->addWidget(playBut, 1, 0);
	gridLayout->addWidget(pauseBut, 2, 0);
	gridLayout->addWidget(stopBut, 3, 0);
	gridLayout->addWidget(exitBut, 3, 2);
	gridLayout->addWidget(infoCB, 1, 2);



	QHBoxLayout* freqLay = new QHBoxLayout;
	freqLay->addWidget(freqSli);
	freqLay->addWidget(freqLab);

	QHBoxLayout* expRadLay = new QHBoxLayout;
	expRadLay->addWidget(expRadSli);
	expRadLay->addWidget(expRadLab);

	QHBoxLayout* windLay = new QHBoxLayout;
	windLay->addWidget(windSli);
	windLay->addWidget(windLab);

	gridLayout->addLayout(freqLay, 1, 1);
	gridLayout->addLayout(expRadLay, 2, 1);
	gridLayout->addLayout(windLay, 3, 1);


	QWidget* w = new QWidget();
	w->setLayout(gridLayout);
	setCentralWidget(w);
}

void MainWindow::manageConnections()
{
	connect(playBut, &QPushButton::clicked, this, [=]() {paintArea->state = PaintArea::State::PLAY; });
	connect(pauseBut, &QPushButton::clicked, this, [=]() {paintArea->state = PaintArea::State::PAUSE; });
	connect(stopBut, &QPushButton::clicked, this, [=]() {paintArea->state = PaintArea::State::STOP; });
	connect(exitBut, &QPushButton::clicked, this, &MainWindow::close);
	connect(infoCB, &QCheckBox::clicked, this, [=]() { paintArea->setShowInfo(infoCB->isChecked()); });
	connect(freqSli, QOverload<int>::of(&QSlider::valueChanged), this, [=]() {
		freqLab->setText("Salute rate " + QString::number(freqSli->value()));
		paintArea->setSaluteRate(freqSli->value());
		});
	connect(expRadSli, QOverload<int>::of(&QSlider::valueChanged), this, [=]() {
		if (expRadSli->value() == 1)
			expRadLab->setText("Exp rad random");
		else expRadLab->setText("Exp rad " + QString::number(expRadSli->value()));
		paintArea->setExpRadius(expRadSli->value());
		});
	connect(windSli, QOverload<int>::of(&QSlider::valueChanged), this, [=]() {
		windLab->setText(windIs(windSli->value()));
		paintArea->setWind(windSli->value());
		});

}

const QString MainWindow::windIs(const int w)
{
	if (w == 0) return QString("no wind");
	if (w < 0) return QString("Left wind: " + QString::number(w * -1));
	return QString("Right wind: " + QString::number(w));
}
