#ifndef HELLOQGL_H
#define HELLOQGL_H

#include <QMainWindow>
#include <QGLWidget>
#include <QGridLayout>
#include <QFrame>
#include <vector>

#if _MSC_VER
	#include <gl/glu.h>
#elif __APPLE__
  #include <OpenGL/glu.h>
#else
	#include <GL/glu.h>
#endif
#include "vecmath.h"

#ifndef VECMATH_VERSION
#error "wrong vecmath included, must contain a VECMATH_VERSION macro"
#else
#if VECMATH_VERSION < 2
#error "outdatet vecmath included"
#endif
#endif

class CGView;

class CGMainWindow : public QMainWindow {
	Q_OBJECT

public:

	CGMainWindow (QWidget* parent = 0, Qt::WindowFlags flags = Qt::Window);
	~CGMainWindow ();


public:


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
    void worldCoord(int x, int y, int z, Vector3d &v);
    Vector3d min, max, center;

    std::vector<Vector4d> control_points;
    Quat4d q_old;
    Quat4d q_now;
    int oldX,oldY;
    int currentWidth, currentHeight;

    double minX,minY,maxX,maxY;
	double centerX,centerY,zoom;
	int fillMode;
    bool rotate;
    bool stuff;

public slots:

    //void ownBezier() {opengl_bezier = !opengl_bezier; updateGL();};
    //void openglBezier() {own_bezier = !own_bezier; updateGL();};

protected:

	void paintGL();
	void resizeGL(int,int);

	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);

    void gl_bezier3d(const std::vector<Vector3d> &l);
    void gl_proj4d(const std::vector<Vector4d> &l);

    void mouseToTrackball(int x, int y, int w, int h, Vector3d &v);
    void trackball(Vector3d u, Vector3d v, Quat4d &q);

	CGMainWindow *main;
    int pointMoving=-1;
    double clickedX,clickedY;
    double ep=0.08;
};

#endif

