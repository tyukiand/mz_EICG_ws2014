#include "modernGL.h"

CGMainWindow *w;

CGMainWindow::CGMainWindow (QWidget* parent) 
	: QMainWindow (parent) {
    resize (1280, 720);

    // Create a nice frame to put around the OpenGL widget
    QFrame* f = new QFrame (this);
    f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    f->setLineWidth(2);

    // Create our OpenGL widget
    ogl = new CGView (this,f);
    // Create a menu
    QMenu *file = new QMenu("&File",this);
    file->addAction ("Quit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);

    menuBar()->addMenu(file);

    // Put the GL widget inside the frame
    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(ogl);
    layout->setMargin(0);
    f->setLayout(layout);

    setCentralWidget(f);
}

CGMainWindow::~CGMainWindow () {}

void CGMainWindow::keyPressEvent(QKeyEvent* event) {

    switch(event->key()) {
    case Qt::Key_Down: break;
    case Qt::Key_Up: break;
    }

    ogl->updateGL();
}

CGView::CGView (CGMainWindow *mainwindow,QWidget* parent ) : QGLWidget (parent) {
		main = mainwindow;
        setFocusPolicy(Qt::StrongFocus);
}

void CGView::initShaders() {
    setlocale(LC_NUMERIC, "C");

    if (!program.addShaderFromSourceFile(QGLShader::Vertex, ":/vshader.glsl"))
        close();

    if (!program.addShaderFromSourceFile(QGLShader::Fragment, ":/fshader.glsl"))
        close();

    if (!program.link())
        close();

    if (!program.bind())
        close();

    setlocale(LC_ALL, "");
}






void CGView::initializeGL() {
    initializeGLFunctions();
    initShaders();
    initSolidCubeVBO();
    initSolidSphereVBO();
    initSolidCylinderVBO();
    initTorusVBO(5.0,1.0,50,50);



    qglClearColor(Qt::black);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    zoom = 0.5;
    q_now = QQuaternion(1.0,0.0,0.0,0.0);

    // Prepare texture
    tex[0] = new QOpenGLTexture(QImage(":/Cube_Maps/cubemap_nvlobby_new/positive_x.png"));
    tex[1] = new QOpenGLTexture(QImage(":/Cube_Maps/cubemap_nvlobby_new/positive_y.png"));
    tex[2] = new QOpenGLTexture(QImage(":/Cube_Maps/cubemap_nvlobby_new/positive_z.png"));
    tex[3] = new QOpenGLTexture(QImage(":/Cube_Maps/cubemap_nvlobby_new/negative_x.png"));
    tex[4] = new QOpenGLTexture(QImage(":/Cube_Maps/cubemap_nvlobby_new/negative_y.png"));
    tex[5] = new QOpenGLTexture(QImage(":/Cube_Maps/cubemap_nvlobby_new/negative_z.png"));

    for (int i = 0; i < 6; ++i) {
        glBindTexture(GL_TEXTURE_CUBE_MAP,i);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X,0,4,tex[i]->width(),tex[i]->height(),0);
    }


    tex1->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    tex1->setMagnificationFilter(QOpenGLTexture::Linear);

}

void CGView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 R = QMatrix4x4();
    R.rotate(q_now);
    model.setToIdentity();
    view.setToIdentity();
    view.translate(0.0,0.0,-10.0);
    view *= R;
    view.scale(zoom,zoom,zoom);

    program.bind();
    program.setUniformValue("projection_matrix", projection);
    program.setUniformValue("model_matrix", model);
    program.setUniformValue("view_matrix", view);
    program.setUniformValue("normal_matrix", (view*model).normalMatrix());

    drawScene();

}

void CGView::drawScene(){

    unsigned int texLocation = program.uniformLocation("texture");
    glUniform1i(texLocation, 1);
    glActiveTexture(GL_TEXTURE1);
    tex1->bind();

    drawSolidSphere(QVector3D(0,0,0),2.0);
    drawSolidCylinder(QVector3D(-3,-3,0), QVector3D(-3,3,0),0.5);
    drawTorus();
    drawSolidCube(QVector3D(5,0,0),2.0);
}

void CGView::resizeGL(int w, int h) {
    width = w;
    height = h;
    glViewport(0,0,width,height);
    projection.setToIdentity();
    if (width > height) {
        qreal ratio = width/(qreal) height;
        //projection.ortho(-ratio,ratio,-1.0,1.0,-10.0,10.0);
        projection.perspective(45,ratio,1.0,1000.0);
    } else {
        qreal ratio = height/(qreal) width;
        //projection.ortho(-ratio,ratio,-1.0,1.0,-10.0,10.0);
        projection.perspective(45,ratio,1.0,1000.0);
    }
}

void CGView::mousePressEvent(QMouseEvent *event) {
    oldX = event->x();
    oldY = event->y();
    update();
}

void CGView::mouseReleaseEvent(QMouseEvent*) {
    update();
}

void CGView::wheelEvent(QWheelEvent* event) {
    if (event->delta() < 0) zoom *= 1.2; else zoom *= 1/1.2;
    update();
}

void CGView::mouseMoveEvent(QMouseEvent* event) {
    QVector3D p1, p2;

    //trackball
    mouseToTrackball(oldX,oldY,width,height,p1);
    mouseToTrackball(event->x(),event->y(),width,height,p2);
    QQuaternion q = trackball(p1,p2);
    q_now = q * q_now;
    q_now.normalize();

    oldX = event->x();
    oldY = event->y();

    updateGL();
}

void CGView::trafo(int x, int y, int W, int H, QVector3D &v){
    if (W > H) {

        v.setX((2.0*x-W)/W);   //*/W anstatt /H
        v.setY(1.0-y*2.0/H);
    } else {

        v.setX((2.0*x-W)/W);
        //v.setY((H-2.0*y)/W);
        v.setY(1.0-y*2.0/H);
    }
    v.setZ(0);
}

void CGView::mouseToTrackball(int x, int y, int W, int H, QVector3D &v) {
    if (W > H) {
        //v.setX((2.0*x-W)/H);
        v.setX((2.0*x-W)/W);
        v.setY(1.0-y*2.0/H);
    } else {
        v.setX((2.0*x-W)/W);
        //v.setY((H-2.0*y)/W);
        v.setY(1.0-y*2.0/H);
    }
    double d = v.x()*v.x()+v.y()*v.y();
    if (d > 1.0) {
        v.setZ(0.0);
        v /= sqrt(d);
    } else v.setZ(sqrt(1.0-d));
}

QQuaternion CGView::trackball(const QVector3D& u, const QVector3D& v) {
    QVector3D uxv = QVector3D::crossProduct(u, v);
    QQuaternion ret = QQuaternion(1.0+ QVector3D::dotProduct(u,v),uxv.x(),uxv.y(),uxv.z());
    ret.normalize();
    return ret;
}

int main (int argc, char **argv) {
    QApplication app(argc, argv);

    if (!QGLFormat::hasOpenGL()) {
        qWarning ("This system has no OpenGL support. Exiting.");
        return 1;
    }

    w = new CGMainWindow(NULL);

    w->show();

    return app.exec();
}
