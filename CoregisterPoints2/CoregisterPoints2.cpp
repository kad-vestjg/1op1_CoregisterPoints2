// CoregisterPoints2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <boost/geometry.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/index/predicates.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
//#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "CoregisterPoints2.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

int visited = 0;

int main(int argc, char *argv[])
{

	//===================================================================================================
	//-- <defs>
	//===================================================================================================
	//
	long nHash = 100000;
	double CloseEnough = 0.1;  //10cm (0.1m)
	bool ExtraDebugOutput = false; //bool ExtraDebugOutput = true;
	//
	//===================================================================================================
	//-- </defs>
	//===================================================================================================


	typedef bg::model::point<double, 2, bg::cs::cartesian> point;
	typedef bg::model::box<point> box;
	typedef std::pair<point, unsigned> value;
	time_t start, mid, end;
	time(&start);

	//-- Boundingbox for Netherlands 
	double bbx_ll[2], bbx_ur[2];
	bbx_ll[0] = 10425.156;
	bbx_ll[1] = 306846.198;
	bbx_ur[0] = 278026.090;
	bbx_ur[1] = 621876.300;

	//remove the center of the Netherlands as (virtual) reference point
	double remove_point_x = 155000.0;
	double remove_point_y = 463000.0;


	std::cout << "+------------------------------+" << std::endl;
	std::cout << "|                              |" << std::endl;
	std::cout << "| --- CoregisterPoints <2> --- |" << std::endl;
	std::cout << "|                              |" << std::endl;
	std::cout << "+-----------------------------GV" << std::endl;

	double CloseEnough2 = CloseEnough*CloseEnough;

	//cmdline processing
	if (argc < 2) {
		std::cout << std::endl << "* USAGE: " << argv[0] << " <FilePathName.ext>" << std::endl;
		return 1;
	}
	std::string filename, line;
	std::cout << "- Reading: " << argv[1] << std::endl;

	std::ifstream fp(argv[1]);
	if (!fp.is_open()) {
		std::cout << "Error: Cannot open inputfile..." << std::endl;
		return 1;
	}

	std::vector<SPoint> points;
	int l;
	double rmx, rmy;

	long nLine = 0;
	while (getline(fp, line)) {
		nLine++;
		if ((nLine%nHash) == 0)
			std::cout << "#";

		SPoint newPoint;
		newPoint.Ipt = stoi(line.substr(0, 10));
		newPoint.pn = line.substr(10, 20);
		newPoint.X = stod(line.substr(30, 15));
		newPoint.Y = stod(line.substr(45, 15));
		newPoint.Z = stod(line.substr(60, 15));
		l = int(line.length()) - 80;
		if (l > 0)
			newPoint.fn = line.substr(80, l);

		//-- Check to see if it falls withing NL-boundingbox
		if ((newPoint.X >= bbx_ll[0]) && (newPoint.Y >= bbx_ll[1]) && (newPoint.X <= bbx_ur[0]) && (newPoint.Y <= bbx_ur[1])) {
			//then only add it if it is not the point that is to be removed.
			rmx = (newPoint.X - remove_point_x);
			rmx *= rmx;
			rmy = (newPoint.Y - remove_point_y);
			rmy *= rmy;
			if ( (rmx+rmy) > CloseEnough2)
				points.push_back(newPoint);
		}

		//debug
		/*
		std::cout << "----------------------------" << nLine << std::endl;
		std::cout << "ip: " << newPoint.Ipt << std::endl;
		std::cout << "pn: " << newPoint.pn << std::endl;
		std::cout << "x:  " << setprecision(15) << newPoint.X << std::endl;
		std::cout << "y:  " << setprecision(15) << newPoint.Y << std::endl;
		std::cout << "z:  " << setprecision(15) << newPoint.Z << std::endl;
		std::cout << "fn: " << newPoint.fn << std::endl;
		*/

	}

	fp.close();
	long npoints = long(points.size());
	time(&mid);
	std::cout << " Finished in " << difftime(mid, start) << "s" << std::endl;
	std::cout << "- Number of records: " << nLine << std::endl;
	std::cout << "- Number of usable points: " << npoints << std::endl;

	//-- Put the data in a Kd-tree
	std::cout << "- Creating Kd-tree." << std::endl;
	bgi::rtree< value, bgi::quadratic<16> > rt2;

	//-- Create (boost)-rtree of valid points
	for (long i = 0; i < npoints; i++) {
		point p = point(points[i].X,points[i].Y);
		rt2.insert(std::make_pair(p, size_t(i)));
	}

	time(&end);
	std::cout << "- Kd-tree created in " << difftime(end, mid) << "s" << std::endl;
	mid = end;

	//-- Output file
	std::string fnOut = std::string(argv[1]);
	ReplaceStringInPlace(fnOut, ".txt", "_output.csv");
	std::ofstream of(fnOut.c_str());
	std::cout << "- Writing output to: " << fnOut.c_str() << std::endl;
	if (!of.is_open()) {
		std::cout << "Error: Cannot open outputfile: " << fnOut << std::endl;
		return 1;
	}

	//-- Lookup each of the points
	std::cout << "- Searching for corresponding points" << std::endl;
	std::vector<value> returned_values;

	long nCorresponding = 0;
	point LookFor;
	box bbox;
	double px_min, px_max, py_min, py_max;
	size_t idx_ret;
	nLine = 0;

	for (long i = 0; i < npoints; i++) {
		LookFor = point(points[i].X, points[i].Y);
		returned_values.clear();

		//Define box first for quicklook
		px_min = points[i].X - CloseEnough;
		px_max = points[i].X + CloseEnough;
		py_min = points[i].Y - CloseEnough;
		py_max = points[i].Y + CloseEnough;

		bbox = box(point(px_min, py_min), point(px_max, py_max));
		nLine++;
		if ((nLine%nHash) == 0)
			std::cout << "#";

		rt2.query(bgi::within(bbox) && bgi::satisfies([&](value const& v) {return bg::distance(v.first, LookFor) <= CloseEnough; }), std::back_inserter(returned_values));

		//if there is only one point, that point is always the search-point (LookFor). Check this by comparing its index to i.
		if (returned_values.size() < 2) {
			//if (long(returned_values[0].second) == i) {
			//	//yup.... its the same point.
			//}
		}
		else {
			if (ExtraDebugOutput) {
				std::cout << "Found " << std::setw(6) << returned_values.size() - 1 << " points near point " << points[i].pn << " (" << std::setw(8) << i << ")" << std::endl;
			}

			for (int j = 0; j < returned_values.size(); j++) {
				idx_ret = returned_values[j].second;
				if (idx_ret != i) {
					of << std::setw(10) << i << "," << std::setw(10) << returned_values.size() << "," << points[i].pn.c_str() << "," << std::setprecision(15);
					of << points[i].X << "," << std::setprecision(15) << points[i].Y << "," << points[idx_ret].pn.c_str() << "," << std::setprecision(15);
					of << points[idx_ret].X << "," << std::setprecision(15) << points[idx_ret].Y << ",";
					of << points[i].fn.c_str() << "," << points[idx_ret].fn.c_str() << std::endl;
				}

			}
			
			nCorresponding += long(returned_values.size());
		}

	}

	of.close();

	//Extra <CR/LF> after hash symbols
	if (!ExtraDebugOutput)
		std::cout << std::endl;

	//Elapsed times
	time(&end);
	std::cout << "- " << nCorresponding << " correspondences found in " << difftime(end, mid) << "s" << std::endl;
	std::cout << "- Total time:   " << difftime(end, start) << "s" << std::endl;
	std::cout << "- Done...!" << std::endl;

	//ret

    return 0;
}

/*===============================================================
   Additional functions
  ===============================================================*/

void ReplaceStringInPlace(std::string& subject, const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}
