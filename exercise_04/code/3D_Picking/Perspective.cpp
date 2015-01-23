#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include "Perspective.h"

#include <cmath>
#include <fstream>
#include <sstream>
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

void CGMainWindow::keyPressEvent(QKeyEvent* event) {
	switch(event->key()) {
	case Qt::Key_I: std::cout << "I" << std::flush; break;
	case Qt::Key_M: std::cout << "M" << std::flush; break;
	}

	ogl->updateGL();
}

CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
	main = mainwindow;
}

void CGView::initializeGL() {
	qglClearColor(Qt::black);
	zoom = 1.0;
	phi = -20.7;
	theta = 19.47;
	center = 0.0;
	selectedVertex = -1;
}

void CGView::paintGL() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotated(theta,1.0,0.0,0.0);
	glRotated(phi,0.0,1.0,0.0);
	glScaled(zoom,zoom,zoom);
	glTranslated(-center[0],-center[1],-center[2]);

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0,1.0,1.0);

	glLineWidth(2.0);
	
	for(unsigned int i=0;i<ifs.size();++i) {
		glBegin(GL_LINE_LOOP);
		for(unsigned int j=0;j<ifs[i].size();++j)
			glVertex3dv(coord[ifs[i][j]].ptr());
		glEnd();
	}

	glPointSize(5.0);
	glBegin(GL_POINTS);
	for(unsigned int i=0;i<coord.size();++i) glVertex3dv(coord[i].ptr());
	glEnd();

    if (selectedVertex >= 0) {
        glColor3f(1.0, 0, 0);
        glPointSize(8.0);
        glBegin(GL_POINTS);
          glVertex3dv(coord[selectedVertex].ptr());
        glEnd();
    }

}

void CGView::resizeGL(int width, int height) {
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (width > height) {
		double ratio = width/(double) height;
		glOrtho(-ratio,ratio,-1.0,1.0,-10.0,10.0);
//		glFrustum(-ratio,ratio,-1.0,1.0,1.0,100.0);
//		gluPerspective(fov,ratio,1.0,10000.0);
	}
	else {
		double ratio = height/(double) width;
		glOrtho(-1.0,1.0,-ratio,ratio,-10.0,10.0);
//		glFrustum(-1.0,1.0,-ratio,ratio,1.0,100.0);
//		gluPerspective(fov,1.0/ratio,1.0,10000.0);
	}
	glMatrixMode (GL_MODELVIEW);
}

// The method has been modified such that it just applies 
// `gluUnProject` to 
void CGView::worldCoord(int x, int y, int z, Vector3d &v) {
#if RETINA_DISPLAY
      x *= 2;
      y *= 2;
#endif
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	    std::cout << "Viewport: " << std::endl;
        std::cout << "x: "<< x << " - y: " << y << " - z: " << z<< std::endl; 
        std::cout << viewport[0] << " " << viewport[1] << " " << viewport[2] << " " << viewport[3] << std::endl; 
	GLdouble M[16], P[16];
	glGetDoublev(GL_PROJECTION_MATRIX,P);
	glGetDoublev(GL_MODELVIEW_MATRIX,M);
	gluUnProject(x,viewport[3]-1-y,z,M,P,viewport,&v[0],&v[1],&v[2]);
}

void CGView::mousePressEvent(QMouseEvent *event) {
	oldX = event->x();
	oldY = event->y();

    // adjusted it a bit so it looks better with "venus"
        double epsPow2 = 0.001;
    int alternativeSelectedVertex = findCloseVert(oldX,oldY,epsPow2);
    // this lets us rotate selected vertices
    if (alternativeSelectedVertex >= 0) {
	  selectedVertex = alternativeSelectedVertex;
    }
        updateGL();
}

/** schiesst einen Strahl ausgehend vom Bildschirmpunkt (x,y) durch die Szene 
    und gibt den Index eines Vertex zurueck,
	der einen Abstand geringer als Wurzel(epsPow2) zum Strahl hat und am 
	naechsten hinter der Bildschirmebene liegt. 
	Falls kein solcher Vertex gefunden wird, wird -1 zurueckgegeben. */
int CGView::findCloseVert(int x, int y, double epsPow2)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
    Vector3d mouseObjectCoords;
    worldCoord(x, y, 0, mouseObjectCoords);

    Vector3d someOtherPointUnderTheMouse;
    worldCoord(x, y, -42, someOtherPointUnderTheMouse);


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



void CGView::mouseReleaseEvent(QMouseEvent*) {}

void CGView::wheelEvent(QWheelEvent* event) {
	if (event->delta() < 0) zoom *= 1.2; else zoom *= 1/1.2;
	update();
}

void CGView::mouseMoveEvent(QMouseEvent* event) {
	phi += 0.2*(event->x()-oldX);
	theta += 0.2*(event->y()-oldY);
	oldX = event->x();
	oldY = event->y();

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

