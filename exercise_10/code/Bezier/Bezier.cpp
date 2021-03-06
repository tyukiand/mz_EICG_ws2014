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

    QAction* actionOwn;
    actionOwn = options->addAction ("Show Own Bezier", ogl, SLOT(ownBezier()), Qt::CTRL+Qt::Key_N);
    actionOwn->setCheckable (true);


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
    case Qt::Key_Minus: ogl->scale(0.9);
    case Qt::Key_Plus: ogl->scale(1.025);
	}

	ogl->updateGL();
}

void CGView::scale(double scale){
    if (select_point>=0)
        control_points[select_point][2]*=scale;
}

CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
	main = mainwindow;

    q_old.set(0.0, 0.0, 0.0, 1.0);
    q_now.set(0.0, 0.0, 0.0, 1.0);
    zoom = 0.5;
    center[0]=0.0;
    center[1]=0.0;
    center[2]=0.0;
    select_point=-1;

    control_points.push_back (Vector3d(1,0,2));
    control_points.push_back (Vector3d(0,-0.5,2));
    control_points.push_back (Vector3d(-0.5,1,2));
}

void CGView::initializeGL() 
{
	qglClearColor(Qt::white);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CGView::gl_bezier3d(const std::vector<Vector3d> &l)
{
    int i,j;

    GLfloat tempoints[l.size()][3];

    for(i=0;i<l.size();++i)
        for(j=0;j<3;++j)
            tempoints[i][j]=l[i][j];

    glEnable(GL_MAP1_VERTEX_3);
    glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, l.size(), &tempoints[0][0]);

    glBegin(GL_LINE_STRIP);
    for (i = 0; i <= 42*zoom; i++)
        glEvalCoord1f((GLfloat) i/(42.0*zoom));
    glEnd();
    glFlush();
    glDisable(GL_MAP1_VERTEX_3);
}

void CGView::gl_proj4d(const std::vector<Vector4d> &l)
{
    int i,j;

    GLfloat tempoints[l.size()][4];

    for(i=0;i<l.size();++i){
        tempoints[i][0]=l[i].x();
        tempoints[i][1]=l[i].y();
        tempoints[i][2]=l[i].z();
        tempoints[i][3]=l[i].w();
    }
    glEnable(GL_MAP1_VERTEX_4);
    glMap1f(GL_MAP1_VERTEX_4, 0.0, 1.0, 4, l.size(), &tempoints[0][0]);

    glBegin(GL_LINE_STRIP);
    for (i = 0; i <= 42*zoom; i++)
        glEvalCoord1f((GLfloat) i/(42.0*zoom));
    glEnd();
    glFlush();
    glDisable(GL_MAP1_VERTEX_4);
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

    glColor3d (0.25,0.5,0.25);
	glPointSize (1.0);
	glBegin(GL_LINE_STRIP);
	for(int i=0;i<(int) control_points.size();i++)
                glVertex3dv(control_points[i].ptr());
	glEnd();

    glColor3d (0.25,0.25,0.25);
    glBegin(GL_LINE_STRIP);
    double x,y,w;
    std::vector<Vector4d> proj_points=std::vector<Vector4d>();
    for(int i=0;i<(int) control_points.size();i++){
        x=control_points[i].x();
        y=control_points[i].y();
        w=control_points[i].z();
        glVertex3d(x,y,1);
        proj_points.push_back (Vector4d(x,y,w,w));
        std::cout << x <<","<<y<<","<<w<<std::endl;
    }
    glEnd();

    glLineWidth(0.5);
    glColor3d (0.,0.,0.);
    for(int i=0;i<(int) control_points.size();i++){
        glBegin(GL_LINE_STRIP);
        glVertex3dv(control_points[i].ptr());
        glVertex3d(0,0,0);
        glEnd();
    }




    glColor3d (0,1,1);
    gl_bezier3d(control_points);

    glColor3d (0,1,1);
    gl_proj4d(proj_points);


    glColor3d (1,0,0);
    glPointSize (5.0);
    glBegin(GL_POINTS);
    for(int i=0;i<(int) control_points.size();i++)
        if (i!=select_point)
            glVertex3dv(control_points[i].ptr());
    glEnd();

    if (select_point>=0){
        glColor3d (1,0.5,0.5);
        glPointSize (7.0);
        glBegin(GL_POINTS);
        glVertex3dv(control_points[select_point].ptr());
        glEnd();
    }


    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);

    glColor4f(105/255.0,105/255.0,105/255.0,0.95);

    glVertex3d( 1, 1,1);
    glVertex3d(-1, 1,1);
    glVertex3d(-1,-1,1);
    glVertex3d( 1,-1,1);

    glVertex3d( 1, 1,1);
    glVertex3d( 1,-1,1);
    glVertex3d(-1,-1,1);
    glVertex3d(-1, 1,1);

    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_QUADS);

    glColor4f(0,0,0,1);

    glVertex3d( 1, 1,0);
    glVertex3d(-1, 1,0);
    glVertex3d(-1,-1,0);
    glVertex3d( 1,-1,0);

    glVertex3d( 1, 1,0);
    glVertex3d( 1,-1,0);
    glVertex3d(-1,-1,0);
    glVertex3d(-1, 1,0);

    glEnd();

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
        gluPerspective(45,ratio,1.0,10000.0);
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
	double dx, dy;

    oldX = event->x();
    oldY = event->y();
    q_old = q_now;
    if (event->button() == Qt::RightButton){
        rotate=1;
    }
    if (event->button() == Qt::LeftButton){
        double epsPow2 = 0.001;
        int alternativeSelectedVertex = findCloseVert(oldX,oldY,epsPow2,control_points);
        std::cout << "Selected Vertex: " << alternativeSelectedVertex << std::endl;
        if (alternativeSelectedVertex >= 0) {
          select_point = alternativeSelectedVertex;
          update();
        }
    }
}

