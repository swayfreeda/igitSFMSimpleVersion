#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "igit_glviewer.h"

#include <mainwindow.h>
#include <QMainWindow>
#include <QThread>
#include<QMap>

#include<GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();


public slots:
	
	bool loadMesh();
	bool saveMesh();
	void enableActionTexture();

	void viewerMessageToStatusBar(QString str);
signals:

	void displayTexture(bool);
	void textureImageDir(QString);

private:

private:
	
	Ui::MainWindow *ui;

	// vertices of mesh generated from poisson surface reconstruction
	GLfloat * m_vertices_;

	GLfloat * m_normals_;

	// facets of mesh generated from poisson surface reconstruction
	GLuint * m_facets_;

	//texture coordinate of each vertices
	GLfloat * m_texture_coords_;

	GLuint *m_element_num_;

	QImage m_texture_image_;

};

#endif // MAINWINDOW_H
