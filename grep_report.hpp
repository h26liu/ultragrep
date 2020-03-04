/*! \author		Haohan Liu
	\name		grep_report.hpp
	\date		2019-11-01
*/
#pragma once

#include <boost/filesystem.hpp>

class GrepReport {
public:
	GrepReport(boost::filesystem::path p, int n, int c, std::string s) {
		this->_filepath = p;
		this->_linenumber = n;
		this->_count = c;
		this->_lineStr = s;
	}

	boost::filesystem::path _filepath;
	int _linenumber;
	int _count;
	std::string _lineStr;
};