#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <fstream>
#include<string>

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDataStream>

#include <QProgressDialog>
#include <qactiongroup.h>

using namespace std;

//***************************************************CLASS MEMBER************************************************//
MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// tools
	QIcon icon0;
	icon0.addFile(QStringLiteral("icons/fileOpen.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->actionLoad_Mesh->setIcon(icon0);
	ui->mainToolBar->addAction(ui->actionLoad_Mesh);

	QIcon icon1;
	icon1.addFile(QStringLiteral("icons/fileSave.png"), QSize(), QIcon::Normal, QIcon::Off);
	ui->actionSave_Mesh->setIcon(icon1);
	ui->mainToolBar->addAction(ui->actionSave_Mesh);

	// display action group 
	QActionGroup *displayActions = new QActionGroup(this);
	displayActions->addAction(ui->actionVetices);
	displayActions->addAction(ui->actionWire_Frame);
	displayActions->addAction(ui->actionFlat);
	displayActions->addAction(ui->actionTexure);
	ui->actionWire_Frame->setChecked(true);

	m_element_num_ = new GLuint[2];
	ui->viewer->setElementNumPtr(m_element_num_);

	ui->viewer->setTextureImagePtr(&m_texture_image_);
	

	//----------------------------------------SLOTS AND SIGNALS------------------------------------------------------------//
	//save mesh
	connect(ui->actionSave_Mesh, SIGNAL(triggered()), this, SLOT(saveMesh()));
	//load mesh
	connect(ui->actionLoad_Mesh, SIGNAL(triggered()), SLOT(loadMesh()));

	// display functions
	connect(ui->actionVetices, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_vertices(bool)));
	connect(ui->actionWire_Frame, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_wire_frame(bool)));
	connect(ui->actionFlat, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_flat(bool)));
	//connect(ui->actionFlatLine, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_flat_line(bool)));
	connect(ui->actionTexure, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_display_texture(bool)));
	connect(this, SIGNAL(displayTexture(bool)), ui->viewer, SLOT(toggle_display_texture(bool)));

	// show information from viewer
	connect(ui->viewer, SIGNAL(statusBar(QString)), this, SLOT(viewerMessageToStatusBar(QString)));
	// play path
	connect(ui->actionPath, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_setKeyFrame(bool)));
	connect(ui->actionPlay_Path, SIGNAL(triggered(bool)), ui->viewer, SLOT(toggle_play_path(bool)));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
	delete ui;
}

////////////////////////////viewerMessageToStatusBar////////////////////////////////////////////////////////
void MainWindow::viewerMessageToStatusBar(QString str)
{
	statusBar()->showMessage(str);
	update();
}

////////////////////////////textureMappingToTextEdit////////////////////////////////////////////////////////
void MainWindow::enableActionTexture()
{

	ui->actionTexure->setEnabled(true);

	update();
	ui->viewer->updateGL();
}

