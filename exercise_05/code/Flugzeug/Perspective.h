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
#include "MyPlane.h"

#if _MSC_VER
	#include <gl/glu.h>
#elif __APPLE__
  #include <OpenGL/glu.h>
#else
	#include <GL/glu.h>
#endif

#ifndef VECMATH_VERSION
#  error "vecmath-library might be out-of-date. Please use newer version"
#endif

class CGView;

class CGMainWindow : public QMainWindow {
	Q_OBJECT

public:

	CGMainWindow (QWidget* parent = 0, Qt::WindowFlags flags = Qt::Window);
	~CGMainWindow ();

	QActionGroup *projectionMode;

public slots:

//	void loadPolyhedron();

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

  	Vector3d min, max, center, move;	// min, max and center of the coords of the loaded model

	double zoom;
	double phi,theta;
	std::vector<Vector3d> coord;			// the coords of the loaded model
	std::vector<std::vector<int> > ifs;		// the faces of the loaded model, ifs[i] contains the indices of the i-th face
	
protected:

	void paintGL();
	void resizeGL(int,int);

	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
	void keyPressEvent (QKeyEvent * event);

	CGMainWindow *main;
    int oldX,oldY;

private:

	
	double fov;// = 0.4;
	double maxHeight;// = 1;
		
	bool plane_fly;// = false;
	bool follow;//  = false;
		
	/// Hier startet der Flieger!
	Vector3d place;// (0,0.6,0);
		
	/// Diskreter Schritt bei der Bewegung!
	double step;// = 0.01;
		
	/// Das ist f"ur das H"ohenfeld!
	static const int sizeX = 100;
	static const int sizeY = 100;
	double height_vector[sizeX][sizeY];
	Vector3d normal_vector[sizeX][sizeY];
		
	/// Wireframe oder Filled?
	bool wire;// = false;
		
	/// Der Flieger
	MyPlane plane;
		
	void init_height ();
	void drawHeight ();

public slots:

	void updateProjectionMode() {
		resizeGL(width(),height());
		updateGL();
	}

	void reset_plane () {plane.placeMe (place);};
	void toggle_follow() {follow = !follow;};
	void timer ();

};

#endif
