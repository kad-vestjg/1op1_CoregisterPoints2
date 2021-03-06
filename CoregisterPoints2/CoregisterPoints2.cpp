/*===================================================================================================

                                          COREGISTERPOINTS (2)

---------------------------------------------------------------------------------------------------

 Performs coregistration of points within ONE datafile. Looks through all permutation of points
 to find those that are possibly the same, within a given range (10cm default).

 Note: On large datasets (what it is actually for), do not run it in debug mode (too slow)

---------------------------------------------------------------------------------------------------

 USAGE: Coregisterpoints2 <FileName.txt> [Distance (m)]

 <FileName.ext>   (Required)   Input filename of data, stored as Id Pn X Y Z in fixed format (!)
                               Output in FileName_Output.csv and FileName_Output_Overview.csv
 [Distance (m)]   (Optional)   Range within which points are set to identical

---------------------------------------------------------------------------------------------------

 (!) Dataformat of input file: (:: ClassicReadingRoutine = true)

 1        10        20        30        40        50        60        70        80
 +--------+---------+---------+---------+---------+---------+---------+---------+------->
       1417               10861       1779.559       2145.712          0.000    Comment
       1418               10862       1782.001       2146.667          0.000    Path
        ...                 ...            ...            ...            ...    .......

 (Fortran format: '(I10,A20,3F15.3,A)' )


 (!) Alternative dataformat of input file:  (:: ClassicReadingRoutine = false)

 == 4 arguments ==
 Point X Y Z

 1417 1779.559 2145.712 0.000
 1418 1782.001 2146.667 0.000
 .... ........ ........ .....

 ------------- OR -------------

 == 5 arguments ==
 Id Point X Y Z

 0 1417 1779.559 2145.712 0.000
 1 1418 1782.001 2146.667 0.000
 . .... ........ ........ .....


 ------------- OR -------------

 == >5 arguments ==
 Id Point X Y Z Extra Unused Data

 0 1417 1779.559 2145.712 0.000 path comment bs ....
 1 1418 1782.001 2146.667 0.000 only_path ....
 . .... ........ ........ ..... ....... ....... ....

 Free format, separated by one or more spaces.

---------------------------------------------------------------------------------------------------

 External references/libraries:

 -- Boost (geometry/predicates/rtree)
 
---------------------------------------------------------------------------------------------------

 Author:  G.M.B. Vestjens

 Version  Author            Date          Changes
 -------  ------            ----          -------
 1.0      G.M.B. Vestjens   17-11-2017    Finished version using a default distance of 10cm
 1.1      G.M.B. Vestjens   09-03-2018    Added a distance/range option for the commandline

---------------------------------------------------------------------------------------------------

                                                               (C) 1832-2018  Het Kadaster
                                                                              Postbus 9046 
                                                                              7300 GH Apeldoorn

===================================================================================================*/

