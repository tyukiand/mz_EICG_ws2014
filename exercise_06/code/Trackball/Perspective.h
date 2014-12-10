#ifndef PERSPECTIVE_H
#define PERSPECTIVE_H

#include <QMainWindow>
#include <QGLWidget>
#include <QGridLayout>
#include <QFrame>
#include <QActionGroup>
#include <vector>
#include <iostream>

#include "vecmath.h"

#ifndef VECMATH_VERSION
#error "wrong vecmath included, must contain a VECMATH_VERSION macro"
#else
#if VECMATH_VERSION < 2
#error "outdatet vecmath included"
#endif
#endif

#if _MSC_VER
	#include <gl/glu.h>
#elif __APPLE__
  #include <OpenGL/glu.h>
#else
	#include <GL/glu.h>
#endif

class CGView;

class CGMainWindow : public QMainWindow {
	Q_OBJECT
public:
	CGMainWindow (QWidget* parent = 0, Qt::WindowFlags flags = Qt::Window);
	~CGMainWindow ();
	QActionGroup *projectionMode;
private:
	CGView *ogl;	
};

class CGView : public QGLWidget {
	Q_OBJECT
public:
	CGView(CGMainWindow*,QWidget*);
	void initializeGL();
	/** transforms the picture coords (x,y,z) to world coords by 
		inverting the projection and modelview matrix (as it it is 
		after invocation of paintGL) and stores the result in v */
	void worldCoord(int x, int y, int z, Vector3d &v);

        Vector3d min, max, center;	
        // min, max and center of the coords of the loaded model
        //
	double zoom;
    Quat4d q_old;
	Quat4d q_now;
	
protected:

	void paintGL();
	void resizeGL(int,int);

	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);

    int oldX,oldY;
    int currentWidth, currentHeight;
    void mouseToTrackball(int x, int y, int w, int h, Vector3d &v);
    void trackball(Vector3d u, Vector3d v, Quat4d &q);
};

#endif
