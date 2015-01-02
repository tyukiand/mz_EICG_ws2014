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

#ifndef VECMATH_VERSION
#  error "vecmath-library might be out-of-date. Please use newer version"
#endif

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
	Quat4d q_now;
  double zoom;
	double phi,theta;
	std::vector<Vector3d> coord;			// the coords of the loaded model
	std::vector<std::vector<int> > ifs;		// the faces of the loaded model, ifs[i] contains the indices of the i-th face

    Vector3d interpolate(Vector3d a, Vector3d b,double z,double begin, double end);//interpolates color
    /*double projection(double point_x,double point_y,double x1,double y1,double x2, double y2);// projektion von P auf gerade g=r+t*u
    //=>P'= r+ t'u t' spezielles t so dass P' orthogonal projektion von p auf gerade g rueckgabe t'*/
    void setLightCol(Vector3d lightColorVec, GLfloat& lightColor0,GLfloat& lightColor1,GLfloat& lightColor2);
	
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

    void mouseToTrackball(int, int, int, int, Vector3d&);
    void trackball (Vector3d, Vector3d, Quat4d&);
		// Field of Interest f"ur gluPerspective!
		double fov;// = 0.4;
		double maxHeight;// = 1;
		
		bool plane_fly;// = false;
		bool sun;// = true;
		bool light_house;// = true;
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
		
		/// Positionen des Leuchttrurms und der Sonne!
		Vector3d lightHousePos;//(2.5,0.70,0);
		Vector3d lightHouseDir;//(5,-.08,0);
		Vector3d sunPos;//(0,5,5);

		void colorVector(double z, Vector3d& c);
		void setSun(Vector3d p);
		void lightHouse(Vector3d p, Vector3d d);
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
