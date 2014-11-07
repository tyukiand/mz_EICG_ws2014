#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include "Lizard.h"

#define VISUAL_FEEDBACK

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

#ifdef max
#undef max
#endif

CGMainWindow::CGMainWindow (QWidget* parent, Qt::WindowFlags flags)
: QMainWindow (parent, flags) {
	resize (604, 614);

	// Create a menu
	QMenu *file = new QMenu("&File",this);
	file->addAction ("Load polygon", this, SLOT(loadPolygon()), Qt::CTRL+Qt::Key_L);
	file->addAction ("Save polygon", this, SLOT(savePolygon()), Qt::CTRL+Qt::Key_S);
	file->addAction ("Quit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);

	QMenu *options = new QMenu("&Options",this);
	options->addAction ("Contour/Filling", this, SLOT(fillPolygon()), Qt::CTRL+Qt::Key_F);

	menuBar()->addMenu(file);
	menuBar()->addMenu(options);

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

CGMainWindow::~CGMainWindow () { }

void CGMainWindow::fillPolygon() {
	ogl->fillMode = (ogl->fillMode+1)%3;
	ogl->updateGL();
}

void CGMainWindow::savePolygon() {
	QString fn = QFileDialog::getSaveFileName(this, "Save as ...", QString(), "POL files (*.pol)" );

	if (fn.isEmpty()) return;
	statusBar()->showMessage ("Saving polygon ...");
	std::ofstream file(fn.toLatin1());
	file.close();
}

void CGMainWindow::loadPolygon() {
	QString fn = QFileDialog::getOpenFileName(
    	this, 
    	"Load polygon ...", QString(), "POL files (*.pol)" 
    );

	if (fn.isEmpty()) return;
	statusBar()->showMessage ("Loading polygon ...");
	std::ifstream file(fn.toLatin1());
	double x,y;
	int m,n;

	ogl->minX = ogl->minY = std::numeric_limits<double>::max();
	ogl->maxX = ogl->maxY = -ogl->minX;

	file >> n;
	ogl->poly.resize(n);
	for(int i=0;i<n;i++) {
		file >> m;
		ogl->poly[i].resize(3*m);
		for(int j=0;j<m;j++) {
			file >> x >> y;
			if (x < ogl->minX) ogl->minX = x;
			else if (x > ogl->maxX) ogl->maxX = x;
			if (y < ogl->minY) ogl->minY = y;
			else if (y > ogl->maxY) ogl->maxY = y;
			ogl->poly[i][3*j+0] = x;
			ogl->poly[i][3*j+1] = y;
			ogl->poly[i][3*j+2] = 0.0;
		}
	}
	file.close();

	std::cout << "minX = " << ogl->minX << std::endl;
	std::cout << "maxX = " << ogl->maxX << std::endl;
	std::cout << "minY = " << ogl->minY << std::endl;
	std::cout << "maxY = " << ogl->maxY << std::endl;

	ogl->zoom = 2.0/std::max(ogl->maxX-ogl->minX,ogl->maxY-ogl->minY);
	ogl->centerX = (ogl->minX+ogl->maxX)/2;
	ogl->centerY = (ogl->minY+ogl->maxY)/2;

	ogl->updateGL();
	statusBar()->showMessage ("Loading polygon done.",3000);
}

void CGMainWindow::keyPressEvent(QKeyEvent* event) {
	switch(event->key()) {
	case Qt::Key_I: std::cout << "I" << std::flush; break;
	case Qt::Key_M: std::cout << "M" << std::flush; break;
	}

	ogl->updateGL();
}



// CGView

CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
	main = mainwindow;
	#ifdef VISUAL_FEEDBACK
        lastX = 0;
        lastY = 0;
        lastClickInside = false;
	#endif
}

void CGView::initializeGL() {
	qglClearColor(Qt::white);
	zoom = 1.0;
	fillMode = 0;

	tobj = gluNewTess();
#ifdef _MSC_VER
	gluTessCallback(tobj,(GLenum)GLU_TESS_BEGIN, (void(APIENTRY*)()) glBegin);
	gluTessCallback(tobj,(GLenum)GLU_TESS_VERTEX,(void(APIENTRY*)()) glVertex3dv);
	gluTessCallback(tobj,(GLenum)GLU_TESS_END,   (void(APIENTRY*)()) glEnd);
#elif __MINGW32__
    gluTessCallback(tobj,(GLenum)GLU_TESS_BEGIN, (void(APIENTRY*)()) glBegin);
    gluTessCallback(tobj,(GLenum)GLU_TESS_VERTEX,(void(APIENTRY*)()) glVertex3dv);
    gluTessCallback(tobj,(GLenum)GLU_TESS_END,   (void(APIENTRY*)()) glEnd);
#else
    gluTessCallback(tobj,(GLenum)GLU_TESS_BEGIN, (void(*)())glBegin);
    gluTessCallback(tobj,(GLenum)GLU_TESS_VERTEX,(void(*)())glVertex3dv);
    gluTessCallback(tobj,(GLenum)GLU_TESS_END,   (void(*)())glEnd);
#endif
	gluTessProperty(tobj,GLU_TESS_BOUNDARY_ONLY,GL_FALSE);
}

void CGView::paintGL() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScaled(zoom,zoom,1.0);
	glTranslated(-centerX,-centerY,0.0);

	glClear(GL_COLOR_BUFFER_BIT);

	if (fillMode > 0) {
		glColor3f(0.0f,1.0f,0.0f);
		for(int i=0;i<(int) poly.size();i++) {
			gluTessBeginPolygon(tobj,NULL);
			gluTessBeginContour(tobj);
			for(int j=0;j<(int) poly[i].size()/3;j++) 
				gluTessVertex(tobj,&poly[i][3*j],&poly[i][3*j]);
			gluTessEndContour(tobj);
			gluTessEndPolygon(tobj);		
		}
	}

	if (fillMode % 2 == 0) {
		glColor3f(0.0f,0.0f,0.0f);
		for(int i=0;i<(int) poly.size();i++) {
			glBegin(GL_LINE_STRIP);
			for(int j=0;j<(int) poly[i].size()/3;j++)
				glVertex2d(poly[i][3*j+0],poly[i][3*j+1]);
			glEnd();
		}
	}

    #ifdef VISUAL_FEEDBACK
    // if the last click was inside the polygon,
	// paint a bunch of colored points around the last click
	// position, for quick visual feedback. Only points inside the
	// polygon are painted.
	glColor3f(0.8f, 0.3f, 0.1f);
	if (lastClickInside) {
	    int i;
	    glBegin(GL_POINTS);
	    double px;
	    double py;
        for (i = 0; i < 10000; i ++) {
            px = lastX + cos(i * 0.03) * (i / 5000.0);
            py = lastY + sin(i * 0.03) * (i / 5000.0);
            if (insidePoly(px, py)) {
                glVertex2d(px, py);
            }
        }
        glEnd();
	}
    #endif
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
	dx += centerX;
	dy += centerY;
}

