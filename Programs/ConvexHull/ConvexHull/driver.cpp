#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <random>
#include "hull-bruteforce.h"

void readPoints(char const* filename, std::vector<Point> & points) {
	std::ifstream in(filename,std::ifstream::in);

	Point p;
	in >> p;
	while ( in.good() ) {
		points.push_back( p );
		in >> p;
	}
}

template <typename Container>
void print( Container const& indices, std::vector<Point> const& points ) {
    typename Container::const_iterator it   = indices.begin(), it_e = indices.end();
    for ( ; it != it_e; ++it ) {
        std::cout << points[ *it ] << std::endl;
    }
}

//rotate indices to brint pivot in front
//if pivot is not in indices do nothing (hullBruteForce2 has an error)
void rotate( std::vector<int> & indices, std::vector<Point> const& points, Point const& pivot ) {
    //find pivot in points
    size_t pivot_index = std::find( points.begin(), points.end(), pivot ) - points.begin();
    //std::cout << "pivot_index = " << pivot_index << std::endl;
    if ( pivot_index == points.size() ) throw "in rotate: pivot is not is points\n";

    //find pivot_index in indices and bring it forward
    std::vector<int>::iterator pivot_it = std::find( indices.begin(), indices.end(), pivot_index );
    //std::cout << "pivot_it = " << pivot_it-indices.begin() << std::endl;

    std::rotate(  indices.begin(), pivot_it, indices.end() );
}

void test0() { //simple hullBruteForce test
    Point p [] = { {0,0}, {0,2}, {2,0}, {2,2}, {1,1} };
    std::vector<Point> points( p, p+sizeof(p)/sizeof(p[0]) );
    std::set<int> hull = hullBruteForce( points );

    print( hull,points );
}

void test1() { //simple hullBruteForce2 test
    Point p [] = { {0,0}, {0,2}, {2,0}, {2,2}, {1,1} };
    std::vector<Point> points( p, p+sizeof(p)/sizeof(p[0]) );
    std::vector<int> hull = hullBruteForce2( points );
    
    Point pivot = {0,0};
    rotate( hull,points,pivot );
    print( hull,points );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//hullBruteForce tests
void test2() { 
    std::vector<Point> points;
    readPoints( "points1", points );
    std::set<int> hull = hullBruteForce( points );
    
    print( hull,points );
}

void test3() { 
    std::vector<Point> points;
    readPoints( "points2", points );
    std::set<int> hull = hullBruteForce( points );
    
    print( hull,points );
}

void test4() { 
    std::vector<Point> points;
    readPoints( "points3", points );
    std::set<int> hull = hullBruteForce( points );
    
    print( hull,points );
}

void test5() { 
    std::vector<Point> points;
    readPoints( "points4", points );
    std::set<int> hull = hullBruteForce( points );
    
    print( hull,points );
}

void test6() { 
    std::vector<Point> points;
    readPoints( "points5", points );
    std::set<int> hull = hullBruteForce( points );
    
    print( hull,points );
}

void test7() { 
    std::vector<Point> points;
    readPoints( "points6", points );
    std::set<int> hull = hullBruteForce( points );
    
    print( hull,points );
}

//same tests as 2..7, but using hullBruteForce2 function
//vector returned by student is rotated to bring a specified point in front
//to ensure identical output
void test8() { 
    std::vector<Point> points;
    readPoints( "points1", points );
    std::vector<int> hull = hullBruteForce2( points );
    
    Point pivot = {0,0};
    rotate( hull,points,pivot );
    print( hull,points );
}

void test9() { 
    std::vector<Point> points;
    readPoints( "points2", points );
    std::vector<int> hull = hullBruteForce2( points );
    
    Point pivot = {0,0};
    rotate( hull,points,pivot );
    print( hull,points );
}

void test10() { 
    std::vector<Point> points;
    readPoints( "points3", points );
    std::vector<int> hull = hullBruteForce2( points );
    
    Point pivot = {179 , 95};
    rotate( hull,points,pivot );
    print( hull,points );
}

void test11() { 
    std::vector<Point> points;
    readPoints( "points4", points );
    std::vector<int> hull = hullBruteForce2( points );
    
    Point pivot = {9 , 1};
    rotate( hull,points,pivot );
    print( hull,points );
}

void test12() { 
    std::vector<Point> points;
    readPoints( "points5", points );
    std::vector<int> hull = hullBruteForce2( points );
    
    Point pivot = {999 , 122};
    rotate( hull,points,pivot );
    print( hull,points );
}

void test13() { 
    std::vector<Point> points;
    readPoints( "points6", points );
    std::vector<int> hull = hullBruteForce2( points );
    
    Point pivot = {1,2726 };
    rotate( hull,points,pivot );
    print( hull,points );
}

void (*pTests[])() = { 
    test0,test1,                               //debugging
    test2,test3,test4,test5,test6,test7,       //hullBruteForce
    test8,test9,test10,test11,test12,test13 }; //hullBruteForce2

void GenNormalPoints()
{
	Point n;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution<> dis(0, 200);
	n.x = dis(gen);
	n.y = dis(gen);

}

void GenUniformPOints()
{
	Point u;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 199);
	u.x = dis(gen);
	u.y = dis(gen);

	std::ofstream outfile("data.200", std::ofstream::binary);
	//outfile.write(, 200);
}

int main(int argc, char ** argv) try {
	GenNormalPoints();
	GenUniformPOints();
	if (argc >1) {
		int test = 0;
		std::sscanf(argv[1],"%i",&test);
		pTests[test]();
	}
	return 0;
} catch( char const* str) {
	std::cout << str << std::endl;
}
