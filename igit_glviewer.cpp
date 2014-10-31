#include"igit_glviewer.h"
#include<qvector.h>
#include<qvector3d.h>
#include<qvector2d.h>

#include<QKeyEvent>
#include<qdebug.h>
#include<manipulatedFrame.h>
#include <qframe.h>

#include<GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>


#define BUFFER_OFFSET(offset) ((GLubyte*) NULL + offset)
#define NumberOf(array)        (sizeof(array)/sizeof(array[0]))

GLuint arrayIndex;
GLsizei NumElements;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GLViewer::GLViewer(QWidget *parent, const QGLWidget* constWidget, Qt::WindowFlags) :QGLViewer(parent, constWidget)
{
	g_display_world_axis_ = true;

	g_display_vertices_ = false;
	g_display_wire_frame_ = false;
	g_display_flat_ = false;
	g_display_texture_ = false;
	g_set_key_frame_ = false;
	g_play_path_ = false;

	g_texture_id_ = -1;

	//------------ set the interpolation  ------------------------------//
	// myFrame is the Frame that will be interpolated.
	qglviewer::Frame* myFrame = new qglviewer::Frame();

	// Set myFrame as the KeyFrameInterpolator interpolated Frame.
	g_kfi_.setFrame(myFrame);
	g_kfi_.setLoopInterpolation();

	connect(&g_kfi_, SIGNAL(interpolated()), SLOT(updateGL()));
	connect(&g_kfi_, SIGNAL(interpolated()), SLOT(setCamera()));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GLViewer::~GLViewer()
{}

/////////////////////////////////////////////////init/////////////////////////////////////////////////////////////////////////
void GLViewer::init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDisable(GL_DITHER);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// BLEND
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}
////////////////////////////////////////////////viewAll///////////////////////////////////////////////////////////////////////////
void GLViewer::viewAll()
{
	computeSceneBoundingBox();
	setSceneBoundingBox(qglviewer::Vec(g_min_x_, g_min_y_, g_min_z_), qglviewer::Vec(g_max_x_, g_max_y_, g_max_z_));
}

