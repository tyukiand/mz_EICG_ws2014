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
#include <string> 
#include <iostream>
#include <math.h>

#include "Perspective.h"

#include "MyPlane.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CGMainWindow::CGMainWindow (QWidget* parent, Qt::WindowFlags flags)
: QMainWindow (parent, flags) {
	resize (604, 614);

	// Create a nice frame to put around the OpenGL widget
	QFrame* f = new QFrame (this);
	f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
	f->setLineWidth(2);

	// Create our OpenGL widget
	ogl = new CGView (this,f);

	QMenu *file = new QMenu("&Plane",this);
	file->addAction ("&Reset plane", this->ogl, SLOT(reset_plane()), Qt::CTRL+Qt::Key_R);
	file->addAction ("&Toggle follow", this->ogl, SLOT(toggle_follow()), Qt::CTRL+Qt::Key_T);
	file->addAction ("Quit", this, SLOT(close()), Qt::CTRL+Qt::Key_Q);

	menuBar()->addMenu(file);

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
	main = mainwindow;
	fov = 0.4;
	maxHeight = 1;
		
	plane_fly = false;
	sun = true;
	light_house = true;
	follow  = false;
		
	/// Hier startet der Flieger!
	place = Vector3d (0,0.6,0);
		
	/// Diskreter Schritt bei der Bewegung!
    step = 0.03;
		
	/// Wireframe oder Filled?
	wire = false;
		
	/// Positionen des Leuchttrurms und der Sonne!
	lightHousePos = Vector3d(2.5,0.70,0);
	lightHouseDir = Vector3d(5,-.08,0);
	sunPos = Vector3d(0,5,5);

	/// Um das H"ohenfeld zu erzeugen
  init_height ();

	/// Hier kommt der Flieger!
    plane = MyPlane ((char*)"redbaron.off");

	plane.placeMe (place);

	/// Um Keyboard-Events durchzulassen
	setFocusPolicy(Qt::StrongFocus);

}

Vector3d CGView::interpolate(Vector3d a, Vector3d b, double z, double begin, double end){
    double u=(z-begin)/(end-begin);//u(begin)=0,u(end)=1
    a*=(1-u);
    b*=u;
    Vector3d c=a+b;
    return c;
}

/// Berechnet die Farbe zum Rendern des Terrains!
void CGView::colorVector(double z, Vector3d& c){
		Vector3d gras (0.19, 0.617, 0.02);
		Vector3d erde (0.325, 0.2, 0);
		Vector3d schnee (0.78, 0.99, 0.99);
		Vector3d wasser (0.101, 0.027, 0.961);

		///Braun! Den rest macht IHR!
        c = Vector3d(.9,.5,.2);

        ///ADD YOUR CODE HERE
        if(z<0.15) c=wasser;
        if(z>=0.15 && z<=0.35)c=gras;
        if(z>0.35 && z<=0.5) c=interpolate(gras,erde,z,0.35,0.5);
        if(z>0.5) c=interpolate(erde,schnee,z,0.5,1);//z zwichen 0 und 1
	}
/*double CGView::projection(double point_x, double point_y, double x1, double y1, double x2, double y2){//proj: P(point)=r+((point-r)*u)*u wobei u normiert
    double t=0;
    t=((point_x-x1)*(x2-x1)+(point_y-y1)*(y2-y1))/((x2+x1)*(x2+x1)+(y2+y1)*(y2+y1));
    return t;
}*/

void CGView::setLightCol(Vector3d lightColorVec, GLfloat &lightColor0, GLfloat &lightColor1, GLfloat &lightColor2){
    lightColor0=lightColorVec.x();
    lightColor1=lightColorVec.y();
    lightColor2=lightColorVec.z();
}

