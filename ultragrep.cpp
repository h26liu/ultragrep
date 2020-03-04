/*! \author		Haohan Liu
	\name		ultragrep.cpp
	\date		2019-11-01
*/
#include "grep_report.hpp"
#include "ultragrep.hpp"

// find occurances of given data
unsigned ultragrep::findAllOccurances(string& data)
{
	// Occurance count
	unsigned count = 0;

	// Finding all the match. 
	for (sregex_iterator it = sregex_iterator(data.begin(), data.end(), expr);
		it != sregex_iterator(); it++) {
		count++;
	}

	return count;
}

void ultragrep::retrieve_extensions() {
	stringstream ss(sExtension);
	string s;

	while (getline(ss, s, '.')) {
		if (!s.empty())
			vExtension.push_back("." + s);
	}
}

// show help information
void ultragrep::print_help() {
	cout << "UltraGrep 1.0.0 (c) Haohan Liu 2019\nUsage: ultragrep [-v] folder regex [.ext]*" << endl;
}

// show grep report
void ultragrep::print_report() {
	// remove duplcates from path vector
	sort(vPath.begin(), vPath.end());
	vPath.erase(unique(vPath.begin(), vPath.end()), vPath.end());

	cout << "Grep Report:" << endl;

	int _matches = 0;

	for (size_t i = 0; i < vPath.size(); i++)
	{
		//
		// line#, count, line_str
		//
		cout << "\n" << vPath[i] << endl;
		for (GrepReport _gp : gpMap[vPath[i]])
		{
			_matches += _gp._count;

			cout << "[" << _gp._linenumber << ":" << _gp._count << "] " << _gp._lineStr << endl;
		}
	}

	cout << "\nFiles with matches = " << vPath.size() << endl;
	cout << "Total number of matches = " << _matches << endl;
	cout << "Scan Completed in " << threadElapsed << " seconds." << endl;
}

// read readable file
void ultragrep::read_file(fs::path gPath) {
	// read the file
	if (verboseMode) {
		lock_guard<mutex> lk(g_mtx);
		cout << "Grepping: " << gPath << "\n" << endl;
	}

	// process the file content
	lock_guard<mutex> lk(g_mtx);

	string line;
	ifstream ifs(gPath.c_str());
	if (ifs) {
		unsigned lineCount = 0;
		while (getline(ifs, line)) {
			lineCount++;
			unsigned occurances = findAllOccurances(line);

			if (occurances > 0) {
				// store information to use later
				GrepReport _gp(gPath, lineCount, occurances, line);
				gpMap[gPath].push_back(_gp);
				vPath.push_back(gPath);

				if (verboseMode)
					cout << "Matched " << occurances << ": " << gPath.string() << " [" << lineCount << "] " << line << endl;
			}
		}
	}
	else {
		cerr << "Couldn't open " << gPath.string() << " for reading\n";
		//EXIT_FAILURE;
	}
}

// search given directory recursively
void ultragrep::search_directory(fs::path gPath) {
	if (verboseMode) {
		lock_guard<mutex> lk(g_mtx);
		cout << "Scanning: " << gPath << "\n" << endl;
	}

	try {
		for (fs::recursive_directory_iterator end, dir(gPath);
			dir != end; ++dir) {
			if (fs::is_regular_file(*dir, dirErrCode)) {
				fs::path ext = dir->path().filename().extension();

				if (emptyExtList && ext == ".txt") {
					read_file(*dir);
				}
				// with specific extension
				else if (!emptyExtList && std::find(vExtension.begin(), vExtension.end(), ext.string()) != vExtension.end()) {
					read_file(*dir);
				}
			}
			else if (fs::is_directory(*dir, dirErrCode)) {
				if (verboseMode) {
					lock_guard<mutex> lk(g_mtx);
					cout << "Scanning: " << dir->path() << "\n" << endl;
				}
			}
		}
	}
	catch (exception & e) {
		cout << e.what() << endl;
	}
}

// search root directory
void ultragrep::search_root_directory(fs::path cPath) {
	emptyExtList = vExtension.empty();

	if (!emptyExtList)
		retrieve_extensions();

	for (fs::directory_iterator end, dir(cPath);
		dir != end; ++dir) {
		if (fs::is_directory(*dir, dirErrCode)) { // dir is pointing to directory
			vSubfolder.push_back(dir->path());
		}
		else if (fs::is_regular_file(*dir, dirErrCode)) { // dir pointing to a readable file
			fs::path ext = dir->path().filename().extension();

			// without specific extension
			if (emptyExtList && ext.string() == ".txt") {
				vReadable.push_back(dir->path());
			}
			// with specific extension
			else if (!emptyExtList && std::find(vExtension.begin(), vExtension.end(), ext.string()) != vExtension.end()) {
				vReadable.push_back(*dir);
			}
		}
	}
}

// launch threads in thread pool in cpp11 style
void ultragrep::launch_thread_cpp11() {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	LARGE_INTEGER threadStart, threadStop;
	QueryPerformanceCounter(&threadStart);


	if (!vSubfolder.empty()) {
		for (fs::path& subfolder : vSubfolder)
		{
			vThread.push_back(std::thread(&ultragrep::search_directory, this, subfolder));
		}
	}

	if (!vReadable.empty()) {
		for (fs::path& file : vReadable)
		{
			vThread.push_back(std::thread(&ultragrep::read_file, this, file));
		}
	}

	// launch threads
	/** Wait for all the threads in vector to join **/
	// iterate over the thread vector
	while (true) {
		bool threadNotFinished = false;

		for (std::thread& th : vThread)
		{
			// if thread Object is Joinable then Join that thread.
			if (th.joinable()) {
				threadNotFinished = true;
				th.join();
			}
		}

		if (!threadNotFinished) {
			QueryPerformanceCounter(&threadStop);
			threadElapsed = (threadStop.QuadPart - threadStart.QuadPart) / double(frequency.QuadPart);

			print_report();
			break;
		}
	}
}

void ultragrep::run(int& argc, char* argv[]) {
	if (argc < 3) { // only one/two args
		print_help();
	}
	else if (argc == 3) { // three args
		if (fs::is_directory(argv[1], dirErrCode)) {
			expr = argv[2];
			verboseMode = false;
			search_root_directory(argv[1]);
			launch_thread_cpp11();
		}
	}
	else if (argc == 4) {
		string str(argv[1]);

		if (str.find("-v") != std::string::npos) { // verbose mode, no extension list
			expr = argv[3];
			verboseMode = true;
			search_root_directory(argv[2]);
			launch_thread_cpp11();
		}
		else { // extension list provided, not in verbose mode
			sExtension = argv[3];
			retrieve_extensions();
			expr = argv[2];
			search_root_directory(argv[1]);
			launch_thread_cpp11();
		}
	}
	else if (argc == 5) {
		string str(argv[1]);

		if (str.find("-v") != std::string::npos && fs::is_directory(argv[2], dirErrCode)) {
			sExtension = argv[4];
			retrieve_extensions();
			verboseMode = true;
			expr = argv[3];
			search_root_directory(argv[2]);
			launch_thread_cpp11();
		}
		else {
			cerr << "Error: Invalid parameter." << endl;
			return;
		}
	}
}