/**
 * Tests whether a horizontal ray shot in the positive x-direction
 * from a start point `p` intersects a line segment from `a` to `b`.
 * 
 * @return 0 if no intersection, 1 otherwise
 */
int CGView::intersect(
  double px, double py,
  double ax, double ay,
  double bx, double by
) { 
  // we use a macro, becase we do not want to evaluate this expression 
  // if it's not necessary, but we also do not want to write it out twice 
  // or introduce additional helper function.
  #define DET bx * py - by * px - ax * py + ay * px - bx * ay + by * ax
  return 
    (py <= ay && py > by && DET < 0) ||
    (py > ay && py <= by && DET > 0);
  #undef DET
}

void CGView::mousePressEvent(QMouseEvent *event) {
	double dx, dy;
	worldCoord(event->x(),event->y(),dx,dy);
	std::cout << "Mouse pressed at (" << dx << "," << dy <<")" << std::endl;

	double px = dx;		
	double py = dy;		
	// double ux,uy,vx,vy; // (generates unnecessary warnings)
	#ifdef VISUAL_FEEDBACK
	  lastX = px;
	  lastY = py;
	#endif
	int i = 0;
	int numIntersections = 0; // collides with the required `intersect` method, renamed
	for(int j=0;j<(int) poly[i].size()/3-1;j++){
		int j3=3*j; 
		numIntersections += intersect(
		  px, py, 
		  poly[i][j3], poly[i][j3 + 1], 
		  poly[i][j3 + 3], poly[i][j3 + 4]
		);
	}
	#ifdef VISUAL_FEEDBACK
	  lastClickInside = numIntersections % 2;
	  updateGL();
	#endif
	std::cout << ((numIntersections%2==1)?"inside":"outside") << std::endl;
}

/**
 * Checks whether a point lies inside the first polygon of the `poly` array.
 * (Extracted functionality from `mousePressedEvent` for testing purposes).
 */
bool CGView::insidePoly(double px, double py) {
  int i = 0;
  int numIntersections = 0; // collides with the required `intersect` method, renamed
  for(int j=0;j<(int) poly[i].size()/3-1;j++){
  	int j3=3*j; 
  	numIntersections += intersect(
  	  px, py, 
  	  poly[i][j3], poly[i][j3 + 1], 
  	  poly[i][j3 + 3], poly[i][j3 + 4]
  	);
  }
  return numIntersections % 2 == 1;
}

void CGView::mouseReleaseEvent (QMouseEvent* event) {
	std::cout << "Mouse released" << std::endl;
}

void CGView::wheelEvent(QWheelEvent* event) {
	if (event->delta() < 0) zoom *= 1.1; else zoom *= 1/1.1;
	update();
}

void CGView::mouseMoveEvent (QMouseEvent* event) {
	QPoint current = event->pos();
	std::cout << "Mouse moved" << std::endl;

	int x = current.x();
	int y = current.y();
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