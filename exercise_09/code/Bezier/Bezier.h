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
	bool show_bezier;

public slots:

	void toggleBezier() {show_bezier = !show_bezier; updateGL();};

protected:

	void paintGL();
	void resizeGL(int,int);

	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);

	void de_casteljau(double t0, const std::vector<Vector3d> &l);
	void draw_bezier_casteljau (const std::vector<Vector3d> &l);

	CGMainWindow *main;
};

#endif

