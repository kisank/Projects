/*\file   hull-bruteforce.cpp
\author Kisan Khandelwal
\par    email: kisan.khandelwal@digipen.edu
\par    DigiPen login: kisan.khandelwal
\par    Course: CS597
\par    Assignment #3
\date   02/14/2017
\brief
This file contains the implementation of convex hull which has two methods defined below

Function contains:
hullbruteforce: this function returns indices that form convex hull,by finding if for a given 
set of two points all points lie either on right side or left side.
run-time complexity:O(N^3)

hullbruteforce2: this function is the optimization of above function with finding the lowest
x value in the given set of points and creating a line from this point to all other points.
The point is added in the list as a point on hull if all other points formed by this line are
on left side(counterclock wise).
run-time complexity:O(K*N^2)where k is the number of vertex on quick hull

*/
#include "hull-bruteforce.h"
#include <algorithm>
#include <iostream>

bool Point::operator==( Point const& arg2 ) const {
    return ( (x==arg2.x) && (y==arg2.y) );
}

std::ostream& operator<< (std::ostream& os, Point const& p) {
	os << "(" << p.x << " , " << p.y << ") ";
	return os;
}

std::istream& operator>> (std::istream& os, Point & p) {
	os >> p.x >> p.y;
	return os;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//return value is (on left, on right)
std::pair<bool,bool> get_location(
		float const& p1x, //check which side of the line (p1x,p1y)-->(p2x,p2y) 
		float const& p1y, //point (qx,qy) is on
		float const& p2x,
		float const& p2y,
		float const& qx,
		float const& qy
		) 
{
	Point dir   = {p2x-p1x,p2y-p1y};
	Point norm  = {dir.y, -dir.x};
	Point point = {qx-p1x,qy-p1y};
	//scalar product is positive if on right side
	float scal_prod = norm.x*point.x + norm.y*point.y;
	return std::make_pair( (scal_prod<0), (scal_prod>0));
}

//returns a set of indices of points that form convex hull
std::set<int> hullBruteForce ( std::vector< Point > const& points ) {
	int num_points = points.size();
	//std::cout << "number of points " << num_points << std::endl;
	if ( num_points < 3 ) throw "bad number of points";
	//stores the indices of convex hull
	std::set<int> hull_indices;
	//used to compare the current side of the point with previous side
	//if current side is not equal to previous side the point will not
	//be added to hull
	std::pair<bool, bool> prev_side(false,false);
	std::pair<bool, bool> current_side(false,false);

	//true for the point if it is on same side
	bool OnSameside;
	//used to initialize the previous state to current state
	int m = 0;

	//starting from first point in the list to make a line with second point
	//and compare with third
	for (int i = 0;i < num_points-1;++i)
	{
		//second point
		for (int j = i+1;j < num_points;++j)
		{
			//if current value at i is not same as j then enter
			if (i != j)
			{
				//initiliaze same side to true
				OnSameside = true;
				//third point to check if lies on the same side as other points
				for (int k = 0;k < num_points && OnSameside;++k)
				{
					//point i,j,k should not be same,skipped if either of them match
					if (k != j && k != i)
					{
						//get the current side of the point k,left value if for left side and right for right side
						current_side = get_location(points.at(i).x, points.at(i).y, points.at(j).x, points.at(j).y, points.at(k).x, points.at(k).y);
				
						//initialize to check previous and current side
						if (m == 0)
						{
							prev_side = current_side;
							//increment cuz initialized already,dont enter unless initialize require again
							m++;
						}
						//if previous side is not equal to current,point is not on same side
						if (prev_side != current_side)
						{
							OnSameside = false;
						}
									
					}
					//make previous side equal to current side before exiting
					prev_side = current_side;
				}
				//decrement m once j is over
				m--;
				//add the point in the hull if other points are on the same side
				if (OnSameside)
				{
					hull_indices.insert(i);
					//hull_indices.insert(points.at(i).y);
					hull_indices.insert(j);
					//hull_indices.insert(points.at(j).y);
				}
			}
		}
	}
	//return the indices of hull
	return hull_indices;
}

std::vector<int> hullBruteForce2 ( std::vector< Point > const& points ) {
	int num_points = points.size();
	if ( num_points < 3 ) throw "bad number of points";
	//two points,v1 is for vertex,f is first point in hull to compare and terminate while loop
	Point v1,f;
	f.x = -1;
	f.y = -1;
	//to store hull indices
	std::vector<int> hull_indices;
	//to determine which side of the line the point is,we dont want any point on right side
	std::pair<bool, bool> side;
	//we want to sweep counterclock wise so all points should be on left,so right is always false
	std::pair<bool, bool> wantedSide(true,false);
	//to iterate and find the lowest x value
	int a;
	//initialize v1 to starting point from the list of points
	v1 = points.at(0);
	//get the point with the lowest x and call it v1
	for (a = 0;a < num_points;++a)
	{
		if (points.at(a).x < v1.x )
		{
			v1 = points.at(a);
		}
	}
	//true if all points are on left side
	bool leftSide;
	//true  the first element added is added in hull_indices
	bool added = false;
	//to check if the last element added is the first element
	bool first=false;
	//loop till we get all points in the hull,i.e first point added is equal to last point added
	while (first!=true)
	{
		for (int i = num_points-1;i >=0;--i)
		{
			if (!first)
			{
				//initialize the three bools to false so they dont give the wrong points in the hull depending on previous values
				leftSide = false;
				side.first = false;
				side.second = false;
				//if ((points.at(i).x != v1.x) && (points.at(i).y != v1.y))
				//check if current point is equal to v1,if true skip this point
				if (!(points.at(i) == v1))
				{
					for (int j = 0;j < num_points;++j)
					{
						//check if current point is equal to jth point,if true skip this point
						if (!(points.at(j) == v1))
						{	//break;
							{
								//check if ith point is same as jth point,if true skip the jth point
								if (i != j)
								{
									//check which side the current point is to the line formed by v1 and ith point
									side = get_location(v1.x, v1.y, points.at(i).x, points.at(i).y, points.at(j).x, points.at(j).y);

									//if on right side,stop
									if (side.second == true)
										break;
									//if on left side,left side=true else make left side false
									if (side == wantedSide)
									{
										leftSide = true;
									}
									else leftSide = false;
								}
								//if on right side,break out of j loop
								if (side.second == true)
									break;
							}
						}
					}
					//if on right side,make left side false to avoid pushing the point in hull because of previous value
					if(side.second==true)
					{
						leftSide = false;
					}
				}
				//if on left side,add the point in the hull and make the newly added point as v1
				if (leftSide)
				{
					//make the current point to be added in hull as new v1
					v1 = points.at(i);
					//do not add the duplicate value in hull
					if (!(v1 == f))
					{
						hull_indices.push_back(i);
					}
					//leftSide = false;
					//if the current point added is same as the first point,make first true and break from while loop
					if (v1 == f)
					{
						first = true;
					}
					//make first point(v1) in the hull as first so next time when this point is added we break out of while loop
					if ((hull_indices.size() == 1) && (added == false))
					{						
						f = v1;
						added = true;
					}
				}
			}
		}
	}
	//return the points that form convex hull
	return hull_indices;
}
