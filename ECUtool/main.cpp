#include <QApplication>
#include "ui/mainwindow.hpp"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyle("Fusion");

	MainWindow w;
	w.show();

	return app.exec();
}

