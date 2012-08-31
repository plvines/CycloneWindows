#include <stdlib.h>
#include <sstream>
#include "PDS.h"
#include "Table.h"
#include "cyclone.h"
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "utilities.h"
#include <string>

using namespace std;

// PRE: filename contains the name of a file with tables
// POST: the file has been read and its tables constructed into Table
// objects in the tables array
string readInput(string filename){
  ifstream inputStream(filename.c_str());
  string input = "";

  // Try to open the file
  if (inputStream.is_open()){
    
    char * temp = new char[LINESIZE];
    
    // Read the file into the input string
    while(inputStream){
      int oldLength = input.length();
      inputStream.getline(temp, LINESIZE);
      input += temp;
      while (input.length() - oldLength == LINESIZE){
	oldLength = input.length();
	inputStream.getline(temp, LINESIZE);
	input += temp;
      }
      input += "\n";
    }
    input += '\0';
    delete temp;
  }
  else{
    cout << "Input File: *" << filename << "* Not Found" << endl;
  }
  return input;
}

// PRE: tables abd pds are defined as representing the user's choice
// (or lack thereof if they are both false) of input type. Num_States
// is the number of states the user has entered
// POST: either tables or pds is true, and num_states is >= MIN_STATES
void promptUser(bool & tables, bool & pds){
      // If table/PDS is unspecified:
      while (!tables && !pds){
	cout << "Is input in table (t) or PDS (p) form?" << endl;
	string tempTables = "";
	cin >> tempTables;
	tables = (tempTables[0] == 't');
	pds = (tempTables[0] == 'p');
      }
}

// Main function, takes in arguments for running cyclone. If the
// minimum arguments are not provided then the user is prompted for them.
int main(int argc, char * argcv[]){

  string filename;
  string trajFile;
  string outputFile;
  int cores = 0;
  int numTraj = 0;
  int numRandomSteps = 0;
  int numRandomRuns  = 0;
  bool tables = true, pds = false, writeFile = false, edges = false, verbose = false, traj = false, multitraj = false, randomUpdate= false;
  int state_check;
  char ** multitrajFiles;
  char ** multitrajOutfiles;

  // No arguments
  if (argc == 1){
    cout << "Enter input filename: " << endl;
    cin >> filename;
    
    promptUser(tables, pds);
  }
  // One or more arguments in addition to filename
  else {    
    string temp = argcv[1];

    if (temp.compare("-clearSHM") == 0){
      return 0;
    }
    else{
      filename = temp;
    }

    for (int i = START_OF_ARGS; i < argc; i++){
      temp = argcv[i];

      // Check for -v argument
      if (temp.compare("-v") == 0){
	verbose = true;
      }

      // Check for -edges argument
      else if (temp.compare("-edges") == 0){
	edges = true;
      }

      // Check for -traj argument
      else if (temp.compare("-traj") == 0){
	traj = true;
	trajFile = argcv[i + 1];
	i++;
      }

      // Check for -random argument
      else if (temp.compare("-random") == 0 || temp.compare("-r") == 0){
	randomUpdate = true;
	numRandomRuns = atoi(argcv[i+1]);
	numRandomSteps = atoi(argcv[i+2]);
	i+=2;
      }

      else if (temp.compare("-multitraj") == 0){
	multitraj = true;
	numTraj = atoi(argcv[i+1]);
	i++;
	multitrajFiles = new char*[numTraj];
	for (int k = 0; k < numTraj; k++){
	  multitrajFiles[k] = (argcv[i+1]);
	  i++;
	}
      }

      // Check for -c argument
      else if (temp.compare("-c") == 0){
	cores = atoi(argcv[i+1]);
	i++;
      }

      // Check for a file write arguments
      else if (temp.compare("-f") == 0){
	  writeFile = true;
	if (multitraj){
	  multitrajOutfiles = new char*[numTraj];
	  for (int k = 0; k < numTraj; k++){
	    multitrajOutfiles[k] = argcv[i + 1];
	    i++;
	  }
	}
	else{
	  outputFile = argcv[i + 1];
	  i++;
	}
      }
    }
      promptUser(tables, pds);    
  }
  

  // Run cyclone
  string input = readInput(filename);
  if (input.length() > 0){
    
    Cyclone * cyc =  new Cyclone(filename, tables);
    if (edges){
      cyc->generateEdges(writeFile, outputFile, verbose);
    }
    else if (traj){
      if (randomUpdate){
	cyc->generateRandomTrajectory(writeFile, outputFile, verbose, trajFile, filename, numRandomRuns, numRandomSteps);
      }
      else {
	cyc->generateTrajectory(writeFile, outputFile, verbose, trajFile, filename);
      }
    }
    else if (multitraj){
      for (int i = 0; i < numTraj; i++){
	if (writeFile){
	  if (randomUpdate){
	    cyc->generateRandomTrajectory(writeFile, multitrajOutfiles[i], verbose, multitrajFiles[i], filename, numRandomRuns, numRandomSteps);
	  }
	  else {
	    cyc->generateTrajectory(writeFile, outputFile, verbose, trajFile, filename);
	  }
	}else{
	  if (randomUpdate){
	    cyc->generateRandomTrajectory(writeFile, outputFile, verbose, multitrajFiles[i], filename, numRandomRuns, numRandomSteps);
	  }
	  else {
	    cyc->generateTrajectory(writeFile, outputFile, verbose, trajFile, filename);
	  }
	}
      }
    }
    else {
      if (cores > 0){
	cyc->run(writeFile, outputFile, cores, verbose);
      }
      else {
	cyc->run(writeFile, outputFile, 1, verbose);
      }
    }

   delete cyc;
	
  }
  return 0;
}
