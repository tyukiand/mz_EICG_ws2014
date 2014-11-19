#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QGLWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include "Tangram.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

#ifdef max
#undef max
#endif

const double VERTEX_SNAP_DISTANCE = 0.15;
const double LINE_SNAP_DISTANCE = 0.15;
const double LINE_DISTANCE_PENALTY = 2;

CGMainWindow::CGMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags) {
    resize(604, 614);

    // Create a menu
    QMenu *file = new QMenu("&File", this);
    file->addAction(
        "Load polygon",
        this,
        SLOT(loadPolygon()),
        Qt::CTRL + Qt::Key_L
    );
    file->addAction("Quit", qApp, SLOT(quit()), Qt::CTRL + Qt::Key_Q);
    menuBar()->addMenu(file);

    // Create a nice frame to put around the OpenGL widget
    QFrame *f = new QFrame(this);
    f->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    f->setLineWidth(2);

    // Create our OpenGL widget
    ogl = new CGView(this, f);

    // Put the GL widget inside the frame
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(ogl);
    layout->setMargin(0);
    f->setLayout(layout);
    setCentralWidget(f);
    statusBar()->showMessage("Ready", 1000);
}

CGMainWindow::~CGMainWindow() { }

/**
 * Loads list of polygons from a .pol file.
 * Computes centers of mass for each polygon (we assume that only vertices
 * have mass, this works well only for polygons where the vertices are
 * distributed more or less evenly)
 */
void CGMainWindow::loadPolygon() {
    QString fn = QFileDialog::getOpenFileName(
                     this,
                     "Load polygon ...",
                     QString(),
                     "POL files (*.pol)"
                 );

    if (fn.isEmpty()) return;

    statusBar()->showMessage("Loading polygon ...");
    std::ifstream file(fn.toLatin1());
    double x, y;
    int m, n;
    double centerOfMassX, centerOfMassY;
    ogl->minX = ogl->minY = std::numeric_limits<double>::max();
    ogl->maxX = ogl->maxY = -ogl->minX;
    file >> n;
    ogl->poly.resize(n);
    ogl->trans.resize(n);
    ogl->center.resize(n);

    for (int i = 0; i < n; i++) {
        file >> m;
        ogl->poly[i].resize(3 * m);
        centerOfMassX = 0;
        centerOfMassY = 0;

        for (int j = 0; j < m; j++) {
            file >> x >> y;

            centerOfMassX += x;
            centerOfMassY += y;

            if (x < ogl->minX) ogl->minX = x;
            else if (x > ogl->maxX) ogl->maxX = x;

            if (y < ogl->minY) ogl->minY = y;
            else if (y > ogl->maxY) ogl->maxY = y;

            ogl->poly[i][3 * j + 0] = x;
            ogl->poly[i][3 * j + 1] = y;
            ogl->poly[i][3 * j + 2] = 0.0;

            std::cout << "Prepare to write trans and center" << std::endl;

            // note that we store the translation relative to
            // the original model coordinates of the polygon,
            // we do not move it to the center of mass
            ogl->trans[i].resize(3);
            ogl->trans[i][0] = 0;
            ogl->trans[i][1] = 0;
            ogl->trans[i][2] = 0;

            ogl->center[i].resize(2);
            ogl->center[i][0] = centerOfMassX / m;
            ogl->center[i][1] = centerOfMassY / m;
        }
    }

    file.close();
    std::cout << "minX = " << ogl->minX << std::endl;
    std::cout << "maxX = " << ogl->maxX << std::endl;
    std::cout << "minY = " << ogl->minY << std::endl;
    std::cout << "maxY = " << ogl->maxY << std::endl;
    ogl->zoom = 2.0 / std::max(ogl->maxX - ogl->minX, ogl->maxY - ogl->minY);
    // this is center of the scene
    ogl->centerX = (ogl->minX + ogl->maxX) / 2;
    ogl->centerY = (ogl->minY + ogl->maxY) / 2;
    ogl->i_picked = 0;
    ogl->updateGL();
    statusBar()->showMessage("Loading polygon done.", 3000);
}

const double ANGLE_CHANGE = M_PI / 12;
void CGMainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Q:
        ogl->trans[ogl->i_picked][2] += ANGLE_CHANGE;
        break;

    case Qt::Key_W:
        ogl->trans[ogl->i_picked][2] -= ANGLE_CHANGE;
        break;
    }

    ogl->updateGL();
}