//////////////////////////////////Load Mesh////////////////////////////////////////////////////////////////
bool MainWindow::loadMesh()
{
#if 1
	QString file_name = QFileDialog::getOpenFileName(this, tr("Open Mesh File"), ".",
		tr("Mesh files(*.ply)"));

	if (file_name.isEmpty())
	{
		statusBar()->showMessage(tr("Fail to Load Mesh!"));
		return false;
	}
	statusBar()->showMessage(tr("Loading Mesh..."));


	// create a progress dialog
	QProgressDialog progress(this);
	progress.setLabelText(tr("Loading Mesh..."));
	progress.setWindowModality(Qt::WindowModal);

	ifstream fin(file_name.toStdString().c_str(), ios::binary);

	int vertex_num  =0;
	fin.read((char*) & vertex_num, sizeof(vertex_num));
	m_element_num_[0] = (GLuint)vertex_num;

	m_vertices_ = new GLfloat[vertex_num *3];
	m_normals_ = new GLfloat[vertex_num *3];
	m_texture_coords_ = new GLfloat[vertex_num *2];
	ui->viewer->setMeshVeticesPtr(m_vertices_);
	ui->viewer->setVerticesNormal(m_normals_);
	ui->viewer->setTextureCoordatesPtr(m_texture_coords_);
	

	int face_num = 0;
	fin.read((char*) & face_num, sizeof(face_num));
	m_element_num_[1] = (GLuint)face_num;

	m_facets_ = new GLuint[face_num * 3];
	ui->viewer->setMeshFacetPtr(m_facets_);
	


	progress.setRange(0, vertex_num + face_num);

	int counter = 0;
	float vertex[8] = {0};

	// read vetices
	for(int i=0; i< vertex_num; i++)
	{
		fin.read((char*)&vertex, sizeof(vertex));

		// position of vertex
		m_vertices_[i * 3 + 0] = vertex[0];
		m_vertices_[i * 3 + 1] = vertex[1];
		m_vertices_[i * 3 + 2] = vertex[2];

		// normal of vertex
		m_normals_[i * 3 + 0] = vertex[3];
		m_normals_[i * 3 + 1] = vertex[4];
		m_normals_[i * 3 + 2] = vertex[5];

		// texture coordinate of vertex
		m_texture_coords_[i * 2 + 0] = vertex[6];
		m_texture_coords_[i * 2 + 1] = vertex[7];
		
		counter++;
		progress.setValue(counter);
		qApp->processEvents();
		if (progress.wasCanceled())
		{
			delete[] m_vertices_;
			delete[] m_facets_;
			delete[] m_normals_;
			delete[] m_texture_coords_;
			return false;
		}
	}
	// read facets
	int face[4] = {0};
	for(int i=0; i<face_num; i++)
	{
		fin.read((char*)&face, sizeof(face));
        
		m_facets_[i * 3 + 0] = face[1];
		m_facets_[i * 3 + 1] = face[2];
		m_facets_[i * 3 + 2] = face[3];

		counter++;
		progress.setValue(counter);
		qApp->processEvents();
		if (progress.wasCanceled())
		{

			delete[] m_vertices_;
			delete[] m_facets_;
			delete[] m_normals_;
			delete[] m_texture_coords_;
			return false;
		}
	}

	fin.close();

	ui->viewer->viewAll();
	ui->viewer->showEntireScene();

	ui->actionVetices->setEnabled(true);
	ui->actionWire_Frame->setEnabled(true);
	ui->actionFlat->setEnabled(true);
	ui->actionFlatLine->setEnabled(true);

	QString texture_image_dir = file_name.replace("ply", "png");

	if (m_texture_image_.load(texture_image_dir))
	{
		enableActionTexture();
		emit displayTexture(true);
	}
	else
	{
		statusBar()->showMessage(QString("Fail to load ") + texture_image_dir);
	}
	ui->viewer->updateGL();
#endif
#if 0
	QString file_name = QFileDialog::getOpenFileName(this, tr("Open Mesh File"), ".",
		tr("Mesh files(*.ply)"));

	if (file_name.isEmpty())
	{
		statusBar()->showMessage(tr("Fail to Load Mesh!"));
		return false;
	}
	QFile file(file_name);
	if (!file.open(QIODevice::ReadOnly))
	{
		statusBar()->showMessage(tr("Fail to Load Mesh!"));
		return false;
	}
	statusBar()->showMessage(tr("Loading Mesh..."));

	// create a progress dialog
	QProgressDialog progress(this);
	progress.setLabelText(tr("Loading Mesh..."));
	progress.setWindowModality(Qt::WindowModal);

	QTextStream in(&file);
	int facet_num = 0;
	int vertex_num = 0;
	int counter = 0;
	bool start_read = false;
	int inner_counter = 0;
	while (!in.atEnd())
	{
		QString line = in.readLine();
		QStringList fields = line.split(" ");

		if (line.startsWith("ply") || line.startsWith("comment") || line.startsWith("format"))
		{
			continue;
		}

		// nummber of vetices
		if (line.startsWith("element vertex"))
		{
			fields.takeFirst();
			fields.takeFirst();
			vertex_num = fields.takeFirst().toInt();

			m_element_num_[0] = (GLuint)vertex_num;

			m_vertices_ = new GLfloat[vertex_num * 3];
			m_normals_ = new GLfloat[vertex_num * 3];
			m_texture_coords_ = new GLfloat[vertex_num * 2];
		
			ui->viewer->setMeshVeticesPtr(m_vertices_);
			ui->viewer->setVerticesNormal(m_normals_);
			ui->viewer->setTextureCoordatesPtr(m_texture_coords_);

			continue;
		}

		// nummber of facets
		if (line.startsWith("element face"))
		{
			fields.takeFirst();
			fields.takeFirst();
			facet_num = fields.takeFirst().toInt();

			m_element_num_[1] = (GLuint)facet_num;
			m_facets_ = new GLuint[facet_num * 3];
			ui->viewer->setMeshFacetPtr(m_facets_);

			continue;
		}
		// end of header
		if (line.startsWith("end_header"))
		{
			progress.setRange(0, vertex_num + facet_num);
			start_read = true;
			counter = 0;
			continue;
		}

		// read vertices
		if (start_read == true && counter < vertex_num)
		{
			QVector3D pt;

			m_vertices_[counter * 3 + 0] = (GLfloat)fields.takeFirst().toFloat();
			m_vertices_[counter * 3 + 1] = (GLfloat)fields.takeFirst().toFloat();
			m_vertices_[counter * 3 + 2] = (GLfloat)fields.takeFirst().toFloat();
		

			m_normals_[counter * 3 + 0] = (GLfloat)fields.takeFirst().toFloat();
			m_normals_[counter * 3 + 1] = (GLfloat)fields.takeFirst().toFloat();
			m_normals_[counter * 3 + 2] = (GLfloat)fields.takeFirst().toFloat();

		
			m_texture_coords_[counter * 2 + 0] = (GLfloat)fields.takeFirst().toFloat();
			m_texture_coords_[counter * 2 + 1] = (GLfloat)fields.takeFirst().toFloat();
			counter++;
		}
		// read facets
		if (start_read == true && counter >= vertex_num && counter < vertex_num + facet_num + 1)
		{
			if (counter == vertex_num)
			{
				counter++;
				inner_counter = 0;
				continue;
			}

			int num = fields.takeFirst().toInt();
			m_facets_[inner_counter * 3 + 0] = fields.takeFirst().toInt();
			m_facets_[inner_counter * 3 + 1] = fields.takeFirst().toInt();
			m_facets_[inner_counter * 3 + 2] = fields.takeFirst().toInt();
		
			inner_counter++;
			counter++;
		}
		progress.setValue(counter);
		qApp->processEvents();

		if (progress.wasCanceled())
		{
			delete[] m_vertices_;
			delete[] m_facets_;
			delete[] m_normals_;
			delete[] m_texture_coords_;
			return false;
		}
	}

	ui->viewer->viewAll();
	ui->viewer->showEntireScene();

	ui->actionVetices->setEnabled(true);
	ui->actionWire_Frame->setEnabled(true);
	ui->actionFlat->setEnabled(true);
	ui->actionFlatLine->setEnabled(true);
	
	QString texture_image_dir = file_name.replace("ply", "png");
	
	if (m_texture_image_.load(texture_image_dir))
	{
		enableActionTexture();
		emit displayTexture(true);
	}
	else
	{
		statusBar()->showMessage(QString("Fail to load ") + texture_image_dir);
	}
	ui->viewer->updateGL();

	return true;

#endif
}
//////////////////////////////////Save Mesh//////////////////////////////////////////////////////////////////
bool MainWindow::saveMesh()
{

	QString file_name = QFileDialog::getSaveFileName(this, tr("Save Mesh File"), ".", tr("Mesh Files(*.ply)"));

	if (file_name.isEmpty())
	{
		statusBar()->showMessage(tr("Fail to Save Mesh !"));
		return false;
	}

	std::ofstream fout(file_name.toStdString().c_str(), std::ios::binary);
	int num_vertice = m_element_num_[0];
	int num_facet = m_element_num_[1];

	// number of vertex
	fout.write((char*)& num_vertice, sizeof(num_vertice));

	//number of facets
	fout.write((char*) & num_facet, sizeof(num_facet));


	// info of vertex
	float vertex[8] = { 0 };
	for (int i = 0; i< num_vertice; i++)
	{
		vertex[0] = m_vertices_[i * 3 + 0];
		vertex[1] = m_vertices_[i * 3 + 1];
		vertex[2] = m_vertices_[i * 3 + 2];

		vertex[3] = m_normals_[i * 3 + 0];
		vertex[4] = m_normals_[i * 3 + 1];
		vertex[5] = m_normals_[i * 3 + 2];

		vertex[6] = m_texture_coords_[i * 2 + 0];
		vertex[7] = m_texture_coords_[i * 2 + 1];

		fout.write((char*) & vertex, sizeof(vertex));
	}

	// info of facets
	int face[4] = { 0 };
	for (int i = 0; i< num_facet; i++)
	{
		face[0] = 3;
		face[1] = m_facets_[i * 3 + 0];
		face[2] = m_facets_[i * 3 + 1];
		face[3] = m_facets_[i * 3 + 2];

		fout.write((char*)&face, sizeof(face));
	}

	fout.close();

	//--------------------------------------------save texture image---------------------------------------//
	QString texture_dir = file_name.replace("ply", "png");
	m_texture_image_.save(texture_dir);

#if 0
	QFile file(file_name);
	if (!file.open(QIODevice::WriteOnly))
	{
		statusBar()->showMessage(tr("Fail to Save Mesh !"));
		return false;
	}
	QString texture_dir = file_name.replace("ply", "png");

	QDataStream out(&file);
	out.setVersion(QDataStream::Qt_5_3);
	
	// number of vertices and facets
	QString name;
	name = QString(" %1 %2").arg(m_vertices_.size()).arg(m_facets_.size());
	out.writeBytes(name.toStdString().c_str(), name.size());

	// info of vertices
	// x, y, z, nx, ny,nz, tx, ty
	for (int i = 0; i < m_vertices_.size(); i++)
	{
		name = QString("%1 %2 %3").arg(m_vertices_[i].x()).arg(m_vertices_[i].y()).arg(m_vertices_[i].z());
		out.writeBytes(name.toStdString().c_str(), name.size());

		name = QString("%1 %2 %3").arg(m_normals_[i].x()).arg(m_normals_[i].y()).arg(m_normals_[i].z());
		out.writeBytes(name.toStdString().c_str(), name.size());

		name = QString("%1 %2").arg(m_texture_coords_[i].x()).arg(m_texture_coords_[i].y());
		out.writeBytes(name.toStdString().c_str(), name.size());
	}

	// indices of facets
	foreach(QVector<int> facet, m_facets_)
	{
		name = QString("%1 %2 %3 %4").arg(facet.size()).arg(facet[0]).arg(facet[1]).arg(facet[2]);
		out.writeBytes(name.toStdString().c_str(), name.size());
	}
    
	//--------------------------------------------save texture image---------------------------------------//
	m_texture_image_.save(texture_dir);
#endif

	return true;
}

