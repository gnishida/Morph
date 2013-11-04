#include "Morph.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	Morph w;
	w.show();
	return a.exec();
}
