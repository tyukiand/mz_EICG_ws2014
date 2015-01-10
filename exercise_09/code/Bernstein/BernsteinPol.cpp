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
   /* QMenu *file = new QMenu("Degree",this);
    file->addAction ("&Wireframe", this->ogl, SLOT(), Qt::CTRL+Qt::Key_W);


    menuBar()->addMenu(file);*/


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
    n=9;
    t.resize(250);
    for(unsigned int i=0;i<t.size();i++){
        t[i]=(i*1.0)/t.size();
    }
    t.resize(t.size()+1);
    t[t.size()-1]=1;



    ///Make timer
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
  timer->start(100); /// also 100 mal pro sekunde!

}

double BernsteinPol::binomCoeff(unsigned int n,unsigned int i){
    double result=1;
    for(unsigned int j=1;j<=i;j++){
        result=result*((n-i)*1.0/j+1);
    }
    return result;
}

double BernsteinPol::polynomValue(double x, unsigned int i, unsigned int n, double binomialCoeff){

    return binomialCoeff*pow(x,i)*pow(1-x,n-i);

}

void BernsteinPol::verticesIPolynom(unsigned int i){
    double binomialCoeff=binomCoeff(n,i);
    for(unsigned int j=0;j<t.size()-1;j++){

        glVertex2d(t[j],polynomValue(t[j],i,n,binomialCoeff));
        glVertex2d(t[j+1],polynomValue(t[j+1],i,n,binomialCoeff));

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
    glLineWidth(.5);
    glColor3d(0,0,0);
    //parallel y-achse
    glVertex2d(0,1);
    glVertex2d(1,1);
    glVertex2d(1,0.2);
    glVertex2d(0.98,0.2);
    glVertex2d(1,0.4);
    glVertex2d(0.98,0.4);
    glVertex2d(1,0.6);
    glVertex2d(0.98,0.6);
    glVertex2d(1,0.8);
    glVertex2d(0.98,0.8);

    //parallel x-Achse
    glVertex2d(1,0);
    glVertex2d(1,1);
    glVertex2d(0.2,1);
    glVertex2d(0.2,0.98);
    glVertex2d(0.4,1);
    glVertex2d(0.4,0.98);
    glVertex2d(0.6,1);
    glVertex2d(0.6,0.98);
    glVertex2d(0.8,1);
    glVertex2d(0.8,0.98);

    //glColor3f(1.0,.0,.0);
    for(unsigned int i=0;i<=n;i++){
        glColor3f(colorCurv[i][0],colorCurv[i][1],colorCurv[i][2]);
        verticesIPolynom(i);
    }

    glColor3d(0,0,0);
    //y-achse
    glVertex2d(0,0);
    glVertex2d(0,1);
    glVertex2d(0,0.2);
    glVertex2d(0.02,0.2);
    glVertex2d(0,0.4);
    glVertex2d(0.02,0.4);
    glVertex2d(0,0.6);
    glVertex2d(0.02,0.6);
    glVertex2d(0,0.8);
    glVertex2d(0.02,0.8);

    //x-achse
    glVertex2d(0,0);
    glVertex2d(1,0);
    glVertex2d(0.2,0);
    glVertex2d(0.2,0.02);
    glVertex2d(0.4,0);
    glVertex2d(0.4,0.02);
    glVertex2d(0.6,0);
    glVertex2d(0.6,0.02);
    glVertex2d(0.8,0);
    glVertex2d(0.8,0.02);
    glEnd();

    char ticks[5][3] ;//{"1","0.8","0.6","0.4","0.2"};
    strcpy(ticks[0],"1.0");
    strcpy(ticks[1],"0.8");
    strcpy(ticks[2],"0.6");
    strcpy(ticks[3],"0.4");
    strcpy(ticks[4],"0.2");

    double start=0.99;//count from 1 to 0 in 0.2 steps
    for(unsigned int i=0;i<5;i++){
        glPushMatrix();
        glTranslatef(-0.07, start, 0.0);
        glScalef(3e-4, 3e-4, 3e-4);
        glColor3f(0,0,0);
        unsigned int ticksLen=3;
        for(unsigned int j=0;j<ticksLen;j++)
            glutStrokeCharacter(GLUT_STROKE_ROMAN, ticks[i][j]);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(start-0.02,-0.04,  0.0);
        glScalef(3e-4, 3e-4, 3e-4);
        glColor3f(0,0,0);
        for(unsigned int j=0;j<ticksLen;j++)
            glutStrokeCharacter(GLUT_STROKE_ROMAN, ticks[i][j]);
        glPopMatrix();
        start=start-0.2;
    }

    double down=0.9;
    for(unsigned int m=0;m<=n;m++){
        glPushMatrix();
        glTranslatef(1.1, down, 0.0);
        glScalef(3e-4, 3e-4, 3e-4);
        glColor3f(0,0,0);
        glutStrokeCharacter(GLUT_STROKE_ROMAN, 'B');
        glutStrokeCharacter(GLUT_STROKE_ROMAN, '0'+m);
        glutStrokeCharacter(GLUT_STROKE_ROMAN, '0'+n);
        glutStrokeCharacter(GLUT_STROKE_ROMAN, '(');
        glutStrokeCharacter(GLUT_STROKE_ROMAN, 'x');
        glutStrokeCharacter(GLUT_STROKE_ROMAN, ')');
        glPopMatrix();
        glBegin(GL_LINES);
        glColor3f(colorCurv[m][0],colorCurv[m][1],colorCurv[m][2]);
        glVertex2d(1.25,down);
        glVertex2d(1.35,down);
        glEnd();
        down=down-0.06;
    }

   /* glEnable(GL_LIGHT0);
    GLfloat pos0[]={0,0,-10,0};
    glLightfv(GL_LIGHT0,GL_POSITION,pos0);
    */
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

/*void BernsteinPol::mousePressEvent(QMouseEvent *event) {
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
}*/

void BernsteinPol::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_0:
        n=0;
        break;

    case Qt::Key_1:
        n=1;
        break;

    case Qt::Key_2:
        n=2;
        break;

    case Qt::Key_3:
        n=3;
        break;

    case Qt::Key_4:
        n=4;
        break;

    case Qt::Key_5:
        n=5;
        break;

    case Qt::Key_6:
        n=6;
        break;

    case Qt::Key_7:
        n=7;
        break;

    case Qt::Key_8:
        n=8;
        break;

    case Qt::Key_9:
        n=9;
        break;
    }

   updateGL();
}

void BernsteinPol::timer(){
    updateGL();
}

int main (int argc, char **argv) {
	QApplication app(argc, argv);

	if (!QGLFormat::hasOpenGL()) {
		qWarning ("This system has no OpenGL support. Exiting.");
		return 1;
	}
    glutInit(&argc, argv);
	CGMainWindow *w = new CGMainWindow(NULL);

	w->show();

	return app.exec();
}

