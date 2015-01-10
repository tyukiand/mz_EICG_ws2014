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

#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <GL/glut.h>

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
    QAction* actionOpengl;
    actionOpengl = options->addAction ("Show Opengl Bezier", ogl, SLOT(openglBezier()), Qt::CTRL+Qt::Key_B);
    actionOpengl->setCheckable (true);
    actionOpengl->setChecked(ogl->opengl_bezier);

    QAction* actionOwn;
    actionOwn = options->addAction ("Show Own Bezier", ogl, SLOT(ownBezier()), Qt::CTRL+Qt::Key_N);
    actionOwn->setCheckable (true);
    actionOwn->setChecked(ogl->own_bezier);


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
void CGView::opengl_method(const std::vector<Vector3d> &l)
{
    int i,j;

    GLfloat tempoints[l.size()][3];

    for(i=0;i<l.size();++i)
        for(j=0;j<3;++j)
            tempoints[i][j]=l[i][j];

    glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, l.size(), &tempoints[0][0]);
    glEnable(GL_MAP1_VERTEX_3);

    glBegin(GL_LINE_STRIP);
    for (i = 0; i <= 42*zoom; i++)
        glEvalCoord1f((GLfloat) i/(42.0*zoom));
    glEnd();
    glFlush();
}

/// Aufgabe 1, (b)
double CGView::bezpoly(double x, unsigned int i, unsigned int n)
{
    double bino=1;
    for(unsigned int j=1;j<=i;j++){
        bino=bino*((n-i)*1.0/j+1);
    }

    return bino*pow(x,i)*pow(1-x,n-i);
}

Vector3d CGView::de_casteljau(double t0,unsigned int r,unsigned int i, const std::vector<Vector3d> &l)
{
    if (r==0){
        return l[i];
    } else
        return de_casteljau(t0,r-1,i,l)*(1-t0)+de_casteljau(t0,r-1,i+1,l)*t0;

}

Vector3d CGView::de_casteljau(double t0, const std::vector<Vector3d> &l)
{
    return de_casteljau(t0,l.size()-1,0,l);
}

/// Aufgabe 1, (c)
void CGView::draw_bezier_casteljau (const std::vector<Vector3d> &l)
{
    int i;
    double n=65536;

    glBegin(GL_LINE_STRIP);
    //glPointSize (4.0);
    //glBegin(GL_POINTS);
    for(i=0;i<n;i++)
    {
        //std::cout << de_casteljau(i/42.0,l)[0] << std::endl;
        //std::cout << de_casteljau(i/42.0,l)[1] << std::endl;
        //std::cout << de_casteljau(i/42.0,l)[2] << std::endl;
        glVertex3dv(de_casteljau(i/(n-1),l).ptr());
    }
    glEnd();
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

    if (opengl_bezier){
        glColor3d (0,1,1);
        opengl_method(control_points);
    }

    if (own_bezier)
    {
        glColor3d (0,1,0);
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
    clickedX=dx+centerX;
    clickedY=dy+centerY;
    pointMoving=nearPoint(clickedX,clickedY);


    if (pointMoving==-1){
        Vector3d lp;
        Vector3d point=Vector3d(dx,dy,0.0);
        projToLine(point,lp);
        if ((lp-point).length()<(ep/zoom)){
            control_points.push_back(lp);
            std::cout << "Dist(" << (lp-point).length() <<")" << std::endl;
            std::cout << "Added(" << lp[0]<<','<< lp[1] <<")" << std::endl;
        }
    } else {
        clickedX=control_points[pointMoving][0];
        clickedY=control_points[pointMoving][1];
    }

    if (event->button() == Qt::RightButton)
        control_points.erase(control_points.begin()+pointMoving);

    update();


	std::cout << "Mouse pressed at (" << dx << "," << dy <<")" << std::endl;
    std::cout << "Mouse pressed at (" << clickedX << "," << clickedY <<")" << std::endl;
    std::cout << "Picked(" << pointMoving <<")" << std::endl;
}

int CGView::nearPoint(double x, double y)
{
    Vector3d point=Vector3d(x,y,0.0);

    for (int i = 0; i < control_points.size(); ++i) {
        if ((point-(control_points[i])).length()<ep/zoom){
            std::cout << "distance: " << (point-(control_points[i])).length() << std::endl;
            return i;
        }
    }
    return -1;
}

void CGView::projectToLineSegment(Vector3d a,Vector3d b,Vector3d x,Vector3d &p)
{
    // `d` is the direction of the line-segment
    double dx = b[0] - a[0];
    double dy = b[1] - a[1];

    // `r` is the position of `(x,y)` relative to `a`
    double rx = x[0] - a[0];
    double ry = x[1] - a[1];

    double rdDot = rx * dx + ry * dy;
    double dNormSq = dx * dx + dy * dy;

    // `p` is the closest point to `(x,y)` on the line segment
    if (rdDot <= 0 || dNormSq == 0) {
        p[0] = a[0];
        p[1] = a[1];
    } else if (rdDot >= dNormSq) {
        p[0] = b[0];
        p[1] = b[1];
    } else {
        double f = rdDot / dNormSq;
        p[0] = a[0] + f * dx;
        p[1] = a[1] + f * dy;
   }
}

void CGView::projToLine(Vector3d point, Vector3d &p)
{
    std::vector<Vector3d> l=control_points;

    double ml=(point-de_casteljau(0,l)).length();
    p=de_casteljau(0,l);

    Vector3d pt;
    for (int i = 0; i < 42*zoom; ++i) {
        projectToLineSegment(de_casteljau(i/(42*zoom),l),de_casteljau((i+1)/(42*zoom),l),
                             point,pt);
        if ((pt-point).length()<ml){
            p=pt;
            ml=(pt-point).length();
        }
    }
}

void CGView::mouseReleaseEvent (QMouseEvent* event) {
	std::cout << "Mouse released" << std::endl;
    pointMoving=-1;
}

void CGView::wheelEvent(QWheelEvent* event) {
	if (event->delta() < 0) zoom *= 1.1; else zoom *= 1/1.1;
	update();
}

void CGView::mouseMoveEvent (QMouseEvent* event) {

    int x = event->x();
    int y = event->y();
    double dx,dy;
    worldCoord(x,y,dx,dy);

    if (pointMoving<0){
        centerX=-dx+clickedX;
        centerY=-dy+clickedY;
        update();
        //std::cout << "set to: " << centerX << "," << centerY << std::endl;
        //std::cout << "set to: " << clickedX << "," << clickedY << std::endl;
        //std::cout << "set to: " << dx << "," << dy << std::endl;
        //paintGL();
    } else {
        control_points[pointMoving][0]=dx;
        control_points[pointMoving][1]=dy;
        update();
    }


	if (event->button() == Qt::LeftButton)
        update();

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

