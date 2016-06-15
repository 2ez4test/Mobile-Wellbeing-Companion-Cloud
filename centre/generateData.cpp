#include <cstdio>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>

using namespace std;

vector <string> getData(const string& str) {
	string temp;
	vector<string> data;
	for (string::const_iterator iter = str.begin(); iter != str.end(); iter++) {
		if (*iter == ',') {
			if (!temp.empty()) {
				data.push_back(temp);
				temp.clear();
			}
		} else {
			temp += *iter;
		}
	}
	data.push_back(temp);
	return data;
}

//int marks[2] = {0,0};
ifstream fileObj[2];

float getDataBasedOnTimeStamp(int type, string filename, long chosenTime) {
	//ifstream file(filename);
	//ifstream& file = file1;
	string str = "";
	long time_stamp = 0;
	int data = 0;
	int mark = 0;
	/*for (int i = 0; i < marks[type]; i++) {
		getline(file,str);
	}*/
	while(1) {
	  while(getline(fileObj[type],str))
	  {	
		//mark++;
		vector<string>data = getData(str);
                time_stamp = stoi(data[0]);
                if(time_stamp == chosenTime) {
			//file.close();marks[type] = mark; 
			return stof(data[1]); }
		else if(time_stamp > chosenTime) {
			//file.close();
			return -999.0; }
		
	  }
	if (!fileObj[type].eof()) break; 
        fileObj[type].clear();
	}
	return -999;
}

int main () {
	fileObj[0].open("/home/pi/heart_rate_monitoring/heartrate_log.txt");
	fileObj[1].open("/home/pi/temperature/temp_log.txt");
	
	ifstream file ("/home/pi/motion/motion_log.txt");
	string str = "";		
	long time_stamp = 0;
	string activity = "";
	ofstream filelog;
  	filelog.open ("/home/pi/centre/log.txt");
	 vector<string>data;
	while (1) {
	  while (getline(file,str))
	  {
		data = getData(str);
		time_stamp = stoi(data[0]);
		activity = data[1];
		//printf("%d\n", time_stamp);
		float heart_rate = getDataBasedOnTimeStamp(0, "/home/pi/heart_rate_monitoring/heartrate_log.txt", time_stamp);
		float temperature = getDataBasedOnTimeStamp(1, "/home/pi/temperature/temp_log.txt",time_stamp);
		if(heart_rate != -999.0 && temperature != - 999.0) {
			printf(".");
			filelog << time_stamp << "," << activity << "," << heart_rate << "," << temperature << "\n";		
			filelog.flush();
		}
		
		//this_thread::sleep_for (chrono::milliseconds(20));
	  }
	if (!file.eof()) break;
	file.clear();
	this_thread::sleep_for (chrono::milliseconds(1000));
	}
	filelog.close();
	file.close();
}
