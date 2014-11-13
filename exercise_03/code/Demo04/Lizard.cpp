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
	file->addAction ("Duplicate active polygon", this, SLOT(duplicatePolygon()), Qt::CTRL+Qt::Key_D);
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

CGMainWindow::~CGMainWindow () { }


void CGMainWindow::loadPolygon() {
	QString fn = QFileDialog::getOpenFileName(this, "Load polygon ...", QString(), "POL files (*.pol)" );

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

	// this is center of the scene
	ogl->centerX = (ogl->minX+ogl->maxX)/2;
	ogl->centerY = (ogl->minY+ogl->maxY)/2;
	
	ogl->i_picked = 0;

	// this is center of the polygon
	ogl->center.resize(1);	
	ogl->center[0].resize(2);	
	ogl->center[0][0]=ogl->centerX;	
	ogl->center[0][0]=ogl->centerY;	

	ogl->trans.resize(1);
	ogl->trans[0].resize(3);
	ogl->trans[0][0] = 0; // x
	ogl->trans[0][1] = 0; // y
	ogl->trans[0][2] = 0; // phi

	ogl->updateGL();
	statusBar()->showMessage ("Loading polygon done.",3000);
}



void CGMainWindow::duplicatePolygon() {
				//duplicates the picked polygon and rotates the copy by
				// 30 degree and translates it by 1 in x
				for(int i=0;i<ogl->poly.size();i++){	
								if(i==ogl->i_picked){
												int N = ogl->poly.size();
												ogl->poly.resize(N+1);
												ogl->poly[N].resize(ogl->poly[i].size());
												for (int j=0;j<ogl->poly[i].size();j++)
																ogl->poly[N][j] = ogl->poly[i][j];

												N = ogl->center.size(); 
												ogl->center.resize(N+1);
												ogl->center[N].resize(2);
												ogl->center[N][0] = ogl->center[i][0]; 
												ogl->center[N][1] = ogl->center[i][1]; 

												N = ogl->trans.size(); 
												ogl->trans.resize(N+1);
												ogl->trans[N].resize(3);
												ogl->trans[N][0] = ogl->trans[i][0]+1.0;
												ogl->trans[N][1] = ogl->trans[i][1]+1.0;
												ogl->trans[N][2] = ogl->trans[i][2]+30;
												ogl->i_picked = N;
												break;
								}
				}
		ogl->updateGL();	
}

void CGMainWindow::keyPressEvent(QKeyEvent* event) {


			// add your code to rotate the picked lizard
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
				qglClearColor(Qt::white);
				zoom = 1.0;
				fillMode = 0;

				tobj = gluNewTess();
#ifdef _MSC_VER
				gluTessCallback(tobj,(GLenum)GLU_TESS_BEGIN, (void(APIENTRY*)()) glBegin);
				gluTessCallback(tobj,(GLenum)GLU_TESS_VERTEX,(void(APIENTRY*)()) glVertex3dv);
				gluTessCallback(tobj,(GLenum)GLU_TESS_END,   (void(APIENTRY*)()) glEnd);
#else
				gluTessCallback(tobj,(GLenum)GLU_TESS_BEGIN, (void(*)()) glBegin);
				gluTessCallback(tobj,(GLenum)GLU_TESS_VERTEX,(void(*)()) glVertex3dv);
				gluTessCallback(tobj,(GLenum)GLU_TESS_END,   (void(*)()) glEnd);
#endif
				gluTessProperty(tobj,GLU_TESS_BOUNDARY_ONLY,GL_FALSE);
}

void CGView::paintGL() {
				glMatrixMode(GL_MODELVIEW);

				glClear(GL_COLOR_BUFFER_BIT);

				for(int i=0;i<(int) poly.size();i++) {

								// View
								glLoadIdentity();
								glScaled(zoom,zoom,1.0);
								glTranslated(-centerX,-centerY,0.0);

								// Model
								glTranslated(trans[i][0],trans[i][1],0);
								glTranslated(center[i][0],center[i][1],0);
								glRotated(trans[i][2],0,0,1);	
								glTranslated(-center[i][0],-center[i][1],0);
								glColor3f(0.0f,1.0f,0.0f);
								if (i == i_picked)
												glColor3f(1.0f,0.0f,0.0f);

								// draw ith polygons interior
								gluTessBeginPolygon(tobj,NULL);
								gluTessBeginContour(tobj);
								for(int j=0;j<(int) poly[i].size()/3;j++) 
												gluTessVertex(tobj,&poly[i][3*j],&poly[i][3*j]);
								gluTessEndContour(tobj);
								gluTessEndPolygon(tobj);		

								// draw ith polygons polyline
								glColor3f(0.0f,0.0f,0.0f);
								glBegin(GL_LINE_STRIP);
								for(int j=0;j<(int) poly[i].size()/3;j++)
												glVertex2d(poly[i][3*j+0],poly[i][3*j+1]);
								glEnd();
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
				dx += centerX;
				dy += centerY;
}



void CGView::mousePressEvent(QMouseEvent *event) {
				double dx, dy;
				worldCoord(event->x(),event->y(),dx,dy);
				std::cout << "Mouse pressed at (" << dx << "," << dy <<")" << std::endl;


			// add your intersect code here
			// also set the new i_picked 

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

				// add code here to move the picked lizard
		

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

