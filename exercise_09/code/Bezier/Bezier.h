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
	void worldCoord(int, int, double&, double&);

	std::vector<Vector3d> control_points;
    double minX,minY,maxX,maxY;
	double centerX,centerY,zoom;
	int fillMode;
    bool opengl_bezier;
    bool own_bezier;

public slots:

    void ownBezier() {opengl_bezier = !opengl_bezier; updateGL();};
    void openglBezier() {own_bezier = !own_bezier; updateGL();};

protected:

	void paintGL();
	void resizeGL(int,int);

	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);

    double bezpoly(double x, unsigned int i, unsigned int n);
    int nearPoint(double x, double y);
    Vector3d de_casteljau(double t0,unsigned int r, unsigned int i, const std::vector<Vector3d> &l);
    Vector3d de_casteljau(double t0, const std::vector<Vector3d> &l);
	void draw_bezier_casteljau (const std::vector<Vector3d> &l);
    void opengl_method(const std::vector<Vector3d> &l);
    void projectToLineSegment(Vector3d a,Vector3d b,Vector3d x,Vector3d &p);
    void projToLine(Vector3d point, Vector3d &p);


	CGMainWindow *main;
    int pointMoving=-1;
    double clickedX,clickedY;
    double ep=0.08;
};

#endif

