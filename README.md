# ultragrep

# Author: Eric Liu
# Nov 2019

A C++ 17 application that recursively searches a files system for text files or code files and list all of the lines that contain a given regular expression. 

The application’s interface is: 
ultragrep [-v] folder expr [extention-list]* 

Examples: 
  ultragrep . word 
  *searches all of the .txt files of the current directory for the string ‘word’  
  ultragrep –v c:\user [Gg]et .cpp.hpp.h 
  *searches the user folder for the C++ source files containing either the string “Get” or “get”. 
  
The report sorts the matches, first by file, then by line number (so you must not report results until the scan is complete). 

Verbose mode (-v) displays the name of each file scanned (as they are scanned) and each match found (as they are found). 

The extension parameter is a list of the file extensions to search.  
If no extension is provided, search files with the .txt extension. 