CGView::CGView(CGMainWindow *mainwindow, QWidget *parent) : QGLWidget(parent) {
    main = mainwindow;
}

void CGView::initializeGL() {
    qglClearColor(Qt::white);
    zoom = 1.0;
    fillMode = 0;
    tobj = gluNewTess();
#ifdef _MSC_VER
    gluTessCallback(tobj, (GLenum)GLU_TESS_BEGIN, (void (APIENTRY *)()) glBegin);
    gluTessCallback(tobj, (GLenum)GLU_TESS_VERTEX, (void (APIENTRY *)()) glVertex3dv);
    gluTessCallback(tobj, (GLenum)GLU_TESS_END, (void (APIENTRY *)()) glEnd);
#else
    gluTessCallback(tobj, (GLenum)GLU_TESS_BEGIN, (void ( *)()) glBegin);
    gluTessCallback(tobj, (GLenum)GLU_TESS_VERTEX, (void ( *)()) glVertex3dv);
    gluTessCallback(tobj, (GLenum)GLU_TESS_END, (void ( *)()) glEnd);
#endif
    gluTessProperty(tobj, GLU_TESS_BOUNDARY_ONLY, GL_FALSE);
}

void CGView::paintGL() {
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < (int) poly.size(); i++) {
        // View
        glLoadIdentity();
        glScaled(zoom, zoom, 1.0);
        glTranslated(-centerX, -centerY, 0.0);

        // Model
        if (i == i_picked) {
            glTranslated(snapProposalX, snapProposalY, 0);
        }

        glTranslated(trans[i][0], trans[i][1], 0);
        glTranslated(center[i][0], center[i][1], 0);
        glRotated(trans[i][2] * 180 / M_PI, 0, 0, 1);
        glTranslated(-center[i][0], -center[i][1], 0);
        glColor4f(0.0f, 0.2f, 0.05f, 0.3f);

        if (i == i_picked) {
            glColor4f(0.6f, 0.5f, 0.3f, 0.8f);
        }

        // draw ith polygons interior
        gluTessBeginPolygon(tobj, NULL);
        gluTessBeginContour(tobj);

        for (int j = 0; j < (int) poly[i].size() / 3; j++)
            gluTessVertex(tobj, &poly[i][3 * j], &poly[i][3 * j]);

        gluTessEndContour(tobj);
        gluTessEndPolygon(tobj);
        // draw ith polygons polyline
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_LOOP);

        for (int j = 0; j < (int) poly[i].size() / 3; j++)
            glVertex2d(poly[i][3 * j + 0], poly[i][3 * j + 1]);

        glEnd();
    }
}

void CGView::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (width > height) {
        double ratio = width / (double) height;
        gluOrtho2D(-ratio, ratio, -1.0, 1.0);
    } else {
        double ratio = height / (double) width;
        gluOrtho2D(-1.0, 1.0, -ratio, ratio);
    }

    glMatrixMode(GL_MODELVIEW);
}

void CGView::worldCoord(int x, int y, double &dx, double &dy) {
    if (width() > height()) {
        dx = (2.0 * x - width()) / height();
        dy = 1.0 - 2.0 * y / (double) height();
    } else {
        dx = 2.0 * x / (double) width() - 1.0;
        dy = (height() - 2.0 * y) / width();
    }

    dx /= zoom;
    dy /= zoom;
    dx += centerX;
    dy += centerY;
}

/**
 * Tests whether a horizontal ray shot in the positive x-direction
 * from a start point `p` intersects a line segment from `a` to `b`.
 *
 * @return 0 if no intersection, 1 otherwise
 */
int CGView::intersect(
    double px, double py,
    double ax, double ay,
    double bx, double by
) {
    // we use a macro, becase we do not want to evaluate this expression
    // if it's not necessary, but we also do not want to write it out twice
    // or introduce additional helper function.
#define DET bx * py - by * px - ax * py + ay * px - bx * ay + by * ax
    return
        (py <= ay && py > by && DET < 0) ||
        (py > ay && py <= by && DET > 0);
#undef DET
}

