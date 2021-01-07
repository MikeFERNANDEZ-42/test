#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <stdint.h>
#include <map>
#include <vector>
#include <list>
#include <iterator>
#include <utility>
#include <algorithm>
#include <exception>
#include <fcntl.h>
#include <ctime>
#include <sstream>
#include <sys/wait.h>

# define RESET   		"\033[0m"
# define BLACK   		"\033[30m"				/* Black */
# define RED     		"\033[31m"				/* Red */
# define GREEN   		"\033[32m"				/* Green */
# define YELLOW  		"\033[33m"				/* Yellow */
# define BLUE    		"\033[34m"				/* Blue */
# define MAGENTA 		"\033[35m"				/* Magenta */
# define CYAN    		"\033[36m"				/* Cyan */
# define WHITE   		"\033[37m"				/* White */
# define BOLDBLACK   	"\033[1m\033[30m"		/* Bold Black */
# define BOLDRED     	"\033[1m\033[31m"		/* Bold Red */
# define BOLDGREEN   	"\033[1m\033[32m"		/* Bold Green */
# define BOLDYELLOW  	"\033[1m\033[33m"		/* Bold Yellow */
# define BOLDBLUE    	"\033[1m\033[34m"		/* Bold Blue */
# define BOLDMAGENTA 	"\033[1m\033[35m"		/* Bold Magenta */
# define BOLDCYAN    	"\033[1m\033[36m"		/* Bold Cyan */
# define BOLDWHITE   	"\033[1m\033[37m"		/* Bold White */

#define TRUE   1  
#define FALSE  0  
#define PORT 8080  

std::vector<std::string>				split(std::string str, std::string separator);
int										searchInVec(std::string find, std::vector<std::string> entry);
std::string								fileToString(std::string file);
std::string								convertInSpaces(std::string line);
std::string								cleanLine(std::string	&line);
int										fileIsOpenable(std::string path);
bool									folderIsOpenable(std::string repos);
std::vector<std::string>				listFilesInFolder(std::string repos);
bool 									inArray(std::string arr[], std::string needle);
std::string								getTime();
char 									**mergeArrays(char **tab1, char **tab2, int freeOption);
long long unsigned  int					getSizeFileBits(std::string filename);
char									*ft_itoa(long int n);
std::string								NumberToString(long int Number);
std::string 							replaceStr(std::string line, std::string s1, std::string s2);
std::string 							replaceStrStart(std::string line, std::string s1, std::string s2);
std::string								getfilename(std::string uri);
void									CleanBody(std::string request);

#endif
