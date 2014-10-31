#ifndef IGIT_GLVIEWER_H
#define IGIT_GLVIEWER_H

#include "qglviewer.h"

#include <QObject>
#include<qdir.h>
#include<QVector3D>
#include<QVector2D>
#include<QVector>


class GLViewer : public QGLViewer
{
	Q_OBJECT

public:
	GLViewer(QWidget *parent = 0, const QGLWidget* constWidget = 0, Qt::WindowFlags f = 0);
	~GLViewer();

	virtual void init();
	virtual void viewAll();
	virtual void draw();

	virtual void mousePressEvent(QMouseEvent * e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void wheelEvent(QWheelEvent *e);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual QString helpString();

	// load texture images
	void makeObjects();

	//----------------------------shared pointers ---------------------------------------//
	// mesh vertices
	void setMeshVeticesPtr(GLfloat* ptr){
		g_vertices_ = ptr;
	}
	// mesh facets
	void setMeshFacetPtr(GLuint * ptr){
		g_facets_ = ptr;
	}

	// normals
	void setVerticesNormal(GLfloat *ptr)
	{
		g_normals_ = ptr;
	}

	// set texture coordinates ptr
	void setTextureCoordatesPtr( GLfloat* ptr){ g_texture_coords_ = ptr; }

	// texture image
	void setTextureImagePtr(QImage * ptr){ g_texture_image_ = ptr; }

	// number of element
	void setElementNumPtr(GLuint * ptr) { g_element_num_ = ptr; }
	
protected:

	void drawWorldAxis(double width, double length);

	// bounding box
	void computeSceneBoundingBox();

protected slots:

	inline void toggle_display_vertices(bool flag)
	{
		viewAll();
		showEntireScene();

		g_display_vertices_ = flag;

		QString outputText = QString("%1 vertices").arg(g_element_num_[0]);
		emit statusBar(outputText);

		if (flag == true)
		{
			g_display_wire_frame_ = false;
			g_display_flat_ = false;
			g_display_texture_ = false;
		}
		updateGL();
	}
	inline void toggle_display_wire_frame(bool flag){

		viewAll();
		showEntireScene();
		g_display_wire_frame_ = flag;

		QString outputText = QString("%1 vertices %1 facets").arg(g_element_num_[0]).arg(g_element_num_[1]);
		emit statusBar(outputText);

		if (flag == true)
		{
			g_display_vertices_ = false;
			g_display_flat_ = false;
			g_display_texture_ = false;
		}
		updateGL();
	}
	inline void toggle_display_flat(bool flag)
	{
		viewAll();
		showEntireScene();
		g_display_flat_ = flag;

		QString outputText = QString("%1 vertices %1 facets").arg(g_element_num_[0]).arg(g_element_num_[1]);
		emit statusBar(outputText);

		if (flag == true)
		{
			g_display_vertices_ = false;
			g_display_wire_frame_ = false;
			g_display_texture_ = false;
		}
		updateGL();
	}
	inline void toggle_display_texture(bool flag)
	{
		viewAll();
		showEntireScene();
		g_display_texture_ = flag;

		QString outputText = QString("%1 vertices %1 facets").arg(g_element_num_[0]).arg(g_element_num_[1]);
		emit statusBar(outputText);

		if (flag == true)
		{
			if (g_texture_id_ == -1)
			{
				makeObjects();
			}
			g_display_vertices_ = false;
			g_display_wire_frame_ = false;
			g_display_flat_ = false;
		}
			updateGL();
		}
	// catch several posion and orientarions of cameras to interplatet them into a path
	void toggle_setKeyFrame(bool flag)
		{
			g_set_key_frame_ = flag;

			if (flag == true)
			{
				g_kfi_.deletePath();
				g_counter_ = 0;
				statusBar("  Get key frame to interpolator: adjust the camera and press CTRL + S to save it");
			}
			updateGL();
		}
	// auto play the camera along the path interplated
	void toggle_play_path(bool flag)
		{
			g_play_path_ = flag;

			if (flag == true)
			{
				if (g_counter_ == 0)
				{
					statusBar("Add Key Frame first!");
				}
				else{
					g_kfi_.startInterpolation();
					statusBar(" Play Path");
				}
			}
			if (flag == false)
			{
				g_kfi_.stopInterpolation();
			}
			updateGL();
		}
	
	// set the position and orientation of camera, needed by play path
	void setCamera()
	{
		camera()->setPosition(g_kfi_.frame()->position());
		camera()->setOrientation(g_kfi_.frame()->orientation());
		updateGL();
	}
signals:
		// transmmit message to mainwindow
		void statusBar(QString info);
private:
		bool g_display_world_axis_;
		bool g_display_vertices_;
		bool g_display_wire_frame_;
		bool g_display_flat_;
		bool g_display_texture_;

		bool g_set_key_frame_;
		bool g_play_path_;


		float g_min_x_;
		float g_min_y_;
		float g_min_z_;
		float g_max_x_;
		float g_max_y_;
		float g_max_z_;

		qglviewer::KeyFrameInterpolator g_kfi_;
		int g_counter_;

		GLfloat *g_vertices_;
		GLfloat *g_normals_;
		GLuint *g_facets_;
		GLfloat *g_texture_coords_;
		GLuint * g_element_num_;

		QImage *g_texture_image_;
		GLuint g_texture_id_;
	};

#endif