/// Dies ist die Sonne
void CGView::setSun(Vector3d p) {

    ///ADD YOUR CODE HERE
       //gelb=(1,1,0)
    Vector3d gelb=Vector3d(1,1,0);
    Vector3d rot=Vector3d(1,0,0);
    Vector3d weis=Vector3d(1,1,1);
    Vector3d schwarz=Vector3d(0,0,0);
    Vector3d sunColor=Vector3d(1,1,0);
    Vector3d lightColVec=Vector3d(1,0,0);
    GLfloat lightColor[]={1,0,0,1};
    GLfloat lightamb[]={0.4,0.4,0.4,1};
    //GLfloat lightambGlo[]={0.2,0.2,0.2,1};
    //glLightModelf(GL_LIGHT_MODEL_AMBIENT,(0.,0.0,0.0,1));
    //glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
    double phiSun=0;
        //double s=acos(sunPos.x()/5);
        phiSun=acos(sunPos.x()/5)*(180/M_PI);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
    if(sunPos.x()<0 && sunPos.y()>0){
        //phiSun=projection(sunPos.x(),sunPos.y(),-5,0,0,5);

        lightColVec=interpolate(rot,weis,phiSun,180,90);
        setLightCol(lightColVec,lightColor[0],lightColor[1],lightColor[2]);
        sunColor=interpolate(rot,gelb,phiSun,180,90);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
        glDisable(GL_LIGHT1);
        light_house=false;
    }
    if(sunPos.x()>0 && sunPos.y()>0){
        //phiSun=projection(sunPos.x(),sunPos.y(),0,5,5,0);

        lightColVec=interpolate(weis,rot,phiSun,90,0);
        setLightCol(lightColVec,lightColor[0],lightColor[1],lightColor[2]);
        sunColor=interpolate(gelb,rot,phiSun,90,0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
        glDisable(GL_LIGHT1);
        light_house=false;
    }
    if(sunPos.x()>0 && sunPos.y()<0){
        //phiSun=projection(sunPos.x(),sunPos.y(),5,0,0,-5);

        lightColVec=interpolate(rot,weis,phiSun,0,90);
        setLightCol(lightColVec,lightColor[0],lightColor[1],lightColor[2]);
        sunColor=interpolate(rot,weis,phiSun,0,90);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightColor);
        glEnable(GL_LIGHT1);
        light_house=true;
    }
    if(sunPos.x()<0 && sunPos.y()<0){
        //phiSun=projection(sunPos.x(),sunPos.y(),0,-5,-5,0);

        lightColVec=interpolate(weis,rot,phiSun,90,180);
        setLightCol(lightColVec,lightColor[0],lightColor[1],lightColor[2]);
        sunColor=interpolate(weis,rot,phiSun,90,180);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightColor);
        glEnable(GL_LIGHT1);
        light_house=true;
    }
    //glLightfv(GL_LIGHT0, GL_AMBIENT, lightColor);
    GLfloat light_position[] = { sunPos.x(), sunPos.y(), sunPos.z(), 0.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glDisable (GL_LIGHTING);
	glPushMatrix();
        glTranslated(p[0],p[1],p[2]);
        glColor3d(sunColor.x(),sunColor.y(),sunColor.z());
		GLUquadric * q = gluNewQuadric();
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        gluSphere (q,.2,20,20);
		//glutSolidSphere(.2,20,20);
	glPopMatrix();
    glEnable (GL_LIGHTING);

}

/// Dies ist der Leuchtturm!
void CGView::lightHouse(Vector3d p, Vector3d d) {

	///ADD YOUR CODE HERE
    GLfloat light1Eigens[] = { 1, 1, 1., 1 };//weises licht
    GLfloat light_position[] = { lightHousePos.x(), lightHousePos.y(), lightHousePos.z(), 1 };
    GLfloat light_dir[] = { lightHouseDir.x(), lightHouseDir.y(), lightHouseDir.z() };
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light_dir);
    glLightf( GL_LIGHT1, GL_SPOT_CUTOFF, 10.0);
    glLightfv(GL_LIGHT1, GL_AMBIENT,  light1Eigens);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 10);//abschwaechung mit 1/entfernung^2
    glLightfv(GL_LIGHT1, GL_POSITION, light_position);


   glDisable (GL_LIGHTING);
	glPushMatrix();

		GLUquadric *myQuad;
		/// Die Birne des Leuchtturms
		glTranslated(p[0],p[1],p[2]);
		glColor3d(1,1,0);
		myQuad = gluNewQuadric();
		gluSphere (myQuad,.02,20,20);
        //glutSolidSphere(.02,20,20);

		/// Die rot/weisse Fassade des Leuchtturms
		myQuad=gluNewQuadric();
        glPushMatrix();
			//ROT
			glRotatef(90,1,0,0);
			glTranslated(0,0,0.3/12);
			glColor3f(1,0,0);
			gluCylinder( myQuad , 0.03 , 0.03 ,0.6/12, 10,10 );
			//WEISS
			glTranslated(0,0,0.6/12);
			glColor3f(1,1,1);
			gluCylinder( myQuad , 0.03 , 0.03 ,0.6/12, 10,10 );
			//ROT
			glTranslated(0,0,0.6/12);
			glColor3f(1,0,0);
			gluCylinder( myQuad , 0.03 , 0.03 ,0.6/12, 10,10 );
			//WEISS
			glTranslated(0,0,0.6/12);
			glColor3f(1,1,1);
			gluCylinder( myQuad , 0.03 , 0.03 ,0.6/12, 10,10 );
			//Schwarzes DACH	
			glTranslated(0,0,-2.8/12);
			glColor3f(0,0,0);
            gluCylinder( myQuad , 0.00 , 0.04 ,0.6/12, 10,10 );
		glPopMatrix();

		//Pr"ufstrahl der in Leuchtrichtung zeigt!
		glColor3f(1,1,0);
		glBegin(GL_LINES);
		glVertex3d( 0,0,0 );
        glVertex3d( .1*d[0], .1*d[1], .1*d[2] );
		glEnd();
    glPopMatrix();
    glEnable (GL_LIGHTING);
}