void CGView::mouseReleaseEvent (QMouseEvent* event) {
	std::cout << "Mouse released" << std::endl;
    pointMoving=-1;
    rotate=0;

}

void CGView::wheelEvent(QWheelEvent* event) {
	if (event->delta() < 0) zoom *= 1.1; else zoom *= 1/1.1;
	update();
}

void CGView::mouseMoveEvent (QMouseEvent* event) {

    int x = event->x();
    int y = event->y();
    Vector3d u;
    worldCoord(x,y,0.0,u);

    if (pointMoving<0){
        if(rotate){
            Vector3d u;
            Vector3d v;
            mouseToTrackball(oldX, oldY, currentWidth, currentHeight, u);
            mouseToTrackball(event->x(), event->y(), currentWidth, currentHeight, v);
            Quat4d q;
            trackball(u, v, q);
            if (q.lengthSquared() > 0) {
              q_now = q * q_old;
            }
            updateGL();
        } else{        
            centerX=-u.x()+clickedX;
            centerY=-u.y()+clickedY;
            update();
        }
    } else {
        control_points[pointMoving][0]=u.x();
        control_points[pointMoving][1]=u.y();
        update();
    }



	if (event->button() == Qt::LeftButton)
        update();

}

int CGView::findCloseVert(int x, int y, double epsPow2, std::vector<Vector3d> coord)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    Vector3d mouseObjectCoords;
    worldCoord(x, y, 0, mouseObjectCoords);

    Vector3d someOtherPointUnderTheMouse;
    worldCoord(x, y, -420, someOtherPointUnderTheMouse);


    Vector3d direction = mouseObjectCoords - someOtherPointUnderTheMouse;
    direction.normalize();
    Vector3d projection;
    Vector3d difference;


    int i;
    double relativeDist;
    double minRelativeDist = + 1.0 / 0.0;
    int bestIndex = -1;
    for (i = 0; i < coord.size(); i++) {
        relativeDist = direction.dot(coord[i] - mouseObjectCoords);
        projection = mouseObjectCoords + direction * relativeDist;
        difference = coord[i] - projection;
        if (difference.lengthSquared() <= epsPow2) {
            if (relativeDist < minRelativeDist) {
                minRelativeDist = relativeDist;
                bestIndex = i;
            }
        }
    }
    return bestIndex;
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

