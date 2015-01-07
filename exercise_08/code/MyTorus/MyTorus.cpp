#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include "MyTorus.h"

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
    ogl = new MyTorus (this,f);

	// Create a menu
    QMenu *file = new QMenu("&Draw Methode",this);
    file->addAction ("&Wireframe", this->ogl, SLOT(drawWireFrame()), Qt::CTRL+Qt::Key_W);
    file->addAction ("&Flat-Shading",this->ogl,SLOT(flating()),Qt::CTRL+Qt::Key_F ),
    file->addAction ("&exact Smooth-Shading", this->ogl, SLOT(exactSmoothing()), Qt::CTRL+Qt::Key_S+Qt::Key_E);
    file->addAction ("&Smooth-Shading", this->ogl, SLOT(smoothing()), Qt::CTRL+Qt::Key_S);
    file->addAction ("Quit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);

    QMenu *colMenu = new QMenu("&Color",this);
    colMenu->addAction ("&Kupfer", this->ogl, SLOT(setKupfer()), Qt::CTRL+Qt::Key_K);
    colMenu->addAction ("&Rubin",this->ogl,SLOT(setRubin()),Qt::CTRL+Qt::Key_R ),
    colMenu->addAction ("&Smaragd", this->ogl, SLOT(setSmaragd()), Qt::CTRL+Qt::Key_M);
    colMenu->addAction ("&Silber", this->ogl, SLOT(setSilber()), Qt::CTRL+Qt::Key_I);

    menuBar()->addMenu(file);
    menuBar()->addMenu(colMenu);


	// Put the GL widget inside the frame
	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(ogl);
	layout->setMargin(0);
	f->setLayout(layout);

	setCentralWidget(f);

	statusBar()->showMessage("Ready",1000);
}

CGMainWindow::~CGMainWindow () {}

void MyTorus::torusNormal(double phi, double theta, Vector3d &normal){
    Vector3d dT_dTheta (-(R+r*cos(phi))*sin(theta),(R+r*cos(phi))*cos(theta),0);
    Vector3d dT_dPhi (-r*sin(phi)*cos(theta),-r*sin(phi)*sin(theta),r*cos(phi));
    normal.cross(dT_dTheta,dT_dPhi);
    normal.normalize();

}

void MyTorus::calPoint(int p_num, int t_num){
    coordTorus.clear();
    for(int i=0;i<p_num;i++){
        phi.push_back((i+1)*2*M_PI/(p_num*1.0));
    }
    for(int l=0;l<t_num;l++){
        theta.push_back((l+1)*2*M_PI/(t_num*1.0));
    }
        for(int i=0;i<p_num;++i){
            std::vector<Vector3d> thetaCoords;
            for(int l=0;l<t_num;l++){
                double x=(this->R+this->r*cos(phi[i]))*cos(theta[l]);
                double y=(this->R+this->r*cos(phi[i]))*sin(theta[l]);
                double z=this->r*sin(phi[i]);
                /*if(x*pow10(16)<1&&x>0||x*pow10(16)>1&&x<0)
                    x=0;
                if(y*pow10(16)<1&&y>0||y*pow10(16)>1&&y<0)
                    y=0;
                if(z*pow10(16)<1&&z>0||z*pow10(16)>1&&z<0)
                    z=0;*/
                Vector3d points=Vector3d(x,y,z);
                thetaCoords.push_back(points);
            }
            coordTorus.push_back(thetaCoords);            
            phi.clear();
            theta.clear();
        }
    }
//}

