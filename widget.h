#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <qcustomplot/qcustomplot.h>
#include <QDebug>
#include <iostream>

typedef QPair<QPointF, QString> Data;
typedef QList<Data> DataList;
typedef QList<DataList> DataTable;

QT_BEGIN_NAMESPACE namespace Ui
{
	class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget {
	Q_OBJECT

    public:
	Widget(QWidget *parent = nullptr);
	~Widget();

    private:
	Ui::Widget *ui;

    private:
	QCustomPlot customPlot;
	QCPTextElement *customPlotTitle;
	QTimer *timer;
	QFile *afile;
	uchar *buf;
	int fd;

    private slots:
	void timer_onTimeout();
};
#endif // WIDGET_H
