#ifndef MYPOLYGON_H
#define MYPOLYGON_H

#include "vecmath.h"
#include <vector>
#include <algorithm>
#include <math.h>
#include <iostream>

#include <QtOpenGL>


class MyPlane
{

public:

	MyPlane () : origin_(0,0,0), nose_(Vector3d(1,0,0)), up_(Vector3d(0,1,0))
		{
		};

	void setPoints (std::vector< Vector3d> l)
		{
			points_.clear ();
			points_.insert (points_.end(), l.begin(), l.end());
			set_into_mass_center ();
		};

	///Malt das Polygon
	void draw(Vector3d color = Vector3d(0,0,0),  bool fill=false)
		{
			glPushMatrix();

				Vector3d noxup = nose_%up_;
				Matrix4d R(
										nose_.x(),   nose_.y(),   nose_.z(),   0,
										noxup.x(),   noxup.y(),   noxup.z(),   0,
										up_.x(),     up_.y(),     up_.z(),     0,
										origin_.x(), origin_.y(), origin_.z(), 1);

				glMultMatrixd (R.ptr());

				glColor3dv (color.ptr());
				if (fill) 
					{
						glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
						glBegin(GL_POLYGON);
					}
				else glBegin(GL_LINE_LOOP);				
					for (unsigned int i=0;i<points_.size();i++)
						glVertex3d (points_[i].x(), points_[i].y(), points_[i].z());
				glEnd();

			glPopMatrix();
		};

	void placeMe (Vector3d p)
		{
			origin_ = p;
		};

	void flyMe (double step)
		{
			origin_ += nose_*step;
		};

	void rollMe (double alpha) 
		{
			///ADD YOUR CODE HERE 
		};

	void yawMe (double alpha) 
		{
			///ADD YOUR CODE HERE 
		};

	void pitchMe (double alpha)
		{
			///ADD YOUR CODE HERE
		};



		int size () {return points_.size();};

		Vector3d nose (){return nose_;};
		Vector3d up (){return up_;};
		Vector3d origin (){return origin_;};

private:

	void set_into_mass_center ()
		{
            ///ADD YOUR CODE HERE
            double xSum,ySum,zSum;
            xSum=0;
            ySum=0;
            zSum=0;
            for(int i=0;i<this->points_.size();++i){
                xSum+=this->points_[i][0];
                ySum+=this->points_[i][1];
                zSum+=this->points_[i][2];
            }
            xSum=xSum/this->points_.size();
            ySum=ySum/this->points_.size();
            zSum=zSum/this->points_.size();
            for(int i=0;i<this->points_.size();++i){
                points_[i][0]-=xSum;
                points_[i][1]-=ySum;
                points_[i][2]-=zSum;
            }
            origin_.set(0.0,0.0,0.0);
    }

	std::vector<Vector3d> points_;
	
	//Ursprung
	Vector3d origin_;

	//Nasenvektor
	Vector3d nose_;

	//Upvector
	Vector3d up_;

};

#endif