/*void MyTorus::quadNormals(int i, int n, Vector3d &normal1, Vector3d &normal2, Vector3d &normal3, Vector3d &normal4){
    Vector3d actualQuad,quad1,quad2,quad3,quad4,quad5,quad6,quad7,quad8;
    //  quad1      quad2       quad3
    //  quad4    actualQuad    quad5
    //  quad7      quad7       quad8
    int n_1,i_1,i2,i1,n2,n1;
    if(n-1<0)
        n_1=coordTorus.size()-1;
    else
        n_1=n-1;
    if(i-1<0)
        i_1=coordTorus[n].size()-1;
    else
        i_1=i-1;
    if(i+2>=coordTorus[n].size()){
        if(i+1>=coordTorus[n].size()){
            i1=0;
            i2=1;
        }
        else
            i2=0;
    }
    else{
        i1=i+1;
        i2=i+2;
     }
    if(n+2>=coordTorus.size()){
        if(n+1>=coordTorus.size()){
            n1=0;
            n2=1;
        }
        else
            n2=0;
    }
    else{
        n1=n+1;
        n2=n+2;
     }
normal1.set(0,0,0);
normal2.set(0,0,0);
normal3.set(0,0,0);
normal4.set(0,0,0);


    Vector3d a=coordTorus[n][i]-coordTorus[n1][i];
    Vector3d b=coordTorus[n][i]-coordTorus[n1][i1];
    actualQuad.cross(b,a);

    Vector3d c=coordTorus[n][i]-coordTorus[n_1][i];
    Vector3d d=coordTorus[n][i]-coordTorus[n_1][i1];
    quad2.cross(d,c);

    Vector3d e=coordTorus[n][i]-coordTorus[n1][i];
    Vector3d f=coordTorus[n][i]-coordTorus[n][i_1];
    quad5.cross(f,e);

    Vector3d g=coordTorus[n][i]-coordTorus[n][i_1];
    Vector3d h=coordTorus[n][i]-coordTorus[n_1][i];
    quad3.cross(h,g);

    Vector3d j=coordTorus[n][i1]-coordTorus[n_1][i1];
    Vector3d k=coordTorus[n][i1]-coordTorus[n][i2];
    quad1.cross(k,j);

    Vector3d l=coordTorus[n][i1]-coordTorus[n][i2];
    Vector3d m=coordTorus[n][i1]-coordTorus[n1][i1];
    quad4.cross(m,l);

    Vector3d p=coordTorus[n1][i1]-coordTorus[n1][i2];
    Vector3d z=coordTorus[n2][i1];
    Vector3d q=coordTorus[n1][i1]-z;
    quad6.cross(q,p);

    Vector3d r=coordTorus[n1][i1]-coordTorus[n2][i1];
    Vector3d s=coordTorus[n1][i1]-coordTorus[n1][i];
    quad7.cross(s,r);

    Vector3d u=coordTorus[n1][i]-coordTorus[n2][i];
    Vector3d v=coordTorus[n1][i]-coordTorus[n1][i_1];
    quad8.cross(v,u);

    normal1=actualQuad+quad2+quad3+quad5;
    normal1/=-4;
    normal1.normalize();

    normal2=actualQuad+quad7+quad8+quad5;
    normal2/=-4;
    normal2.normalize();

    normal3=actualQuad+quad4+quad6+quad7;
    normal3/=-4;
    normal3.normalize();

    normal4=actualQuad+quad1+quad2+quad4;
    normal4/=-4;
    normal4.normalize();
}*/

void MyTorus::wireFrame(){
    for (unsigned int n=0; n<coordTorus.size();n++ ){
        for(unsigned int i=0; i<coordTorus[n].size();i++){

            int n_1,i_1;
            n_1=n+1;
            i_1=i+1;
            if(n==coordTorus.size()-1)
                n_1=0;
            if(i==coordTorus[n].size()-1)
                i_1=0;
            glVertex3d(coordTorus[n][i].x(),coordTorus[n][i].y(),coordTorus[n][i].z());
            glVertex3d(coordTorus[n_1][i].x(),coordTorus[n_1][i].y(),coordTorus[n_1][i].z());
            glVertex3d(coordTorus[n_1][i_1].x(),coordTorus[n_1][i_1].y(),coordTorus[n_1][i_1].z());
            glVertex3d(coordTorus[n][i_1].x(),coordTorus[n][i_1].y(),coordTorus[n][i_1].z());
        }
    }
}

