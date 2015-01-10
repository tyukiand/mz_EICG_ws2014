#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include "Bezier.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

#ifdef max
#undef max
#endif

/// Testfunktion, falls OpenGL nicht richtig mitspielt. Einfach am Ende jeder 
/// Funktion einfügen, als Argument den Namen der Funktion in Anführungszeichen.
void glPrintErrors(const char* as_Function)
{
  GLenum lr_Error = GL_NO_ERROR;
  bool lb_ErrorFound = false;
  int li_ErrorCount = 5;
  do
    {
      lr_Error = glGetError();
      if (lr_Error != GL_NO_ERROR)
        {
          lb_ErrorFound = true;
          li_ErrorCount--;
          char* ls_ErrorString = (char*)gluErrorString(lr_Error);
          std::cerr << "OpenGL Error (" << as_Function << "): " << ls_ErrorString << std::endl;
        }
      if (li_ErrorCount == 0)
        {
          std::cerr << "OPENGL :: ERROR Too many errors!" << std::endl;
          break;
        }
      } while (lr_Error != GL_NO_ERROR);
    if (lb_ErrorFound) exit(0);
}


CGMainWindow::CGMainWindow (QWidget* parent, Qt::WindowFlags flags)
: QMainWindow (parent, flags) {
	resize (604, 614);

	// Create a nice frame to put around the OpenGL widget
	QFrame* f = new QFrame (this);
	f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
	f->setLineWidth(2);

	// Create our OpenGL widget
	ogl = new CGView (this,f);

	QMenu *options = new QMenu("&Options",this);
	QAction* action;
	action = options->addAction ("Show Bezier", ogl, SLOT(toggleBezier()), Qt::CTRL+Qt::Key_B);
	action->setCheckable (true);
	action->setChecked (ogl->show_bezier);

	menuBar()->addMenu(options);

	// Put the GL widget inside the frame
	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(ogl);
	layout->setMargin(0);
	f->setLayout(layout);

	setCentralWidget(f);

	statusBar()->showMessage("Ready",1000);
}

CGMainWindow::~CGMainWindow () { }

void CGMainWindow::keyPressEvent(QKeyEvent* event) {
	switch(event->key()) {
	case Qt::Key_I: std::cout << "I" << std::flush; break;
	case Qt::Key_M: std::cout << "M" << std::flush; break;
	}

	ogl->updateGL();
}

CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
	main = mainwindow;

	centerX = centerY = 0;

	show_bezier = false;

	control_points.push_back (Vector3d(-.7,.7,0));
	control_points.push_back (Vector3d(-.7,-.7,0));
	control_points.push_back (Vector3d(.7,.7,0));
	control_points.push_back (Vector3d(.7,-.7,0));
}

void CGView::initializeGL() 
{
	qglClearColor(Qt::white);
	zoom = 1.0;
}

/// Aufgabe 1, (a)
void CGView::de_casteljau(double t0, const std::vector<Vector3d> &l)
{
	/// ADD YOUR CODE
}

/// Aufgabe 1, (b)
void CGView::draw_bezier_casteljau (const std::vector<Vector3d> &l)
{
	/// ADD YOUR CODE
}


void CGView::paintGL() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScaled(zoom,zoom,1.0);
	glTranslated(-centerX,-centerY,0.0);

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3d (0,0,1);
	glPointSize (1.0);
	glBegin(GL_LINE_STRIP);
	for(int i=0;i<(int) control_points.size();i++)
				glVertex2dv(control_points[i].ptr());
	glEnd();

	glColor3d (1,0,0);
	glPointSize (3.0);
	glBegin(GL_POINTS);
	for(int i=0;i<(int) control_points.size();i++)
				glVertex2dv(control_points[i].ptr());
	glEnd();

	if (show_bezier)
		{
			glColor3d (0,0,0);
			glPointSize (2.0);
			draw_bezier_casteljau (control_points);
		}
}

void CGView::resizeGL(int width, int height) {
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (width > height) {
		double ratio = width/(double) height;
		gluOrtho2D(-ratio,ratio,-1.0,1.0);
	}
	else {
		double ratio = height/(double) width;
		gluOrtho2D(-1.0,1.0,-ratio,ratio);
	}
	glMatrixMode (GL_MODELVIEW);
}

void CGView::worldCoord(int x, int y, double &dx, double &dy) {
	if (width() > height()) {
		dx = (2.0*x-width())/height();
		dy = 1.0-2.0*y/(double) height();
	} else {
		dx = 2.0*x/(double) width()-1.0;
		dy = (height()-2.0*y)/width();
	}
	dx /= zoom;
	dy /= zoom;
}

void CGView::mousePressEvent(QMouseEvent *event) {
	double dx, dy;
	worldCoord(event->x(),event->y(),dx,dy);
	std::cout << "Mouse pressed at (" << dx << "," << dy <<")" << std::endl;
}

void CGView::mouseReleaseEvent (QMouseEvent* event) {
	std::cout << "Mouse released" << std::endl;
}

void CGView::wheelEvent(QWheelEvent* event) {
	if (event->delta() < 0) zoom *= 1.1; else zoom *= 1/1.1;
	update();
}

void CGView::mouseMoveEvent (QMouseEvent* event) {
	std::cout << "Mouse moved" << std::endl;

	int x = event->x();
	int y = event->y();
	if (event->button() == Qt::LeftButton)
		updateGL();
}

int main (int argc, char **argv) {
	QApplication app(argc, argv);

	if (!QGLFormat::hasOpenGL()) {
		qWarning ("This system has no OpenGL support. Exiting.");
		return 1;
	}

	CGMainWindow *w = new CGMainWindow(NULL);

	w->show();

	return app.exec();
}

