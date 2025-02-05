#include <QApplication>
#include "ui/mainwindow.hpp"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow w;
	w.show();

	app.setStyle("Fusion");
	return app.exec();
}

