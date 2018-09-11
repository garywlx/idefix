#ifndef IDEFIX_QPLOT_H
#define IDEFIX_QPLOT_H

#include <QtWidgets/QMainWindow>
#include "qcustomplot.h"

namespace IDEFIX {
class QPlot: public QMainWindow {
	Q_OBJECT 

public:
	explicit QPlot(QWidget* parent = 0);
	~QPlot();

	void init();
	
private:
	QCustomPlot* m_plot;
};	
};

#endif