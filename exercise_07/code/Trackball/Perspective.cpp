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
#include "OffReader.h"

CGMainWindow::CGMainWindow (QWidget* parent, Qt::WindowFlags flags)
: QMainWindow (parent, flags) {
	resize (604, 614);


	// Create a menu
  QMenu *file = new QMenu("&File",this);
  file->addAction ("Load polyhedron", this, SLOT(loadPolyhedron()), Qt::CTRL+Qt::Key_L);
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

void CGMainWindow::loadPolyhedron() {
  QString filename = QFileDialog::getOpenFileName(this, "Load polyhedron ...", QString(), "OFF files (*.off)" );

  if (filename.isEmpty()) return;
  statusBar()->showMessage ("Loading polyhedron ...");
  std::ifstream file(filename.toLatin1());
  int vn,fn,en;

  ogl->min = +std::numeric_limits<double>::max();
  ogl->max = -std::numeric_limits<double>::max();

  std::string s;
  file >> s;

  file >> vn >> fn >> en;
  std::cout << "number of vertices : " << vn << std::endl;
  std::cout << "number of faces    : " << fn << std::endl;
  std::cout << "number of edges    : " << en << std::endl;

  ogl->coord.resize(vn);
  
  for(int i=0;i<vn;i++) {
    file >> ogl->coord[i][0] >> ogl->coord[i][1] >> ogl->coord[i][2];

    for(int j=0;j<3;++j) {
      if (ogl->coord[i][j] < ogl->min[j]) ogl->min[j] = ogl->coord[i][j];
      if (ogl->coord[i][j] > ogl->max[j]) ogl->max[j] = ogl->coord[i][j];
    }

  }

  ogl->ifs.resize(fn);

  for(int i=0;i<fn;i++) {
    int k;
    file >> k;
    ogl->ifs[i].resize(k);
    for(int j=0;j<k;j++) file >> ogl->ifs[i][j];
  }

  file.close();

  std::cout << "min = " << ogl->min << std::endl;
  std::cout << "max = " << ogl->max << std::endl;

  Vector3d extent = ogl->max - ogl->min;
  ogl->zoom = 1.5/extent.maxComp();

  ogl->center = (ogl->min+ogl->max)/2;
  
  ogl->updateGL();
  statusBar()->showMessage ("Loading polyhedron done.",3000);
}

CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
	q_old.set(0.0, 0.0, 0.0, 1.0);
  q_now.set(0.0, 0.0, 0.0, 1.0);
  q_animated.set(0.0, 0.0, 0.0, 1.0);
  timeParameter = 0;
  animationRunning = false;
  animationMode = SLERP;

  setFocusPolicy(Qt::StrongFocus);
}

void CGView::initializeGL() {
	qglClearColor(Qt::black);
	zoom = 1.0;
	center = 0.0;
  
	// glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_NORMALIZE);
	//glEnable(GL_COLOR_MATERIAL);

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
  timer->start(20);
}

void CGView::paintModel() {
  glLineWidth(2.0);
  
  for(unsigned int i=0;i<ifs.size();++i) {
    glBegin(GL_LINE_LOOP);
    for(unsigned int j=0;j<ifs[i].size();++j)
      glVertex3dv(coord[ifs[i][j]].ptr());
    glEnd();
  }

  // glPointSize(5.0);
  // glBegin(GL_POINTS);
  // for(unsigned int i=0;i<coord.size();++i) 
  //   glVertex3dv(coord[i].ptr());
  // glEnd();
}

void CGView::paintGL() {
  glMatrixMode(GL_MODELVIEW);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw the rotated model
  glLoadIdentity();
  glTranslated(0, 0, -3);
  glMultMatrixd(currentRotationMatrix.transpose().ptr());
  glScaled(zoom, zoom, zoom);
  glTranslated(-center[0],-center[1],-center[2]);
  glColor4f(0.0, 0.3, 0.6, 0.7);
  paintModel();

  // draw the same without rotation
  glLoadIdentity();
  glTranslated(0, 0, -3);
  glScaled(zoom, zoom, zoom);
  glTranslated(-center[0],-center[1],-center[2]);
  glColor4f(0.2, 0.2, 0.2, 0.2);
  paintModel();
}

void CGView::timer() {
  if (animationRunning) {
    double dt = 0.02;
    timeParameter += dt;
    if (timeParameter > 1 + dt) {
      animationRunning = false;
      q_now = Quat4d(0,0,0,1);
      q_old = Quat4d(0,0,0,1);
      std::cout << "animation finished" << std::endl;
      std::cout << "q_now: " << q_now.q[0] << "," <<
        q_now.q[1] << "," << q_now.q[2] << "," <<
        q_now.q[3] << std::endl;
    } else {
      double t = fmin(1, timeParameter);
      double tc = 1 - timeParameter;
      if (animationMode == EULER_ANGLES) {
        std::cout << "q_now: " << q_now.q[0] << "," <<
          q_now.q[1] << "," << q_now.q[2] << "," <<
          q_now.q[3] << std::endl;

        Matrix4d m;
        m.setRotate(q_now);
        // this crashes with probability ~0...
        #define MAT(r,c) m(r, c)
        double pitch = -asin(MAT(2,0));
        double yaw = atan2(MAT(1,0), MAT(0,0));
        double roll = atan2(MAT(2,1), MAT(2,2));
        Matrix4d temp;

        std::cout << "Roll " << roll << " pitch " << pitch <<
          " yaw " << yaw << std::endl;

        currentRotationMatrix.makeIdentity();
        temp.makeRotate(tc * yaw, 0, 0, 1);
        currentRotationMatrix *= temp;
        temp.makeRotate(tc * pitch, 0, 1, 0);
        currentRotationMatrix *= temp;
        temp.makeRotate(tc * roll, 1, 0, 0);
        currentRotationMatrix *= temp;
      } else if (animationMode == SLERP) {
        double phi = acos(q_now.q[3]);
        double s = sin(phi);
        q_animated = 
          q_now * sin(tc * phi) / s +
          Quat4d(0,0,0,sin(t * phi) / s) ;
        currentRotationMatrix.makeRotate(q_animated);
      }
      updateGL();
    }
  }
}

void CGView::resizeGL(int width, int height) {
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (width > height) {
		double ratio = width/(double) height;
		gluPerspective(45,ratio,1.0,10000.0);
	} else {
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
  if (animationRunning) {
    q_now = q_animated;
    q_old = q_animated;
    animationRunning = false;
  }
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
    currentRotationMatrix.makeRotate(q_now);
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
  currentRotationMatrix.makeRotate(q_now);
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

void CGView::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
    case Qt::Key_S : 
      animationMode = SLERP;
      timeParameter = 0;
      animationRunning = true; 
      break;
    case Qt::Key_E :
      animationMode = EULER_ANGLES;
      timeParameter = 0;
      animationRunning = true; 
      break;
  }
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