void MyTorus::flatShading(){
    for (unsigned int n=0; n<coordTorus.size();n++ ){
        for(unsigned  int i=0; i<coordTorus[n].size();i++){
            int n_1,i_1;//benoetigt fuer das schliessen des toruses
            n_1=n+1;
            i_1=i+1;
            if(n==coordTorus.size()-1)
                n_1=0;
            if(i==coordTorus[n].size()-1)
                i_1=0;
            Vector3d a=coordTorus[n][i]-coordTorus[n_1][i];
            Vector3d b=coordTorus[n][i]-coordTorus[n_1][i_1];
            Vector3d normal;
            normal.cross(b,a);
            normal.normalize();
            glNormal3d(normal.x(),normal.y(),normal.z());

            glVertex3d(coordTorus[n][i].x(),coordTorus[n][i].y(),coordTorus[n][i].z());
            glVertex3d(coordTorus[n_1][i].x(),coordTorus[n_1][i].y(),coordTorus[n_1][i].z());
            glVertex3d(coordTorus[n_1][i_1].x(),coordTorus[n_1][i_1].y(),coordTorus[n_1][i_1].z());
            glVertex3d(coordTorus[n][i_1].x(),coordTorus[n][i_1].y(),coordTorus[n][i_1].z());
         }
    }
}

/*void MyTorus::smoothShading(){
    for (unsigned int n=0; n<coordTorus.size();n++ ){
        int l=0;
        if(n>46){
           Vector3d z=coordTorus[48][0];
            l++;
        }
        for(unsigned int i=0; i<coordTorus[n].size();i++){
            int n_1,i_1;
            n_1=n+1;
            i_1=i+1;
            if(n==coordTorus.size()-1)
                n_1=0;
            if(i==coordTorus[n].size()-1)
                i_1=0;
            Vector3d normal1,normal2,normal3,normal4 ;
            quadNormals(i,n,normal1, normal2, normal3, normal4);

            glNormal3d(normal1.x(),normal1.y(),normal1.z());
            glVertex3d(coordTorus[n][i].x(),coordTorus[n][i].y(),coordTorus[n][i].z());

            glNormal3d(normal2.x(),normal2.y(),normal2.z());
            glVertex3d(coordTorus[n_1][i].x(),coordTorus[n_1][i].y(),coordTorus[n_1][i].z());

            glNormal3d(normal3.x(),normal3.y(),normal3.z());
            glVertex3d(coordTorus[n_1][i_1].x(),coordTorus[n_1][i_1].y(),coordTorus[n_1][i_1].z());

            glNormal3d(normal4.x(),normal4.y(),normal4.z());
            glVertex3d(coordTorus[n][i_1].x(),coordTorus[n][i_1].y(),coordTorus[n][i_1].z());
        }
    }
}*/

void MyTorus::smoothExactShading(){
    for (unsigned int n=0; n<coordTorus.size();n++ ){
        for(unsigned  int i=0; i<coordTorus[n].size();i++){
            Vector3d normal1,normal2,normal3,normal4 ;
            int n_1,i_1;
            n_1=n+1;
            i_1=i+1;
            if(n==coordTorus.size()-1)
                n_1=0;
            if(i==coordTorus[n].size()-1)
                i_1=0;
            torusNormal(phi[n],theta[i],normal1);
            torusNormal(phi[n_1],theta[i],normal2);
            torusNormal(phi[n_1],theta[i_1],normal3);
            torusNormal(phi[n],theta[i_1],normal4);

            glNormal3d(normal1.x(),normal1.y(),normal1.z());
            glVertex3d(coordTorus[n][i].x(),coordTorus[n][i].y(),coordTorus[n][i].z());

            glNormal3d(normal2.x(),normal2.y(),normal2.z());
            glVertex3d(coordTorus[n_1][i].x(),coordTorus[n_1][i].y(),coordTorus[n_1][i].z());

            glNormal3d(normal3.x(),normal3.y(),normal3.z());
            glVertex3d(coordTorus[n_1][i_1].x(),coordTorus[n_1][i_1].y(),coordTorus[n_1][i_1].z());

            glNormal3d(normal4.x(),normal4.y(),normal4.z());
            glVertex3d(coordTorus[n][i_1].x(),coordTorus[n][i_1].y(),coordTorus[n][i_1].z());
        }
    }
}

