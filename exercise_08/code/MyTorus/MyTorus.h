#ifndef MYTORUS_H
#define MYTORUS_H


#include <QMainWindow>
#include <QGLWidget>
#include <QGridLayout>
#include <QFrame>
#include <QActionGroup>
#include <vector>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <assert.h>

#include <QtOpenGL>

#include "vecmath.h"

#if _MSC_VER
	#include <gl/glu.h>
#elif __APPLE__
  #include <OpenGL/glu.h>
  #define RETINA_DISPLAY 0
#else
	#include <GL/glu.h>
#endif
class MyTorus;

class CGMainWindow : public QMainWindow {
	Q_OBJECT

public:

	CGMainWindow (QWidget* parent = 0, Qt::WindowFlags flags = Qt::Window);
	~CGMainWindow ();


private:

    MyTorus *ogl;
};

class MyTorus : public QGLWidget {
	Q_OBJECT

public:

    MyTorus(CGMainWindow*,QWidget*);
	void initializeGL();
	
	/** transforms the picture coords (x,y,z) to world coords by 
		inverting the projection and modelview matrix (as it it is 
		after invocation of paintGL) and stores the result in v */

	void worldCoord(int x, int y, int z, Vector3d &v);
    void draw(int p_num, int t_num);
    void torusNormal(double phi, double theta,Vector3d &normal);
    void quadNormals(int i,int n, Vector3d &normal1,Vector3d &normal2,Vector3d &normal3,Vector3d &normal4);
    void calPoint(int p_num, int t_num);
    void wireFrame();
    void flatShading();
    void smoothShading();
    void smoothExactShading();
    void colorChoose();

    double R,r;
	double zoom;
    std::vector<double> theta,phi;
    double alpha,beta;
    std::vector<std::vector<Vector3d> > coordTorus;		

private:
    int p_num,t_num;
    bool wire;
    bool flat;
    bool smooth;
    bool smoothExact;
    int colorCode;//colorCode: default: Kupfer 1:Rubin 2:Smaragdgruen 3:Silber

	
protected:

	void paintGL();
	void resizeGL(int,int);

	void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
    //void keyPressEvent(QKeyEvent*);

        CGMainWindow *main;
        int oldX,oldY;

public slots:

    void exactSmoothing(){
        wire=false;
        flat=false;
        smooth=false;
        smoothExact=true;
    }

    void flating(){
        wire=false;
        flat=true;
        smooth=false;
        smoothExact=false;
    }

    void drawWireFrame(){
        wire=true;
        flat=false;
        smooth=false;
        smoothExact=false;
    }

    void smoothing(){
        wire=false;
        flat=false;
        smooth=true;
        smoothExact=false;
    }

    void setKupfer(){
        colorCode=0;
    }

    void setRubin(){
        colorCode=1;
    }

    void setSmaragd(){
        colorCode=2;
    }

    void setSilber(){
        colorCode=3;
    }

    void timer ();



};

#endif
