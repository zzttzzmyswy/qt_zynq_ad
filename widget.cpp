#include "widget.h"
#include "ui_widget.h"

#define AD_DEV_FILE "/dev/ad_zzt"
#define PAGE_SIZE (1024 * 1024 * 2)
#define AD_DEV_MAP_OFFSET 64

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <poll.h>

struct pollfd fds[1];
}

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
	ui->setupUi(this);
	ui->gridLayout->addWidget(&customPlot);
	customPlot.setMinimumWidth(1000);
	customPlot.addGraph();
	customPlot.plotLayout()->insertRow(0);
	customPlotTitle = new QCPTextElement(&customPlot, tr("AD Read Data"));
	customPlot.plotLayout()->addElement(0, 0, customPlotTitle);
	customPlot.graph(0)->setLineStyle(QCPGraph::lsLine);

	afile = new QFile(AD_DEV_FILE);
	afile->open(QIODevice::WriteOnly | QIODevice::Text);
	afile->write(QString("1").toUtf8());
	/* uchar *map(qint64 offset, qint64 size, MemoryMapFlags flags = NoOptions); */
	//buf = afile->map(0, 20);
	afile->close();
	fd = open(AD_DEV_FILE, O_RDWR);
	buf = (uchar *)mmap(0, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
	qDebug() << QString::asprintf("buf:%x", buf);

	fds[0].fd = fd; //指定文件描述
	fds[0].events = POLLIN; //有数据可以读的时候返回

	timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->start(40);
	connect(timer, SIGNAL(timeout()), this, SLOT(timer_onTimeout()));
}

Widget::~Widget()
{
	afile->open(QIODevice::WriteOnly | QIODevice::Text);
	afile->write(QString("0").toUtf8());
	afile->close();
	munmap(buf, PAGE_SIZE);
	delete ui;
}

void Widget::timer_onTimeout()
{
	QVector<double> vecX(1000, 0);
	QVector<double> vecY(1000, 0); // init Y data with 0;
	int ret = 0;
	/* wait ad data flash flag */
	ret = poll(fds, 1, 5000);
	if (!ret) {
		qDebug() << "wait ad data flash flag timeout \n";
	} else {
		for (int i = 0; i < vecX.size(); ++i) {
			vecX[i] = i;
			vecY[i] = (((buf[AD_DEV_MAP_OFFSET + i * 1000]) * 1.0 /
				    255.0) *
				   10.0) -
				  5;
		}
		customPlotTitle->setText(
			QString("AD Read Data mean value:%1")
				.arg((std::accumulate(std::begin(vecY),
						      std::end(vecY), 0.0) /
				      1000)));
		/* 建表 */
		customPlot.xAxis->setLabel(tr("point index"));
		customPlot.yAxis->setLabel(tr("AD Value Data"));
		customPlot.xAxis->setRange(-1, 1000);
		customPlot.yAxis->setRange(-5, 5);
		customPlot.graph(0)->setData(vecX, vecY);
		customPlot.replot();
	}
	timer->start(40);
}
