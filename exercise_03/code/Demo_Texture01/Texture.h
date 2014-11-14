#ifndef TEXTURE_H
#define TEXTURE_H

#include <QMainWindow>
#include <QGLWidget>
#include <QGridLayout>
#include <QFrame>
#include <iostream>

#if _MSC_VER
	#include <gl/glu.h>
#elif __APPLE__
  #include <OpenGL/glu.h>
#else
	#include <GL/glu.h>
#endif

#define N 100

class CGView;

class CGMainWindow : public QMainWindow {
	Q_OBJECT
public:
	CGMainWindow (QWidget* parent = 0, Qt::WindowFlags flags = Qt::Window);
	~CGMainWindow ();
public slots:
	void loadPicture();
	void showGrid();
	void largerBrush();
	void smallerBrush();
	void resetGrid();
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
	double centerX,centerY,oldPosX,oldPosY,zoom;

	GLuint texname;                               /// TEXTUR !
	void toggleGrid();
    void scaleBrush(float factor); // changes the size of the brush
    void resetGrid();
protected:
	void paintGL();
	void resizeGL(int,int);
	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
	CGMainWindow *main;

private:
	bool gridOn;
	float grid[N][N][2]; // grid coordinates of points that are moved around
	float T[N][N][2]; // constant texture coordinates in square [0,1]x[0,1]
	float brushX, brushY; // world coords of the brush
	float brushRadius; // radius of the edited region
	void scaleLocally(float factor); // "inflates" the image under the brush
	// selects the index of a grid point that is closest to the pixel `(px, py)`
    void pick(int px, int py, int &row, int &col); 
    bool draggingOn;
    int draggedRow;
    int draggedCol;
    float draggedX, draggedY;
    // returns coords of point (row, col) affected by dragging
    void dragPoint(int row, int col, float* arr2f); 
};

#endif
