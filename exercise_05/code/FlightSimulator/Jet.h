#ifndef JET_H
#define JET_H

#include "vecmath.h"
#include <vector>
#include <algorithm>
#include <math.h>
#include <iostream>

#include <QtOpenGL>


class Jet {
private:

    void set_into_mass_center();
    double velocity;
    std::vector<Vector3d> points_;

    //Ursprung
    Vector3d origin_;

    //Nasenvektor
    Vector3d nose_;

    //Upvector
    Vector3d up_;

    double propellerAngle;
    double propellerPositionX;

public:

    Jet() : velocity(800), origin_(0, 0, 0), nose_(Vector3d(1, 0, 0)), up_(Vector3d(0, 0, 1)) {
        propellerAngle = 0;
    };

    void setPoints(std::vector< Vector3d> l) {
        points_.clear();
        points_.insert(points_.end(), l.begin(), l.end());
        set_into_mass_center();
    };

    ///Malt das Polygon
    void draw(Vector3d color,  bool fill);

    void placeMe(Vector3d p) {
        origin_ = p;
    };

    void flyMe(double dt) {
        origin_ += nose_ * (velocity * dt);
        propellerAngle += 30 * dt;
    };

    void rollMe(double alpha) {
        Vector3d ortho =  up_ % nose_;
        up_ = up_ * cos(alpha) - ortho * sin(alpha);
    }

    void yawMe(double alpha) {
        Vector3d ortho = up_ % nose_;
        nose_ = ortho * sin(alpha) + nose_ * cos(alpha);
    }

    void pitchMe(double alpha) {
        double ca = cos(alpha);
        double sa = sin(alpha);
        Vector3d new_nose_ = up_ * sa + nose_ * ca;
        up_ = up_ * ca - nose_ * sa;
        nose_ = new_nose_;
    }


    int size() {return points_.size();};

    Vector3d nose() {return nose_;};
    Vector3d up() {return up_;};
    Vector3d origin() {return origin_;};



};

#endif
