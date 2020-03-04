/*! \author		Haohan Liu
	\name		ultragrep.hpp
	\date		2019-11-01
*/
#pragma once

#include "grep_report.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <regex>
#include <mutex>
#include <map>
#include <windows.h>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

class ultragrep {
private:
	boost::system::error_code dirErrCode;

	bool verboseMode = false;
	bool emptyExtList = false;

	// regex
	regex expr;

	// threads need to launch
	vector<thread> vThread;

	// user specified file extensions
	string sExtension;
	vector<string> vExtension;
	// readable files under the current root directory
	vector<fs::path> vReadable;
	// sub folders under the root dir
	vector<fs::path> vSubfolder;

	// all useful information
	// <path, <line#, <count, line_str>>>
	vector<fs::path> vPath;
	map<fs::path, vector<GrepReport>> gpMap;

	// mutex
	mutex g_mtx;

	// time
	double threadElapsed;
public:
	unsigned findAllOccurances(string& data);
	void retrieve_extensions();

	void print_help();
	void print_report();

	void read_file(fs::path gPath);
	void search_directory(fs::path gPath);
	void search_root_directory(fs::path cPath);

	void launch_thread_cpp11();

	void run(int& argc, char* argv[]);
};