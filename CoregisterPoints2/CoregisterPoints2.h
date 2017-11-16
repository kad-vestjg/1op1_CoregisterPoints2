#pragma once


struct SPoint {
	int Ipt;
	std::string pn;
	double X;
	double Y;
	double Z;
	std::string fn;
};


//Prototypes
void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);