/// Initialisiert das H"ohenfeld
void CGView::init_height ()
{
	double s = .25;
	double t = .15;
  double a = sizeX*sizeY;
  for (int i=0;i<sizeX;i++)
    for (int j=0;j<sizeY;j++)
			{
      	height_vector[i][j] = (1/a)*(j*i)+0.05*(sin(s*i)+cos(t*j));
      	height_vector[i][j] *= height_vector[i][j];

				///Hier die normalen!
				Vector3d dx(1, j/a + s*0.05*cos(s*i),0);
				Vector3d dy(0, i/a - t*0.05*sin(t*j),1);
				normal_vector[i][j] = (dy%dx);
				normal_vector[i][j].normalize();
				
			}
  return;
}

/// Malt das H"ohenfeld
void CGView::drawHeight () {

	glPushMatrix ();
		glTranslated(-5.0, 0.0, -5.0);
		glScaled(10.0/(sizeX-1), 3.0, 10.0/(sizeY-1));	
		glLineWidth(1);	
		
		Vector3d c(1,1,1);				
		if (!wire) glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		else glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

		for (int i=0; i<(sizeX-1); i++){
			glBegin(GL_TRIANGLE_STRIP);
			for (int j=0; j<(sizeY); j++){			
					if (!wire) colorVector(height_vector[i][j], c);
					glColor3dv(c.ptr());
					glNormal3dv (normal_vector[i][j].ptr());
					glVertex3d(i, height_vector[i][j], j);
					if (!wire) colorVector(height_vector[i+1][j], c);
					glColor3dv(c.ptr());
					glNormal3dv (normal_vector[i+1][j].ptr());
					glVertex3d(i+1, height_vector[i+1][j], j);				
			}
		glEnd();
		}

		/// WASSER 

        /// ADD YOUR CODE HERE
        //glEnable (GL_BLEND);
        double alpha=0.7;
		/// Tesselierung des Wassers

		Vector3d wasser (0.101, 0.027, 0.961);

		double hWater = .15;
        //glColor3dv( wasser.ptr() );
        glColor4d(wasser.x(),wasser.y(),wasser.z(),alpha);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glNormal3d(0,1,0);
		for (int i=0; i<2*sizeX-1; i++){
            glBegin(GL_TRIANGLE_STRIP);
			for (int j=0; j<2*sizeY; j++){			
					glVertex3d(i,   hWater, j);
					glVertex3d(i+1, hWater, j);				
			}
        glEnd();
		}

    glPopMatrix ();
    //glDisable(GL_BLEND);
}

/// Das kennt Ihr aus der Vorlesung
void CGView::resizeGL(int width, int height) {
  double ratio;

  int W = width;
  int H = height;
  glViewport(0, 0, W, H);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();	

  if (W > H)
  {
    ratio = (double) W / (double) H;
    gluPerspective(180.0*fov,ratio,0.1,10000.0);
  }
  else
  {
    ratio = (double) H / (double) W;
    gluPerspective(180.0*fov,1.0/ratio,0.1,10000.0);
  } 	
  glMatrixMode(GL_MODELVIEW);
}


void CGView::mouseToTrackball (int x, int y, int W, int H, Vector3d &v) {

#if RETINA_DISPLAY
      x *= 2;
      y *= 2;
#endif
	double wx, wy, dist;

	// transform in world coordinates

	// panorama view
	if (W >= H) {
		wx = ( 2.0 / (double)H) * x - ((double)W / (double)H);
		wy = (-2.0 / (double)H) * y + 1.0;
	}

	// portrait view
	else {
		wx = ( 2.0 / (double)W) * x - 1.0;
		wy = (-2.0 / (double)W) * y + ((double)H / (double)W);
	}

	// click inside or outside the sphere
	dist = sqrt(wx*wx + wy*wy);

	// inside
	if (dist <= 1.0) {
		v[0] = wx;
		v[1] = wy;
		v[2] = sqrt(1.0 - wx*wx - wy*wy); // missing z-coordinate
	}

	// outside
	else {
		v[0] = wx;
		v[1] = wy;
		v[2] = 0.0;
		v.normalize(); // jump to next point on the sphere
	}
}

void CGView::trackball (Vector3d u, Vector3d v, Quat4d &q) {

	double qs;
	Vector3d qv, ucv;

	// create the components

	// via given formula (works)
	qs = 1 + u.dot(v);
	qv.cross(u, v);

	// compose and normalize
	q.set(qv[0], qv[1], qv[2], qs);
	q.normalize();
}

