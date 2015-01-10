#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include "BernsteinPol.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

#ifdef max
#undef max
#endif

CGMainWindow::CGMainWindow (QWidget* parent, Qt::WindowFlags flags)
: QMainWindow (parent, flags) {
    resize (604, 614);

    // Create a nice frame to put around the OpenGL widget
    QFrame* f = new QFrame (this);
    f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    f->setLineWidth(2);

    // Create our OpenGL widget
    ogl = new BernsteinPol (this,f);

	// Create a menu
    /*QMenu *file = new QMenu("Draw Methode",this);
    file->addAction ("&Wireframe", this->ogl, SLOT(drawWireFrame()), Qt::CTRL+Qt::Key_W);
    file->addAction ("&Flat-Shading",this->ogl,SLOT(flating()),Qt::CTRL+Qt::Key_F ),
    file->addAction ("&exact Smooth-Shading", this->ogl, SLOT(exactSmoothing()), Qt::CTRL+Qt::Key_S+Qt::Key_E);
    file->addAction ("&Smooth-Shading", this->ogl, SLOT(smoothing()), Qt::CTRL+Qt::Key_S);
    file->addAction ("Quit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);*/



    //menuBar()->addMenu(file);


	// Put the GL widget inside the frame
	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(ogl);
	layout->setMargin(0);
	f->setLayout(layout);

	setCentralWidget(f);

	statusBar()->showMessage("Ready",1000);
}

CGMainWindow::~CGMainWindow () {}




BernsteinPol::BernsteinPol (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
	main = mainwindow;

    setFocusPolicy(Qt::StrongFocus);
}

void BernsteinPol::initializeGL() {
    qglClearColor(Qt::white);

    zoom = 1.0;
    n=2;
    binomialCoeff=1;
    t.resize(250);
    for(int i=0;i<t.size();i++){
        t[i]=(i*1.0)/t.size();
    }

   /* glEnable(GL_NORMALIZE);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/


    ///Make timer
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
  timer->start(100); /// also 100 mal pro sekunde!

}

double BernsteinPol::polynomValue(double x,int i){
    return binomialCoeff*pow(x,i)*pow(1-x,n-i);

}

void BernsteinPol::verticesIPolynom(int i){
    for(int j=0;j<t.size()-1;j++){

        glVertex2d(t[j],polynomValue(t[j],i));
        glVertex2d(t[j+1],polynomValue(t[j+1],i));

    }
}

void BernsteinPol::paintGL() {
    //glEnable(GL_LIGHTING);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScaled(zoom,zoom,0);
    glTranslated(-0.5,-0.5,0);

    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_LINES);

    glColor3f(1.0,.0,.0);
    verticesIPolynom(2);
    glColor3d(0,0,0);
    glVertex2d(0,0);
    glVertex2d(0,1);
    glVertex2d(0,0);
    glVertex2d(1,0);
    glEnd();

   /* glEnable(GL_LIGHT0);
    GLfloat pos0[]={0,0,-10,0};
    glLightfv(GL_LIGHT0,GL_POSITION,pos0);
    glLineWidth(1.0);*/
}

void BernsteinPol::resizeGL(int width, int height) {
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (width > height) {
		double ratio = width/(double) height;
        glOrtho(-ratio,ratio,-1.0,1.0,-10.0,10.0);
	}
	else {
		double ratio = height/(double) width;
        glOrtho(-1.0,1.0,-ratio,ratio,-10.0,10.0);
	}
	glMatrixMode (GL_MODELVIEW);
}

void BernsteinPol::worldCoord(int x, int y, int z, Vector3d &v) {
#if RETINA_DISPLAY
      x *= 2;
      y *= 2;
#endif
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
        std::cout << "x: "<< x << " - y: " << y << " - z: " << z<< std::endl; 
        std::cout << viewport[0] << " " << viewport[1] << " " << viewport[2] << " " << viewport[3] << std::endl; 
	GLdouble M[16], P[16];
	glGetDoublev(GL_PROJECTION_MATRIX,P);
	glGetDoublev(GL_MODELVIEW_MATRIX,M);
	gluUnProject(x,viewport[3]-1-y,z,M,P,viewport,&v[0],&v[1],&v[2]);
}

void BernsteinPol::mousePressEvent(QMouseEvent *event) {
	oldX = event->x();
    oldY = event->y();
}


void BernsteinPol::mouseReleaseEvent(QMouseEvent*) {}

void BernsteinPol::wheelEvent(QWheelEvent* event) {
	if (event->delta() < 0) zoom *= 1.2; else zoom *= 1/1.2;
	update();
}

void BernsteinPol::mouseMoveEvent(QMouseEvent* event) {
    //alpha += -0.2*(event->x()-oldX);
    //beta += -0.2*(event->y()-oldY);
	oldX = event->x();
	oldY = event->y();

	updateGL();
}

//timer aktualisiert bild damit auswahl menue eintraege ueber tastenkombination strg+...
//wirkung zeigt ohne nochmal auf das menue zuklicken
void BernsteinPol::timer(){
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