/////////////////////////////////////////////////draw///////////////////////////////////////////////////////////////////
void GLViewer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);

	// draw world axis
	if (g_display_world_axis_ == true){
		//drawWorldAxis(1.0, 1.0);
	}

	//---------------------------------------mesh related---------------------------------------------------//
	// display mesh vertices
	if (g_display_vertices_ == true){
		//drawVertices();

		glBindVertexArray(arrayIndex);

		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glDrawElements(GL_TRIANGLES, NumElements, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	}

	// display mesh in wire frame mode 
	if (g_display_wire_frame_ == true){
		//drawWireFrame();

		glBindVertexArray(arrayIndex);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, NumElements, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	}

	// display mesh in Flat Mode
	if (g_display_flat_ == true){
		//drawFlat();

		glBindVertexArray(arrayIndex);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, NumElements, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
	}

	// draw mesh with texture
	if (g_display_texture_ == true){
		//drawTexture();
		glEnable(GL_TEXTURE_2D); // 加这个，命令就可以了
		glBindVertexArray(arrayIndex);
		glBindTexture(GL_TEXTURE_2D, g_texture_id_);
		glDisableClientState(GL_COLOR_ARRAY);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, NumElements, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
		
		glEnableClientState(GL_COLOR_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}
	glFlush();
}
/////////////////////////////////////////////////mousePressEvent//////////////////////////////////////////////////////////////
void GLViewer::mousePressEvent(QMouseEvent * e)
{

	QGLViewer::mousePressEvent(e);
}
////////////////////////////////////////////////mouseReleaseEvent//////////////////////////////////////////////////////////////////
void GLViewer::mouseReleaseEvent(QMouseEvent *e)
{
	QGLViewer::mouseReleaseEvent(e);

}
/////////////////////////////////////////////////mouswMoveEvent/////////////////////////////////////////////////////////////////////
void GLViewer::mouseMoveEvent(QMouseEvent *e)
{
	QGLViewer::mouseMoveEvent(e);
}
/////////////////////////////////////////////////wheelEvent/////////////////////////////////////////////////////////////////////
void GLViewer::wheelEvent(QWheelEvent *e)
{
	QGLViewer::wheelEvent(e);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GLViewer::keyPressEvent(QKeyEvent *e)
{
	if ((e->key() == Qt::Key_1) && (e->modifiers() == Qt::CTRL) && g_set_key_frame_ == true)
	{
		qDebug() << "Position: " << camera()->position().x << " " << camera()->position().y << " " << camera()->position().z << endl;
		qDebug() << "Angle: " << camera()->orientation().angle() << " " << endl;
		qDebug() << "axis: " << camera()->orientation().axis().x << " " << camera()->orientation().axis().y << " " << camera()->orientation().axis().z << endl;

		qglviewer::ManipulatedFrame* frame = new qglviewer::ManipulatedFrame();
		frame->setPosition(camera()->position());
		frame->setOrientation(camera()->orientation());
		g_kfi_.addKeyFrame(frame);
		g_counter_++;

		QString text = QString("%1 key frames in total").arg(g_counter_);
		statusBar(text);
		updateGL();
	}
	if ((e->key() == Qt::Key_Plus) && (g_play_path_ == true))
	{
		g_kfi_.setInterpolationSpeed(g_kfi_.interpolationSpeed() + 0.25);
		updateGL();
	}
	if ((e->key() == Qt::Key_Minus) && (g_play_path_ == true))
	{
		g_kfi_.setInterpolationSpeed(g_kfi_.interpolationSpeed() - 0.25);
		updateGL();
	}
	else{
		QGLViewer::keyPressEvent(e);
	}
}
////////////////////////////////////////////////makeTextures///////////////////////////////////////////////////////////////
void GLViewer::makeObjects()
{
	// texture 
	QImage tex;
	tex = convertToGLFormat(*g_texture_image_);
	glGenTextures(1, &g_texture_id_);

	glBindTexture(GL_TEXTURE_2D, g_texture_id_);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glewInit();

	glGenVertexArrays(1, &arrayIndex);

	enum{ Verices, TexCoords, Colors, Elements, NumAttris };
	GLuint buffers[NumAttris];
	

	GLfloat *colors = new GLfloat[g_element_num_[0] * 3];
	for (int i = 0; i < g_element_num_[0]; i++)
	{
		colors[i * 3 + 0] = (GLfloat)0.5;
		colors[i * 3 + 1] = (GLfloat)0.5;
		colors[i * 3 + 2] = (GLfloat)0.5;
	}

	NumElements = (GLsizei)g_element_num_[1] * 3;

	glBindVertexArray(arrayIndex);
	glGenBuffers(NumAttris, buffers);

	//VERTEX
	glBindBuffer(GL_ARRAY_BUFFER, buffers[Verices]);
	glBufferData(GL_ARRAY_BUFFER, g_element_num_[0] * 3 * sizeof(GLfloat), g_vertices_, GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
	glEnableClientState(GL_VERTEX_ARRAY);

	// COLOR
	glBindBuffer(GL_ARRAY_BUFFER, buffers[Colors]);
	glBufferData(GL_ARRAY_BUFFER, g_element_num_[0] * 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);
	glColorPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
	glEnableClientState(GL_COLOR_ARRAY);

	// TEXTURE COORDS
	for (int i = 0; i < g_element_num_[0]; i++)
	{
		g_texture_coords_[i * 2 + 1] = 1 - g_texture_coords_[i * 2 + 1];
	}
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TexCoords]);
	glBufferData(GL_ARRAY_BUFFER, g_element_num_[0] * 2 * sizeof(GLfloat), g_texture_coords_, GL_STATIC_DRAW);
	glTexCoordPointer(2, GL_FLOAT, 0, BUFFER_OFFSET(0));
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//Face ELEMENT
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[Elements]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_element_num_[1] * 3 * sizeof(GLuint),g_facets_, GL_STATIC_DRAW);
}
/////////////////////////////////////////////////helpString/////////////////////////////////////////////////////////////////
QString GLViewer::helpString()
{
	QString text("<h2> MeshLive 1.0 [2006.10.18.1]<p></h2>");
	text += "An easy and extensible mesh interaction C++ program for real-time applications..<p> ";
	text += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Designed by hywu, jpan, xlvector. Since 2006.<p> ";
	text += "Based on:<p> ";
	text += "QT (http://www.trolltech.com/)<p> ";
	text += "libQGLViewer (http://artis.imag.fr/~Gilles.Debunne/QGLViewer/)<p> ";
	text += "CGAL (http://www.cgal.org/, http://www-sop.inria.fr/geometrica/team/Pierre.Alliez/)<p> ";
	text += "OpenMesh (http://www.openmesh.org/)<p> ";
	text += "Boost (http://www.boost.org/)<p> ";
	text += "OpenCV (http://sourceforge.net/projects/opencvlibrary/)<p> ";
	text += "Python (http://www.python.org/)<p> ";
	text += "etc.<p> ";

	return text;

}
/////////////////////////////////////////////////drawWorldAxis////////////////////////////////////////////////////////////////////
void GLViewer::drawWorldAxis(double width, double length)
{
	double axisLength = length;

	//drawAxis(length);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(width);

	//axis X
	glBegin(GL_LINES);
	{
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(axisLength, 0.0, 0.0);
	}
	glEnd();

	//axis Y
	glBegin(GL_LINES);
	{
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, axisLength, 0.0);
	}
	glEnd();

	// axis Z
	glBegin(GL_LINES);
	{
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, axisLength);
	}
	glEnd();

	glLineWidth(1.0);;
	glDisable(GL_LINE_SMOOTH);

	glColor3f(1.0, 0.0, 0.0);
	renderText(axisLength, 0.0, 0.0, "X", QFont("helvetica", 12, QFont::Bold, TRUE));

	glColor3f(0.0, 1.0, 0.0);
	renderText(0.0, axisLength, 0.0, "Y", QFont("helvetica", 12, QFont::Bold, TRUE));

	glColor3f(0.0, 0.0, 1.0);
	renderText(0.0, 0.0, axisLength, "Z", QFont("helvetica", 12, QFont::Bold, TRUE));

}

////////////////////////////////////////////////computeSceneBoundingBox////////////////////////////////////////////////////////////
void GLViewer::computeSceneBoundingBox()
{
	g_min_x_ = g_min_y_ = g_min_z_ = 1000000;
	g_max_x_ = g_max_y_ = g_max_z_ = -1000000;

	for (int i = 0; i< g_element_num_[0]; i++)
	{
		if (g_min_x_ > g_vertices_[i * 3 + 0]) g_min_x_ = g_vertices_[i * 3 + 0];
		if (g_min_y_ > g_vertices_[i * 3 + 1]) g_min_y_ = g_vertices_[i * 3 + 1];
		if (g_min_z_ > g_vertices_[i * 3 + 2]) g_min_z_ = g_vertices_[i * 3 + 2];

		if (g_max_x_ <  g_vertices_[i * 3 + 0]) g_max_x_ = g_vertices_[i * 3 + 0];
		if (g_max_y_ <  g_vertices_[i * 3 + 1]) g_max_y_ = g_vertices_[i * 3 + 1];
		if (g_max_z_ <  g_vertices_[i * 3 + 2]) g_max_z_ = g_vertices_[i * 3 + 2];

	}
}