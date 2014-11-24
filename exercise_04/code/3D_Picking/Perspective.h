#ifndef MAPS_H
#define MAPS_H

#include <QMainWindow>
#include <QGLWidget>
#include <QGridLayout>
#include <QFrame>
#include <vector>
#include <iostream>

#include "vecmath.h"

#if _MSC_VER
	#include <gl/glu.h>
#elif __APPLE__
  #include <OpenGL/glu.h>
  #define RETINA_DISPLAY 0
#else
	#include <GL/glu.h>
#endif
class CGView;

class CGMainWindow : public QMainWindow {
	Q_OBJECT

public:

	CGMainWindow (QWidget* parent = 0, Qt::WindowFlags flags = Qt::Window);
	~CGMainWindow ();

public slots:

	void loadPolyhedron();

protected:

	void keyPressEvent(QKeyEvent*);

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
	int  findCloseVert(int x, int y, double epsPow2); // epsPow2 = epsilon^2

    Vector3d min, max, center;	// min, max and center of the coords of the loaded model
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

        CGMainWindow *main;
        int oldX,oldY;
        int selectedVertex;  // Index des ge-picked-ten Vertex
};

#endif
