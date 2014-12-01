#include "Jet.h"

void Jet::set_into_mass_center() {
    double xSum, ySum, zSum;
    propellerPositionX = -100000;
    xSum = 0;
    ySum = 0;
    zSum = 0;
    int n = this -> points_.size();

    for (int i = 0; i < n; ++i) {
        xSum += this->points_[i].x();
        ySum += this->points_[i].y();
        zSum += this->points_[i].z();
        propellerPositionX = fmax(propellerPositionX, this->points_[i].x());
    }

    xSum /= n;
    ySum /= n;
    zSum /= n;

    for (int i = 0; i < n; ++i) {
        points_[i] -= Vector3d(xSum, ySum, zSum);
    }

    propellerPositionX -= xSum;
}

void Jet::draw(Vector3d color = Vector3d(0, 0, 0),  bool fill = false) {
    glPushMatrix();
    Vector3d noxup = nose_ % up_;
    Matrix4d R(
        nose_.x(),   nose_.y(),   nose_.z(),   0,
        noxup.x(),   noxup.y(),   noxup.z(),   0,
        up_.x(),     up_.y(),     up_.z(),     0,
        origin_.x(), origin_.y(), origin_.z(), 1
    );

    glMultMatrixd(R.ptr());

    glColor3dv(color.ptr());

    // Sorry, fill does not work with polygons. I don't know what it is filling,
    // it's neither the polygon, nor the convex hull of it, it just looks like
    // garbage...
    fill = false;

    if (fill) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_POLYGON);
    } else glBegin(GL_LINE_LOOP);

    for (unsigned int i = 0; i < points_.size(); i++)
        glVertex3d(points_[i].x(), points_[i].y(), points_[i].z());

    glEnd();

    glColor3f(0.5, 0.5, 0.4);
    glBegin(GL_LINES);
    double c = cos(propellerAngle);
    double s = sin(propellerAngle);
    double r = 1.3;
    glVertex3d(propellerPositionX, c * r, s * r);
    glVertex3d(propellerPositionX, - c * r, -s * r);
    glEnd();

    glPopMatrix();
};