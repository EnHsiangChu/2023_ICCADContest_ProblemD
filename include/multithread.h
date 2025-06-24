#pragma once
#ifndef _MULTITHREAD_H_
#define _MULTITHREAD_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <cstdlib> 
#include <ctime>
#include <set>
#include <thread>


using namespace std;

#include "structure.h"
#include "update.h"
#include "search.h"
#include "dddetail.h"


void legalization(Plane**result, int thread_id, string case_file_name, int t);
vector<Trantile> transform_function(Plane* &plane_result);



#endif // !_MULTITHREAD_H_

