#ifndef TANGRAM_H
#define TANGRAM_H

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
    CGMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = Qt::Window);
    ~CGMainWindow();
public slots:
    void loadPolygon();
public:
protected:
    void keyPressEvent(QKeyEvent *);
private:
    CGView *ogl;
};

// OGL Context
class CGView : public QGLWidget {
    Q_OBJECT
public:
    CGView(CGMainWindow *, QWidget *);

    void initializeGL();
    void worldCoord(int, int, double &, double &);

    std::vector<std::vector<double> > poly;
    double minX, minY, maxX, maxY;
    double centerX, centerY, zoom;
    int fillMode;
    int i_picked;

    // trans at i holds x-coord, y-coord and phi
    // of i-th polygon (in world-coords)
    std::vector<std::vector<double> > trans;

    // center holds x-coord and y-coord
    // of the center of the i-th polygon (relatively to model-coords)
    std::vector<std::vector<double> > center;

protected:
    void paintGL();
    void resizeGL(int, int);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

    CGMainWindow *main;
    GLUtriangulatorObj *tobj;
    bool insidePoly(int idx, double px, double py);
    int intersect(
      double px, double py, double ax, double ay, double bx, double by);
    void worldToPolyCoords(int i, double x, double y, double &px, double &py);
    void polyToWorldCoords(int i, double x, double y, double &wx, double &wy);
    double dragStartX;
    double dragStartY;

    /* computes a snap proposal from the current offset of the 
     * dragged polygon
     */
    void proposeSnap(double xOffset, double yOffset);

    /* the proposed offset that would result in some kind of "snapping" of
     * the currently dragged polygon
     */
    double snapProposalX;
    double snapProposalY;

    /*
     * coordinates of the point that would "snap"
     */
    double snappingPointX;
    double snappingPointY;
};
#endif

