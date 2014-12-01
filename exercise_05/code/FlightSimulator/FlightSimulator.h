#ifndef FLIGHT_SIMULATOR_H
#define FLIGHT_SIMULATOR_H

#include <QMainWindow>
#include <QGLWidget>
#include <QGridLayout>
#include <QFrame>
#include <QActionGroup>
#include <vector>
#include <iostream>

#include "vecmath.h"
#include "Jet.h"

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

// Properties of the generated terrain grid (10x10 km square)
const double landscapeSizeX = 10000;
const double landscapeSizeY = 10000;
const int sizeX = 100;
const int sizeY = 100;

// everything 100 m around buildings must be completely flat
const double townFlatRadius = 100;
// this radius away from the buildings the terrain can do whatever it wants
const double townMaxRadius = 1000;
const double townPlateauHeight = 1000;

class CGView;

class CGMainWindow : public QMainWindow {
    Q_OBJECT

public:

    CGMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = Qt::Window);
    ~CGMainWindow();

    QActionGroup *projectionMode;

public slots:

//  void loadPolyhedron();

private:

    CGView *ogl;

};

class CGView : public QGLWidget {
    Q_OBJECT

public:

    CGView(CGMainWindow *, QWidget *);
    void initializeGL();

    /** transforms the picture coords (x,y,z) to world coords by
        inverting the projection and modelview matrix (as it it is
        after invocation of paintGL) and stores the result in v */
    void worldCoord(int x, int y, int z, Vector3d &v);

    // Vector3d min, max, center, move;    // min, max and center of the coords of the loaded model

    double zoom;
    double phi, theta;
    bool wire;
    std::vector<Vector3d> coord;            // the coords of the loaded model
    std::vector<std::vector<int> > ifs;     // the faces of the loaded model, ifs[i] contains the indices of the i-th face

protected:

    void paintGL();
    void resizeGL(int, int);

    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void keyPressEvent(QKeyEvent *event);

    CGMainWindow *main;
    int oldX, oldY;
    double oldTheta, oldPhi;

private:


    double fov;// = 0.4;
    double maxHeight;// = 1;

    bool plane_fly;// = false;
    bool follow;//  = false;

    /// Hier startet der Flieger!
    Vector3d place;// (0,0.6,0);

    /// Diskreter Schritt bei der Bewegung!
    double step;// = 0.01;

    // this vector stores the weight of the terrain function relative to
    // the weight of the height preferred by Houston.
    double town_vs_terrain[sizeX][sizeY];

    // this vector stores the height of the surrounding terrain
    double height_vector[sizeX][sizeY];
    Vector3d normal_vector[sizeX][sizeY];

    /// Der Flieger
    Jet plane;

    // the town geometry
    std::vector<Vector3d> town_vertices;
    std::vector<int> town_triangles;

    void init_height();
    void init_town();
    void drawLandscape(bool wireframeOnly, Vector3d color);
    void drawTown();
    void drawOcean();

public slots:

    void updateProjectionMode() {
        resizeGL(width(), height());
        updateGL();
    }

    void reset_plane() {plane.placeMe(place);};
    void toggle_follow() {follow = !follow;};
    void toggle_wire() {wire = !wire;}
    void timer();

};

#endif
