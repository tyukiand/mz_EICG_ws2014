#ifndef MODERNGL_H
#define MODERNGL_H

#include <QActionGroup>
#include <QApplication>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLocale>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QVector3D>
#include <QElapsedTimer>
#include <QSlider>
#include <QLabel>
#include <QComboBox>

#include <QtCore>
#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL/QGLShaderProgram>
#include <QOpenGLTexture>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include "vecmath.h"
class CGView;

class CGMainWindow : public QMainWindow {
    Q_OBJECT

public:

    CGMainWindow (QWidget* parent = 0); // , Qt::WFlags flags = Qt::Window);
    ~CGMainWindow ();
    CGView *ogl;

public slots:

protected:
   void keyPressEvent(QKeyEvent*);
};

class CGView : public QGLWidget, public QGLFunctions {
    Q_OBJECT

public:

    CGView(CGMainWindow*,QWidget*);
    void initShaders();
    void initializeGL();


    void initSolidCubeVBO();
    void initSolidSphereVBO();
    void initSolidCylinderVBO();
    void refineSolidSphere(const std::vector<QVector3D>&, std::vector<QVector3D>&);

    void initTorusVBO(double R, double r, int N, int n);
    void drawTorus();


    qreal zoom;

    GLuint vboSolidCubeId,vboSolidSphereId,vboSolidCylinderId,vboTorusId;
    int vboSolidCubeSize,vboSolidSphereSize,vboSolidCylinderSize,vboTorusSize;
    void trafo(int x, int y, int W, int H, QVector3D &v);

    QOpenGLTexture* tex[6];

    QVector3D lightPos;
    QVector3D lightDir;   
protected:

    void paintGL();
    void resizeGL(int,int);
    void drawScene();

    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
    void drawSolidSphere(const QVector3D&, qreal);
    void drawSolidCylinder(const QVector3D&, const QVector3D&, qreal);
    void drawSolidCube(const QVector3D&, qreal);
    void mouseToTrackball(int x, int y, int W, int H, QVector3D &v);
    QQuaternion trackball(const QVector3D& u, const QVector3D& v);

    CGMainWindow *main;
    int oldX,oldY;
    QQuaternion q_now;

private:

    QGLShaderProgram program, genShadowProgram;
    QMatrix4x4 projection,modelView,model, view;
    QMatrix4x4 light_view_matrix, light_projection_matrix, scale_bias_matrix, shadow_matrix;
    int width, height;

public slots:
    void updateProjectionMode() {
        resizeGL(width,height);
        updateGL();
    }  
};

#endif