void MyTorus::colorChoose(){
    //colorCode: default: Kupfer 1:Rubin 2:Smaragdgruen 3:Silber
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat spekular[4];
    GLfloat shine[1];

    switch(colorCode){
    case 1:
        ambient[0]= 0.17;   ambient[1]= 0.01;   ambient[2]= 0.01;   ambient[3]= 0.5;
        diffuse[0] = 0.61;  diffuse[1]= 0.04;   diffuse[2]= 0.04;   diffuse[3]= 0.5;
        spekular[0]= 0.73;  spekular[1]=0.63;   spekular[2]=0.63;   spekular[3]= 0.5;
        shine[0]=76.8;
        break;
    case 2:
        ambient[0]= 0.02;   ambient[1]= 0.17;   ambient[2]= 0.02;   ambient[3]= 0.5;
        diffuse[0] = 0.08;  diffuse[1]= 0.61;   diffuse[2]= 0.08;   diffuse[3]= 0.5;
        spekular[0]= 0.63;  spekular[1]=0.73;   spekular[2]=0.63;   spekular[3]= 0.5;
        shine[0]=76.8;
        break;
    case 3:
        ambient[0]= 0.19;   ambient[1]= 0.19;   ambient[2]= 0.19;   ambient[3]= 1.0;
        diffuse[0] = 0.51;  diffuse[1]= 0.51;   diffuse[2]= 0.51;   diffuse[3]= 1.0;
        spekular[0]= 0.51;  spekular[1]=0.51;   spekular[2]=0.51;   spekular[3]= 1.0;
        shine[0]=51.2;
        break;
    default:
        ambient[0]= 0.19;   ambient[1]= 0.07;   ambient[2]= 0.02;   ambient[3]= 1.0;
        diffuse[0] = 0.70;  diffuse[1]= 0.27;   diffuse[2]= 0.08;   diffuse[3]= 1.0;
        spekular[0]= 0.26;  spekular[1]=0.14;   spekular[2]=0.09;   spekular[3]= 1.0;
        shine[0]=12.8;
        break;
    }

    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,ambient);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR, spekular);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS, shine );
}

void MyTorus::draw(int p_num, int t_num){
    calPoint(p_num,t_num);

    if(wire)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else{
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if(flat)
            glShadeModel(GL_FLAT);
        if(smooth || smoothExact)
            glShadeModel(GL_SMOOTH);
    }
    glBegin(GL_QUADS);

    glColor3f(1,0,0);
    colorChoose();
    if(wire)
        wireFrame();
    else{
        if(flat)
            flatShading();
        /*if(smooth)
            smoothShading();*/
        if(smoothExact)
            smoothExactShading();
     }

    glEnd();
}


MyTorus::MyTorus (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
	main = mainwindow;

    setFocusPolicy(Qt::StrongFocus);
}

void MyTorus::initializeGL() {
    qglClearColor(Qt::white);
	zoom = 1.0;
    alpha = -20.7;
    beta = 19.47;
    this->R=.5;
    this->r=.2;
    wire=true;
    flat=false;
    smooth=false;
    p_num=15;
    t_num=40;
    colorCode=1;
    glEnable(GL_NORMALIZE);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    ///Make timer
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
  timer->start(100); /// also 100 mal pro sekunde!

}

void MyTorus::paintGL() {
    if(!wire){
        glEnable(GL_LIGHTING);
    }
    else
        glDisable(GL_LIGHTING);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glRotated(beta,1.0,0.0,0.0);
    glRotated(alpha,0.0,1.0,0.0);
    glScaled(zoom,zoom,zoom);

	glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0,.0,.0);
    draw(this->p_num, this->t_num);

    glEnable(GL_LIGHT0);
    GLfloat pos0[]={0,0,-10,0};
    glLightfv(GL_LIGHT0,GL_POSITION,pos0);
    glLineWidth(1.0);
}

void MyTorus::resizeGL(int width, int height) {
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

void MyTorus::worldCoord(int x, int y, int z, Vector3d &v) {
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

void MyTorus::mousePressEvent(QMouseEvent *event) {
	oldX = event->x();
    oldY = event->y();
}


void MyTorus::mouseReleaseEvent(QMouseEvent*) {}

void MyTorus::wheelEvent(QWheelEvent* event) {
	if (event->delta() < 0) zoom *= 1.2; else zoom *= 1/1.2;
	update();
}

void MyTorus::mouseMoveEvent(QMouseEvent* event) {
    alpha += -0.2*(event->x()-oldX);
    beta += -0.2*(event->y()-oldY);
	oldX = event->x();
	oldY = event->y();

	updateGL();
}

//timer aktualisiert bild damit auswahl menue eintraege ueber tastenkombination strg+...
//wirkung zeigt ohne nochmal auf das menue zuklicken
void MyTorus::timer(){
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

