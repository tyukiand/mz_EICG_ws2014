#ifndef LIZARD_H
#define LIZARD_H

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

class CGView;

/// Main Window
class CGMainWindow : public QMainWindow {
	Q_OBJECT
public:
	CGMainWindow (QWidget* parent = 0, Qt::WindowFlags flags = Qt::Window);
	~CGMainWindow ();
public slots:
	void loadPolygon();
	void savePolygon();
	void fillPolygon();
public:
protected:
	void keyPressEvent(QKeyEvent*);
private:
	CGView *ogl;
};

// OGL Context
class CGView : public QGLWidget {
	Q_OBJECT
public:
	CGView(CGMainWindow*,QWidget*);

	void initializeGL();
	void worldCoord(int, int, double&, double&);

	std::vector<std::vector<double> > poly;
        double minX,minY,maxX,maxY;
	double centerX,centerY,zoom;
	int fillMode;
protected:
	void paintGL();
	void resizeGL(int,int);
	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);

	CGMainWindow *main;
	GLUtriangulatorObj* tobj;

private:
        int intersect(double px, double py, double ax, double ay, double bx, double by);
        bool insidePoly(double px, double py);
        double lastX;
        double lastY;
        bool lastClickInside;
};
#endif
	