void CGView::initializeGL() {

  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
	glShadeModel(GL_SMOOTH);

  glClearColor(0.0,0.0,0.0,0.0);

	qglClearColor(Qt::black);
	zoom = 1.0;
	q_now = Quat4d (.1,.9,.1,.3);
	q_now.normalize();

	center = Vector3d(0.0, 0.0, 0.0);

	///Make timer
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
  timer->start(100); /// also 100 mal pro sekunde!
}


void CGView::paintGL() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/// Das ist zur korrekten Darstelung des Fliegers, wenn er 
  /// hinter oder vor dem H"ohenfeld fliegt! 
	/// Bitte vorerst ignorieren, das lernt ihr sp"ater!
	glEnable (GL_DEPTH_TEST);

  glLoadIdentity();


		/// Die Rotation zum betrachten der Szene
		/// Mit gluLookAt l"asst sich dem Flugzeug folgen
		/// Die Camera ist leicht "uber dem Flieger plaziert!
		if (follow) 
			{
				const Vector3d& eye = plane.origin() - plane.nose()*.4 + plane.up()*0.3;
				const Vector3d& cen = plane.origin();// + plane.nose()*2.0;

				gluLookAt (eye.x(), eye.y(), eye.z(),
									 cen.x(), cen.y(), cen.z(),
									 plane.up().x(), plane.up().y(), plane.up().z());
						} 
		else
			{
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				/*	Falls perspektivische Projektion verwendet wird, Szene etwas nach hinten schieben, 
					damit sie in der Sichtpyramide liegt. */
				 glTranslated(0.0,0.0,-3.0);
	
				// rotate using quaternions
				Matrix4d M, MT;
				M.makeRotate(q_now);
				MT = M.transpose();
				glMultMatrixd(MT.ptr());

				glScaled(zoom,zoom,zoom);
			}

		setSun(sunPos);
		lightHouse(lightHousePos,lightHouseDir);

    drawHeight();

    if (!wire) glEnable(GL_LIGHTING);
    
		plane.draw(Vector3d(1,0,0), !wire);

//  glutSwapBuffers();
}

void CGView::mousePressEvent(QMouseEvent *event) {
	mouseToTrackball(event->x(), event->y(), width(), height(), move);
}

void CGView::mouseReleaseEvent(QMouseEvent*) {}

void CGView::wheelEvent(QWheelEvent* event) {
	if (event->delta() < 0) zoom *= 1.2; else zoom *= 1/1.2;
	update();
}

void CGView::mouseMoveEvent(QMouseEvent* event) {
	Vector3d dest;
	Quat4d rotate;

	// get the rotation quaternion
	mouseToTrackball(event->x(), event->y(), width(), height(), dest);
	trackball(move, dest, rotate);

	// rotate global view
	q_now = rotate * q_now;
	q_now.normalize();

	// get ready for next rotation
	mouseToTrackball(event->x(), event->y(), width(), height(), move);

	updateGL();
}

/// Hier der timer-Slot zur Animation!
void CGView::timer()
{
  if(plane_fly) plane.flyMe (step); 

	/// spotlight rotieren:
	Matrix4d R;
	R.makeRotate(0.1,0,1,0);
	if (light_house) lightHouseDir = R*lightHouseDir;

	/// sonne rotieren:
	R.makeRotate(0.01,0,0,-1);
	if (sun) sunPos = R*sunPos;

  updateGL(); 
}

/// Ganz viele Funktionen zum steuern des Fliegers!
void CGView::keyPressEvent( QKeyEvent * event) {

	switch (event->key()) {
		case Qt::Key_Up    : plane.pitchMe (0.1); break;
		case Qt::Key_Down  : plane.pitchMe (-0.1); break;
		case Qt::Key_Left  : plane.rollMe (-0.1); break;
		case Qt::Key_Right : plane.rollMe (0.1); break;
		case Qt::Key_Space : plane_fly = !plane_fly; break;
		case Qt::Key_N     : plane.yawMe(0.1); break;
		case Qt::Key_M     : plane.yawMe(-0.1); break;
		case Qt::Key_W     : wire = !wire; break;
		case Qt::Key_L     : light_house = !light_house; break;
		case Qt::Key_S     : sun = !sun; break;
	}
		
	updateGL();
}

int main(int argc, char **argv) {

	QApplication app(argc, argv);

	if (!QGLFormat::hasOpenGL()) {
		qWarning ("This system has no OpenGL support. Exiting.");
		return 1;
	}

	CGMainWindow *w = new CGMainWindow(NULL);

	w->show();

	return app.exec();
}