/**
 * Transforms a point `(wx, wy)` from world coords to
 * the model coords of `i`-th polygon.
 *
 * Writes the result into `(px, py)`
 */
void CGView::worldToPolyCoords(
    int i,
    double wx, double wy,
    double &px, double &py
) {
    double tx = trans[i][0];
    double ty = trans[i][1];
    double angle = trans[i][2];
    double cx = center[i][0];
    double cy = center[i][1];

    // from poly to world:
    // w = T(trans) T(center) R(angle) T(-center) p
    // from world to local poly coords:
    // p = T(center) R(-angle) T(-center) T(-trans) w
    px = wx - tx - cx;
    py = wy - ty - cy;
    double cosA = cos(angle);
    double sinA = sin(angle);
    double tempPx = cosA * px + sinA * py;
    py = -sinA * px + cosA * py + cy;
    px = tempPx + cx;
}

void CGView::polyToWorldCoords(
    int i,
    double px, double py,
    double &wx, double &wy
) {
    double tx = trans[i][0];
    double ty = trans[i][1];
    double angle = trans[i][2];
    double cx = center[i][0];
    double cy = center[i][1];

    // from poly to world:
    // w = T(trans) T(center) R(angle) T(-center) w
    wx = px - cx;
    wy = py - cy;
    double cosA = cos(angle);
    double sinA = sin(angle);
    double tempWx = cosA * wx - sinA * wy;
    wy = sinA * wx + cosA * wy + cy + ty;
    wx = tempWx + cx + tx;
}

/**
 * Checks whether a point lies inside the the `i`-th polygon of
 * the `poly` array.
 */
bool CGView::insidePoly(int i, double px, double py) {
    int numIntersections = 0;

    // This is essentially the same method as in exercise 2, but
    // this time we have to transform the point `p` into the
    // model coords of the polygon (Note: this is of course much
    // faster than translating each vertex of the polygon into world
    // coords)
    double localPx, localPy;
    worldToPolyCoords(i, px, py, localPx, localPy);
    // the damn polygons aren't closed this time... work around
    int mod = poly[i].size();

    for (int j = 0; j < (int) poly[i].size() / 3; j++) {
        int j3 = 3 * j;
        numIntersections += intersect(
                                localPx, localPy,
                                poly[i][j3], poly[i][j3 + 1],
                                poly[i][(j3 + 3) % mod], poly[i][(j3 + 4) % mod]
                            );
    }

    return numIntersections % 2 == 1;
}

void CGView::mousePressEvent(QMouseEvent *event) {
    double dx, dy;
    worldCoord(event->x(), event->y(), dx, dy);
    std::cout << "Mouse pressed at (" << dx << "," << dy << ")" << std::endl;
    int i = 0;

    for (i = 0; i < poly.size(); i++) {
        if (insidePoly(i, dx, dy)) {
            std::cout << "Hit the polygon " << i << std::endl;
            i_picked = i;
            break;
        }
    }

    if (i == poly.size()) {
        i_picked = -1;
    }

    dragStartX = dx;
    dragStartY = dy;
    snapProposalX = 0;
    snapProposalY = 0;
    update();
}

void CGView::mouseReleaseEvent(QMouseEvent *event) {
    std::cout << "Mouse released" << std::endl;

    if (i_picked >= 0) {
        std::cout << "updating trans of " << i_picked << " by " <<
                  snapProposalX << ", " << snapProposalY << std::endl;
        trans[i_picked][0] += snapProposalX;
        trans[i_picked][1] += snapProposalY;
        snapProposalX = 0;
        snapProposalY = 0;
    }
}

void CGView::wheelEvent(QWheelEvent *event) {
    if (event->delta() < 0) zoom *= 1.1; else zoom *= 1 / 1.1;

    update();
}


/**
 * For a point `(x, y)` and a line segment `a` to `b`, computes a
 * point `p` on the line-segment `[a, b]` that is closest to `(x, y)`.
 */
void projectToLineSegment(
    double ax, double ay,
    double bx, double by,
    double x, double y,
    double &px, double &py
) {
    // `d` is the direction of the line-segment
    double dx = bx - ax;
    double dy = by - ay;

    // `r` is the position of `(x,y)` relative to `a`
    double rx = x - ax;
    double ry = y - ay;

    double rdDot = rx * dx + ry * dy;
    double dNormSq = dx * dx + dy * dy;

    // `p` is the closest point to `(x,y)` on the line segment
    if (rdDot <= 0 || dNormSq == 0) {
        px = ax;
        py = ay;
    } else if (rdDot >= dNormSq) {
        px = bx;
        py = by;
    } else {
        double f = rdDot / dNormSq;
        px = ax + f * dx;
        py = ay + f * dy;
    }
}

