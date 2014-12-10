#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>

#define _USE_MATH_DEFINES
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

#include "Perspective.h"

/*
 * Draws a wireframed cube. Code contributed by Andreas Umbach <marvin@dataway.ch>
 */
void glutWireCube( GLdouble dSize )
{
    double size = dSize * 0.5;

    //FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutWireCube" );

#   define V(a,b,c) glVertex3d( a size, b size, c size );
#   define N(a,b,c) glNormal3d( a, b, c );

    /* PWO: I dared to convert the code to use macros... */
    glBegin( GL_LINE_LOOP ); N( 1.0, 0.0, 0.0); V(+,-,+); V(+,-,-); V(+,+,-); V(+,+,+); glEnd();
    glBegin( GL_LINE_LOOP ); N( 0.0, 1.0, 0.0); V(+,+,+); V(+,+,-); V(-,+,-); V(-,+,+); glEnd();
    glBegin( GL_LINE_LOOP ); N( 0.0, 0.0, 1.0); V(+,+,+); V(-,+,+); V(-,-,+); V(+,-,+); glEnd();
    glBegin( GL_LINE_LOOP ); N(-1.0, 0.0, 0.0); V(-,-,+); V(-,+,+); V(-,+,-); V(-,-,-); glEnd();
    glBegin( GL_LINE_LOOP ); N( 0.0,-1.0, 0.0); V(-,-,+); V(-,-,-); V(+,-,-); V(+,-,+); glEnd();
    glBegin( GL_LINE_LOOP ); N( 0.0, 0.0,-1.0); V(-,-,-); V(-,+,-); V(+,+,-); V(+,-,-); glEnd();

#   undef V
#   undef N
}

/*
 * Draws a solid cube. Code contributed by Andreas Umbach <marvin@dataway.ch>
 */
void glutSolidCube( GLdouble dSize )
{
    double size = dSize * 0.5;

#   define V(a,b,c) glVertex3d( a size, b size, c size );
#   define N(a,b,c) glNormal3d( a, b, c );

    /* PWO: Again, I dared to convert the code to use macros... */
    glBegin( GL_QUADS );
        N( 1.0, 0.0, 0.0); V(+,-,+); V(+,-,-); V(+,+,-); V(+,+,+);
        N( 0.0, 1.0, 0.0); V(+,+,+); V(+,+,-); V(-,+,-); V(-,+,+);
        N( 0.0, 0.0, 1.0); V(+,+,+); V(-,+,+); V(-,-,+); V(+,-,+);
        N(-1.0, 0.0, 0.0); V(-,-,+); V(-,+,+); V(-,+,-); V(-,-,-);
        N( 0.0,-1.0, 0.0); V(-,-,+); V(-,-,-); V(+,-,-); V(+,-,+);
        N( 0.0, 0.0,-1.0); V(-,-,-); V(-,+,-); V(+,+,-); V(+,-,-);
    glEnd();

#   undef V
#   undef N
}

CGMainWindow::CGMainWindow (QWidget* parent, Qt::WindowFlags flags)
: QMainWindow (parent, flags) {
	resize (604, 614);


	// Create a menu
        QMenu *file = new QMenu("&File",this);
        file->addAction ("Quit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);
	menuBar()->addMenu(file);

	// Create a nice frame to put around the OpenGL widget
	QFrame* f = new QFrame (this);
	f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
	f->setLineWidth(2);


	// Create our OpenGL widget
	ogl = new CGView (this,f);
	// Put the GL widget inside the frame
	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(ogl);
	layout->setMargin(0);
	f->setLayout(layout);

	setCentralWidget(f);

	statusBar()->showMessage("Ready",1000);
}

CGMainWindow::~CGMainWindow () {}


CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
	q_old.set(0.0, 0.0, 0.0, 1.0);
    q_now.set(0.0, 0.0, 0.0, 1.0);
}

void CGView::initializeGL() {
	qglClearColor(Qt::black);
	zoom = 1.0;
	center = 0.0;

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
}

void CGView::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslated(0.0,0.0,-3.0);

	Matrix4d M, MT;
	M.makeRotate(q_now);
	MT = M.transpose();
	glMultMatrixd(MT.ptr());


	glScaled(zoom,zoom,zoom);
	glTranslated(-center[0],-center[1],-center[2]);

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0,1.0,1.0);

	glutSolidCube (1.0);

	glLineWidth (3.0);	
	glColor3d (0,0,1);
	glutWireCube (1.0);
	
}

void CGView::resizeGL(int width, int height) {
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (width > height) {
		double ratio = width/(double) height;
		gluPerspective(45,ratio,1.0,10000.0);
	}
	else {
		double ratio = height/(double) width;
		gluPerspective(45,1.0/ratio,0.01,10000.0);
	}
	currentWidth = width;
	currentHeight = height;
	glMatrixMode (GL_MODELVIEW);
}

void CGView::worldCoord(int x, int y, int z, Vector3d &v) {
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	GLdouble M[16], P[16];
	glGetDoublev(GL_PROJECTION_MATRIX,P);
	glGetDoublev(GL_MODELVIEW_MATRIX,M);
	gluUnProject(x,viewport[3]-1-y,z,M,P,viewport,&v[0],&v[1],&v[2]);
}

void CGView::mousePressEvent(QMouseEvent *event) {
	oldX = event->x();
	oldY = event->y();
    q_old = q_now;
}

void CGView::mouseReleaseEvent(QMouseEvent *event) {
    Vector3d u;
    Vector3d v;
    mouseToTrackball(oldX, oldY, currentWidth, currentHeight, u);
    mouseToTrackball(event->x(), event->y(), currentWidth, currentHeight, v);
    Quat4d q;
    trackball(u, v, q);
    if (q.lengthSquared() > 0) {
      q_now = q * q_old;
    }
    q_old = q_now;
    updateGL();
}

void CGView::wheelEvent(QWheelEvent* event) {
	if (event->delta() < 0) zoom *= 1.2; else zoom *= 1/1.2;
	update();
}

void CGView::mouseMoveEvent(QMouseEvent* event) {
	Vector3d u;
	Vector3d v;
	mouseToTrackball(oldX, oldY, currentWidth, currentHeight, u);
    mouseToTrackball(event->x(), event->y(), currentWidth, currentHeight, v);
    Quat4d q;
    trackball(u, v, q);
    if (q.lengthSquared() > 0) {
      // sometimes mouseMove events occur even when the mouse stays on
      // the same pixel. The length of `q` becomes 0, and everything breaks
      // down. To prevent this, we check that `q` is not 0.
      q_now = q * q_old;
    }
	updateGL();
}

void CGView::mouseToTrackball(int x, int y, int w, int h, Vector3d &v) {
  double r = fmin(w, h) / 2.0;
  double cx = w / 2.0;
  double cy = h / 2.0;
  double dx = (x - cx) / r;
  double dy = -(y - cy) / r;
  double rho = hypot(dx, dy);
  double z = 0;
  if (rho > 1) {
  	 // pull (dx, dy) to the trackball, leave z=0
     dx /= rho;
     dy /= rho;
  } else {
     z = sqrt(1 - rho * rho);
  }
  v.v[0] = dx;
  v.v[1] = dy;
  v.v[2] = z;
}

void CGView::trackball(Vector3d u, Vector3d v, Quat4d &q) {
  q.set(1 + u.dot(v), u % v);
  q.normalize();
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

