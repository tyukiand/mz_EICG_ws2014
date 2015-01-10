#ifndef BERNSTEINPOL_H
#define BERNSTEINPOL_H


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


#include <string.h>
#include <fstream>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#if _MSC_VER
	#include <gl/glu.h>
#elif __APPLE__
  #include <OpenGL/glu.h>
  #define RETINA_DISPLAY 0
#else
	#include <GL/glu.h>
#endif
class BernsteinPol;

class CGMainWindow : public QMainWindow {
	Q_OBJECT

public:

	CGMainWindow (QWidget* parent = 0, Qt::WindowFlags flags = Qt::Window);
	~CGMainWindow ();


private:

    BernsteinPol *ogl;
};

class BernsteinPol : public QGLWidget {
	Q_OBJECT

public:

    BernsteinPol(CGMainWindow*,QWidget*);
	void initializeGL();
    double polynomValue(double x,unsigned int i,unsigned int n, double binomialCoeff);
	
	/** transforms the picture coords (x,y,z) to world coords by 
		inverting the projection and modelview matrix (as it it is 
		after invocation of paintGL) and stores the result in v */

	void worldCoord(int x, int y, int z, Vector3d &v);
    void verticesIPolynom(unsigned int i);
    double zoom;
    double binomCoeff(unsigned int n, unsigned int i);
    double colorCurv[10][3]={{0.,0.,1.},
                            {0.,1.,0.},
                            {0.,1.,1.},
                            {1.,0.,0.},
                            {1.,0.,1.},
                            {1.,1.,0.},
                            {0.,0.392,0.0},//Darkgreen
                            {0.545,0.271,0.075},//saddleBrown
                            {0.098,0.098,0.439},//midnightBlue
                            {0,0,0}
                           };



private:
    unsigned int n;
    std::vector<double> t;

	
protected:

	void paintGL();
	void resizeGL(int,int);

    /*void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);*/
    void keyPressEvent(QKeyEvent *event);

        CGMainWindow *main;
        int oldX,oldY;

public slots:

    void timer ();



};

#endif