void CGView::proposeSnap(double xOffset, double yOffset) {
    // although some kind of bining would be appropriate in order
    // to keep the runtime roughly linear in the number of vertices
    // of the picked polygon,
    // we use a brute force method, and simply iterate through
    // all vertices and all edges of all other polygons.

    double minimalDistance = 10000000;
    snapProposalX = xOffset;
    snapProposalY = yOffset;
    double ax, ay; // vertex that snaps
    double bx, by; // candidate vertex from other polygon
    double cx, cy, dx, dy; // next endpoints of [a, c], [b, d]
    double projX, projY; // projections to line segments
    double dist; // distance to candidate
    int p, i, j;
    int n = poly[i_picked].size();
    int m;

    // This is only vertex-to-vertex snapping
    for (i = 0; i < n / 3; i++) {
        polyToWorldCoords(
            i_picked,
            poly[i_picked][3 * i], poly[i_picked][3 * i + 1],
            ax, ay
        );
        polyToWorldCoords(
            i_picked,
            poly[i_picked][(3 * i + 3) % n], poly[i_picked][(3 * i + 4) % n],
            cx, cy
        );
        ax += xOffset;
        ay += yOffset;
        cx += xOffset;
        cy += yOffset;

        for (p = 0; p < poly.size(); p++) {
            if (p != i_picked) {
                m = poly[p].size();

                for (j = 0; j < m / 3; j++) {
                    polyToWorldCoords(
                        p,
                        poly[p][3 * j], poly[p][3 * j + 1],
                        bx, by
                    );
                    polyToWorldCoords(
                        p,
                        poly[p][(3 * j + 3) % m], poly[p][(3 * j + 4) % m],
                        dx, dy
                    );
                    // compute distance between vertices
                    dist = hypot(ax - bx, ay - by);

                    if (dist < VERTEX_SNAP_DISTANCE && dist < minimalDistance) {
                        snapProposalX = xOffset + bx - ax;
                        snapProposalY = yOffset + by - ay;
                        minimalDistance = dist;
                    }

                    // compute projections of `a` to `[b, d]` and
                    // of `b` to `[a, c]`, penalize the distances with the
                    // line-snapping factor
                    projectToLineSegment(ax, ay, cx, cy, bx, by, projX, projY);
                    dist = hypot(projX - bx, projY - by);

                    if (dist < VERTEX_SNAP_DISTANCE &&
                            dist * LINE_DISTANCE_PENALTY < minimalDistance) {
                        snapProposalX = xOffset + bx - projX;
                        snapProposalY = yOffset + by - projY;
                        minimalDistance = dist * LINE_DISTANCE_PENALTY;
                    }

                    projectToLineSegment(bx, by, dx, dy, ax, ay, projX, projY);
                    dist = hypot(projX - ax, projY - ay);

                    if (dist < VERTEX_SNAP_DISTANCE &&
                            dist * LINE_DISTANCE_PENALTY < minimalDistance) {
                        snapProposalX = xOffset + projX - ax;
                        snapProposalY = yOffset + projY - ay;
                        minimalDistance = dist * LINE_DISTANCE_PENALTY;
                    }
                }
            }
        }
    }
}

void CGView::mouseMoveEvent(QMouseEvent *event) {
    QPoint current = event->pos();
    std::cout << "Mouse moved" << std::endl;
    int x = current.x();
    int y = current.y();

    double mouseX, mouseY;
    worldCoord(x, y, mouseX, mouseY);

    if (i_picked >= 0) {
        proposeSnap(mouseX - dragStartX, mouseY - dragStartY);
    }

    if (event->button() == Qt::LeftButton) {

    }

    updateGL();
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    if (!QGLFormat::hasOpenGL()) {
        qWarning("This system has no OpenGL support. Exiting.");
        return 1;
    }

    CGMainWindow *w = new CGMainWindow(NULL);
    w->show();
    return app.exec();
}

