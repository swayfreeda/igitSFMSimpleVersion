#include <mainwindow.h>
#include "mainwindow.h"
#include <QApplication>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include<string>

#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <qdatastream.h>

using namespace std;

int main(int argc, char *argv[])
{
#if 1
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(1200,600);
    w.show();
 
    return a.exec();
#endif 

#if 0
	QFile file_out("test.igit");
	if (!file_out.open(QIODevice::WriteOnly)) return - 1;

	QDataStream fout(&file_out);
	fout.setVersion(QDataStream::Qt_5_3);
	
	QString name ;
	name.sprintf("%s", "ply");
	//fout.writeBytes(name.toStdString().c_str(), name.size());
	fout.writeRawData(name.toStdString().c_str(), name.size());

	name.sprintf("%s", "format ascii 1.0");
	//fout.writeBytes(name.toStdString().c_str(), name.size());
	fout.writeRawData(name.toStdString().c_str(), name.size());

	name = QString("element vertex %1").arg(26272);
	//fout.writeBytes(name.toStdString().c_str(), name.size());
	fout.writeRawData(name.toStdString().c_str(), name.size());


#endif

#if 0
	QFile file_in("test.igit");
	if (!file_in.open(QIODevice::ReadOnly)) return -1;
	QDataStream fin(&file_in);
	fin.setVersion(QDataStream::Qt_5_3);

	char *buff;
	uint l;
	fin.readBytes(buff, l);
	QString ss(buff);

	fin.readBytes(buff, l);
	ss.sprintf("%s", buff);

	fin.readBytes(buff, l);
	ss.sprintf("%s", buff);
#endif
	return 0;
}
