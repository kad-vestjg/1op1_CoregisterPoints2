// CoregisterPoints2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <boost/geometry.hpp>
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
	bool OnlyBest = false;
	bool ExtraDebugOutput = false;
	//
	//===================================================================================================
	//-- </defs>
	//===================================================================================================


	typedef bg::model::point<double, 2, bg::cs::cartesian> point;
	typedef bg::model::box<point> box;
	typedef std::pair<box, unsigned> value;
	time_t start, mid, end;
	time(&start);

	//--KDTREE--REPLACE-- kd_node_t *data, *root, *found;
	double best_dist;
	//-- Boundingbox for Netherlands 
	double bbx_ll[2], bbx_ur[2];
	bbx_ll[0] = 10425.156;
	bbx_ll[1] = 306846.198;
	bbx_ur[0] = 278026.090;
	bbx_ur[1] = 621876.300;

	std::cout << "+------------------------------+" << std::endl;
	std::cout << "|                              |" << std::endl;
	std::cout << "| --- CoregisterPoints <2> --- |" << std::endl;
	std::cout << "|                              |" << std::endl;
	std::cout << "+-----------------------------GV" << std::endl;

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


	//New part comes here...... ===================================
	// ----- TODO -----


	//-- Put the data in a Kd-tree
	std::cout << "- Creating Kd-tree." << std::endl;
	//--KDTREE--REPLACE-- data = (struct kd_node_t*) calloc(npoints, sizeof(struct kd_node_t));
	for (long i = 0; i < npoints; i++) {
		//--KDTREE--REPLACE-- data[i].x[0] = points[i].X;
		//--KDTREE--REPLACE-- data[i].x[1] = points[i].Y;
		//--KDTREE--REPLACE-- data[i].pn = points[i].pn;
		//--KDTREE--REPLACE-- data[i].fn = points[i].fn;
	}

	//--KDTREE--REPLACE-- root = make_tree(data, npoints, 0, 2);
	time(&end);
	std::cout << "- Kd-tree created in " << difftime(end, mid) << "s" << std::endl;
	mid = end;

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
	//--KDTREE--REPLACE-- struct kd_node_t testNode;
	double CloseEnough2 = CloseEnough*CloseEnough;
	long nCorresponding = 0;
	nLine = 0;

	//--KDTREE--REPLACE-- std::vector<kd_node_t> BestList;
	for (long i = 0; i < npoints; i++) {
		//--KDTREE--REPLACE-- testNode.x[0] = points[i].X;
		//--KDTREE--REPLACE-- testNode.x[1] = points[i].Y;
		//--KDTREE--REPLACE-- testNode.pn = points[i].pn;
		//--KDTREE--REPLACE-- testNode.fn = points[i].fn;
		visited = 0;
		//--KDTREE--REPLACE-- found = 0;
		best_dist = 1e99;
		//--KDTREE--REPLACE-- BestList.clear();
		nLine++;
		if ((nLine%nHash) == 0)
			std::cout << "#";

		//Only closest or all close points?
		if (OnlyBest) {
			//--KDTREE--REPLACE-- nearest(root, &testNode, 0, 3, &found, &best_dist);
		}
		else {
			//--KDTREE--REPLACE-- nearest_all(root, &testNode, 0, 2, &found, &best_dist, &BestList, CloseEnough2);
		}

		if (best_dist < CloseEnough2) {
			//try, catch
			//--KDTREE--REPLACE-- if (!found) {
			//--KDTREE--REPLACE-- std::cout << "ERROR: NEAREST DID NOT RETURN VALID POINT" << std::endl;
			//--KDTREE--REPLACE-- }
			//--KDTREE--REPLACE-- else {
				//Optional out to screen (debug)
				//--KDTREE--REPLACE-- if (ExtraDebugOutput) {
				//--KDTREE--REPLACE-- if (OnlyBest) {
				//--KDTREE--REPLACE-- printf("- %d -\nPoint %s (%15.3lf, %15.3lf).\nFound %s (%15.3lf, %15.3lf).\ndist %g.  Paths:\n%s\n%s\n------------------------------------------\n",
				//--KDTREE--REPLACE-- i, testNode.pn.c_str(), testNode.x[0], testNode.x[1],
				//--KDTREE--REPLACE-- found->pn.c_str(), found->x[0], found->x[1],
				//--KDTREE--REPLACE-- sqrt(best_dist), testNode.fn.c_str(), found->fn.c_str());
				//--KDTREE--REPLACE-- }
				//--KDTREE--REPLACE-- else {
				//--KDTREE--REPLACE-- printf("- %d -\nPoint %s (%15.3lf, %15.3lf).\nFound %s (%15.3lf, %15.3lf).\ndist %g.  Paths:\n%s\n%s\nnBest: %d\n------------------------------------------\n",
				//--KDTREE--REPLACE-- i, testNode.pn.c_str(), testNode.x[0], testNode.x[1],
				//--KDTREE--REPLACE-- found->pn.c_str(), found->x[0], found->x[1],
				//--KDTREE--REPLACE-- sqrt(best_dist), testNode.fn.c_str(), found->fn.c_str(), int(BestList.size()));
				//--KDTREE--REPLACE-- }
				//--KDTREE--REPLACE-- }

				//To file (always)
				//--KDTREE--REPLACE-- if (OnlyBest) {
				//--KDTREE--REPLACE-- of << testNode.pn.c_str() << "," << std::setprecision(15) << testNode.x[0] << "," << std::setprecision(15) << testNode.x[1] << ",";
				//--KDTREE--REPLACE-- of << found->pn.c_str() << "," << std::setprecision(15) << found->x[0] << "," << std::setprecision(15) << found->x[1] << ",";
				//--KDTREE--REPLACE-- of << std::setprecision(10) << sqrt(best_dist) << "," << testNode.fn.c_str() << "," << found->fn.c_str() << std::endl;
				//--KDTREE--REPLACE-- 
				//--KDTREE--REPLACE-- nCorresponding++;
				//--KDTREE--REPLACE-- }
				//--KDTREE--REPLACE-- else {
				//--KDTREE--REPLACE-- for (int j = 0; j < BestList.size(); j++) {
				//--KDTREE--REPLACE-- of << std::setw(10) << i << "," << std::setw(10) << BestList.size() << "," << testNode.pn.c_str() << "," << std::setprecision(15);
				//--KDTREE--REPLACE-- of << testNode.x[0] << "," << std::setprecision(15) << testNode.x[1] << "," << BestList[j].pn.c_str() << "," << std::setprecision(15);
				//--KDTREE--REPLACE-- of << BestList[j].x[0] << "," << std::setprecision(15) << BestList[j].x[1] << "," << std::setprecision(10) << sqrt(best_dist) << ",";
				//--KDTREE--REPLACE-- of << testNode.fn.c_str() << "," << BestList[j].fn.c_str() << std::endl;
				//--KDTREE--REPLACE-- }
				//--KDTREE--REPLACE-- 
				//--KDTREE--REPLACE-- nCorresponding += long(BestList.size());
				//--KDTREE--REPLACE-- }
				//--KDTREE--REPLACE-- }
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