#include "stdafx.h"
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/index/predicates.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>

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
	double CloseEnough = 0.1;           //10cm (0.1m), default value
	bool ExtraDebugOutput = false;      //bool ExtraDebugOutput = true;
	bool ClassicReadingRoutine = false; //use classic reading routine, otherwise use more modern version
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

	//-- remove the center of the Netherlands as (virtual) reference point
	double remove_point_x = 155000.0;
	double remove_point_y = 463000.0;


	std::cout << "+------------------------------+" << std::endl;
	std::cout << "|                              |" << std::endl;
	std::cout << "| --- CoregisterPoints <2> --- |" << std::endl;
	std::cout << "|                              |" << std::endl;
	std::cout << "+-----------------------------GV" << std::endl;

	//-- commandline processing
	if (argc < 2) {
		std::cout << std::endl << "* USAGE: " << argv[0] << " <FilePathName.ext> [distance (m)]" << std::endl;
		return 0xff;
	}

	if (argc == 3) {
		CloseEnough = strtod(argv[2], NULL);
	}

	//-- Use fixed format or free format. This depends on the name of the executable. If 
	//   this contains 'FIXED' then the fixed format option is used.
	std::string ename = boost::to_upper_copy(std::string(argv[0]));
	ClassicReadingRoutine = (ename.find("FIXED") != std::string::npos);
	if (ClassicReadingRoutine) {
		std::cout << "- Fixed format reader" << std::endl;
	}
	else {
		std::cout << "- Free format reader" << std::endl;
	}

	//-- Display entered coregistration range
	std::cout << "- Coregistration range: " << CloseEnough << " m" << std::endl;
	double CloseEnough2 = CloseEnough * CloseEnough;

	//-- Display inputfile
	std::string filename, line;
	std::cout << "- Reading: " << argv[1] << std::endl;

	//-- Open input file for reading, stop on error
	std::ifstream fp(argv[1]);
	if (!fp.is_open()) {
		std::cout << "Error: Cannot open inputfile..." << std::endl;
		return 1;
	}

	//-- Read the inputfile, interpret/parse the file and add points to 
	//   a point list. Hash (visualisation) along the way
	std::vector<SPoint> points;
	int l;
	double rmx, rmy;

	long nLine = 0;
	while (getline(fp, line)) {
		nLine++;
		if ((nLine%nHash) == 0)
			std::cout << "#";

		SPoint newPoint;
		if (ClassicReadingRoutine) {
			newPoint.Ipt = stoi(line.substr(0, 10));
			newPoint.pn = line.substr(10, 20);
			newPoint.X = stod(line.substr(30, 15));
			newPoint.Y = stod(line.substr(45, 15));
			newPoint.Z = stod(line.substr(60, 15));
			l = int(line.length()) - 80;
			if (l > 0)
				newPoint.fn = line.substr(80, l);
		}
		else {
			int L = 0;
			std::vector<std::string> tokens = split(line, ' ');
			if (tokens.size() == 4) {
				newPoint.Ipt = L++;
				newPoint.pn = tokens[0];
				newPoint.X = stod(tokens[1]);
				newPoint.Y = stod(tokens[2]);
				newPoint.Z = stod(tokens[3]);
			}
			if (tokens.size() == 5) {
				newPoint.Ipt = stoi(tokens[0]);
				newPoint.pn = tokens[1];
				newPoint.X = stod(tokens[2]);
				newPoint.Y = stod(tokens[3]);
				newPoint.Z = stod(tokens[4]);
			}
			if (tokens.size() > 5) {
				newPoint.Ipt = stoi(tokens[0]);
				newPoint.pn = tokens[1];
				newPoint.X = stod(tokens[2]);
				newPoint.Y = stod(tokens[3]);
				newPoint.Z = stod(tokens[4]);
				newPoint.fn = tokens[5];
			}
		}
		//-- Check to see if it falls withing NL-boundingbox
		if ((newPoint.X >= bbx_ll[0]) && (newPoint.Y >= bbx_ll[1]) && (newPoint.X <= bbx_ur[0]) && (newPoint.Y <= bbx_ur[1])) {
			//then only add it if it is not the point that is to be removed.
			rmx = (newPoint.X - remove_point_x);
			rmx *= rmx;
			rmy = (newPoint.Y - remove_point_y);
			rmy *= rmy;
			if ((rmx + rmy) > CloseEnough2)
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

	//-- Intermediate messaging after reading.
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
		point p = point(points[i].X, points[i].Y);
		rt2.insert(std::make_pair(p, size_t(i)));
	}

	//-- Display consumed time.
	time(&end);
	std::cout << "- Kd-tree created in " << difftime(end, mid) << "s" << std::endl;
	mid = end;

	//-- Create output files, stop on error
	std::string fnOut = std::string(argv[1]);
	ReplaceStringInPlace(fnOut, ".txt", "_output.csv");
	std::ofstream of(fnOut.c_str());
	std::cout << "- Writing output to: " << fnOut.c_str() << std::endl;
	if (!of.is_open()) {
		std::cout << "Error: Cannot open outputfile: " << fnOut << std::endl;
		return 0xff;
	}

	std::string fnOutOverview = std::string(argv[1]);
	ReplaceStringInPlace(fnOutOverview, ".txt", "_output_overview.csv");
	std::ofstream ofv(fnOutOverview.c_str());
	std::cout << "- Writing output to: " << fnOutOverview.c_str() << std::endl;
	if (!ofv.is_open()) {
		std::cout << "Error: Cannot open outputfile: " << fnOutOverview << std::endl;
		return 0xff;
	}

	//-- Lookup each of the points and write it to the output files
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

		//-- if there is only one point, that point is always the search-point (LookFor). Check this by comparing its index to i (or not...).
		if (returned_values.size() < 2) {
			//if (long(returned_values[0].second) == i) {
			//	//yup.... its the same point.
			//  //        continue to do nothing....
			//}
		}
		else {
			if (ExtraDebugOutput) {
				std::cout << "Found " << std::setw(6) << returned_values.size() - 1 << " points near point " << points[i].pn << " (" << std::setw(8) << i << ")" << std::endl;
			}

			//To outputfile 1
			for (int j = 0; j < returned_values.size(); j++) {
				idx_ret = returned_values[j].second;
				if (idx_ret != i) {
					of << std::setw(10) << i << "," << std::setw(10) << returned_values.size() - 1 << "," << points[i].pn.c_str() << "," << std::setprecision(15);
					of << points[i].X << "," << std::setprecision(15) << points[i].Y << "," << points[idx_ret].pn.c_str() << "," << std::setprecision(15);
					of << points[idx_ret].X << "," << std::setprecision(15) << points[idx_ret].Y << ",";
					of << points[i].fn.c_str() << "," << points[idx_ret].fn.c_str() << std::endl;
				}

			}

			//To outputFile 2
			ofv << std::setw(10) << i << "," << points[i].pn.c_str() << "," << std::setprecision(15);
			ofv << points[i].X << "," << std::setprecision(15) << points[i].Y << "," << std::setw(10) << returned_values.size() - 1 << std::endl;

			//Progress and comments
			nCorresponding += long(returned_values.size());
		}

	}

	of.close();
	ofv.close();

	//-- Extra <CR/LF> after hash symbols
	if (!ExtraDebugOutput)
		std::cout << std::endl;

	//-- Elapsed times
	time(&end);
	std::cout << "- " << nCorresponding << " correspondences found in " << difftime(end, mid) << "s" << std::endl;
	std::cout << "- Total time: " << difftime(end, start) << "s" << std::endl;
	std::cout << "- Done...!" << std::endl;

	//-- Return
	return 0;
}

//===============================================================
// Additional / Helper functions
//===============================================================

void ReplaceStringInPlace(std::string& subject, const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

std::vector<std::string> split(const std::string &text, char sep) {
	std::vector<std::string> tokens;
	std::size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos) {
		if (end != start) {
			tokens.push_back(text.substr(start, end - start));
		}
		start = end + 1;
	}
	if (end != start) {
		tokens.push_back(text.substr(start));
	}
	return tokens;
}
