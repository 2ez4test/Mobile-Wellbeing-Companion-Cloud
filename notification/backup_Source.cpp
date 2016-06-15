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

static const int AGE = 23;
static const string GENDER = "M";
static const int SUSTAIN_TIME_LIMIT = 30;
static const int TEMP_LOWER_BOUND = 10;
static const int TEMP_UPPER_BOUND = 30;

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
float calibrateRHR() {
	float RHR = 0;
	ifstream file ("/home/pi/centre/log.txt");
	string str = "";
	int i = 0;
	long time_stamp = 0;
	string activity = "";
	float heart_rate, temperature;
	getline(file,str);
	vector<string>data = getData(str);
	time_stamp = stoi(data[0]);
	long startTime = time_stamp;
	long duration = 0;
	while(duration <= 60) {
	  while (getline(file,str))
	  {
		data = getData(str);
		time_stamp = stoi(data[0]);
		activity = data[1];
		heart_rate = stof(data[2]);
		temperature = stof(data[3]);
		// cout << time_stamp << "," << activity << "," << heart_rate << "," << temperature << "\n";
		if(activity == "Lying") {
			RHR += heart_rate;
			i++;
		}
		duration = time_stamp - startTime;
	  }
	  if (!file.eof()) break;
          file.clear();
	}
	file.close();
	printf("RHR = %d\n",(int) (RHR/i));
	return (int) (RHR/i);
}
float calibrateMHR() {
	if(GENDER == "M")
		return 220 - AGE;
	else
		return 226 - AGE;
}
vector<float*> calculateHRZ(float RHR, float MHR) {
	vector<float*> HRZ;
	for (int i = 0; i < 5; i++) {
		float* boundary = new float[2];
		float lowerAlpha = 0.5 + 0.1*i;
		float upperAlpha = 0.6 + 0.1*i;
		boundary[0] = lowerAlpha * MHR + (1 - lowerAlpha) * RHR;
		boundary[1] = upperAlpha * MHR + (1 - upperAlpha) * RHR;
		// cout << boundary[0] << "," << boundary[1] << "\n";
		HRZ.push_back(boundary);
	}
	return HRZ;
}

long time_stamp_start_lowheartrate, time_stamp_start_highheartrate, time_stamp_start_lowtemp, time_stamp_start_hightemp = 0;
int count_lowheartrate, count_highheartrate, count_lowtemp, count_hightemp = 0;

void analyzeHeartRate(long time_stamp, string activity, float heart_rate, float RHR, float* zone1) {
	bool isNormal = false;
	int lowerBoundHeartRate, upperBoundHeartRate;
	// cout << time_stamp << "," << activity << "," << heart_rate << "," << temperature << "\n";
	if(activity == "Lying") {
		lowerBoundHeartRate = 0.8*RHR; upperBoundHeartRate = zone1[0]; }
	else if(activity == "Sitting" || activity == "Standing") {
		lowerBoundHeartRate = 1.5*RHR; upperBoundHeartRate = (zone1[0] + zone1[1])/2; }
	else if(activity == "Walking") {
		lowerBoundHeartRate = zone1[0]; upperBoundHeartRate = zone1[1]; }
	else {
		lowerBoundHeartRate = 0; upperBoundHeartRate = 220; }

	if(heart_rate < lowerBoundHeartRate) {
		count_lowheartrate++;
		if(count_lowheartrate == 1) {
			time_stamp_start_lowheartrate = time_stamp;
			isNormal = true;
		} else if (count_lowheartrate == 10 && time_stamp - time_stamp_start_lowheartrate <= SUSTAIN_TIME_LIMIT) {
			cout << "\nAlert: User's current heart rate is low! \n";
			isNormal = false;
			// Require PNP4Nagios to send notification
		} else if(time_stamp - time_stamp_start_lowheartrate > SUSTAIN_TIME_LIMIT) {
			count_lowheartrate = 0;
			isNormal = true;
		} else {
			isNormal = true;
		}
	}

	else if(heart_rate > upperBoundHeartRate) {
		count_highheartrate++;
		if(count_highheartrate == 1) {
			time_stamp_start_highheartrate = time_stamp;
			isNormal = true;
		} else if (count_highheartrate == 10 && time_stamp - time_stamp_start_highheartrate <= SUSTAIN_TIME_LIMIT) {
			cout << "\nAlert: User's current heart rate is high! \n";
			isNormal = false;
			// Require PNP4Nagios to send notification
		} else if(time_stamp - time_stamp_start_highheartrate > SUSTAIN_TIME_LIMIT) {
			count_highheartrate = 0;
			isNormal = true;
		} else {
			isNormal = true;
		}
	}
	else {
		isNormal = true; }
	if(isNormal) cout << ".";
}
void analyzeTemperature(long time_stamp, float temp) {
	bool isNormal = false;
	if(temp < TEMP_LOWER_BOUND) {
		count_lowtemp++;
		if(count_lowtemp == 1) {
			time_stamp_start_lowtemp = time_stamp;
			isNormal = true;
		} else if (count_lowtemp == 10 && time_stamp - time_stamp_start_lowtemp <= SUSTAIN_TIME_LIMIT) {
			cout << "\nAlert: Room temperature is low! \n";
System("./send_notification.pl -t -r 2ez4test@gmail.com -f html -u -T \"Temperature\" -C \"WARNING: Room temperature is low\" -L \"http://192.168.0.101/pnp4nagios/index.php/graph?host=localhost&srv=Temperature&view=0\"");
");
			isNormal = false;
			// Require PNP4Nagios to send notification
		} else if(time_stamp - time_stamp_start_lowtemp > SUSTAIN_TIME_LIMIT) {
			count_lowtemp = 0;
			isNormal = true;
		} else {
			isNormal = true;
		}
	}

	else if(temp > TEMP_UPPER_BOUND) {
		count_hightemp++;
		if(count_hightemp == 1) {
			time_stamp_start_hightemp = time_stamp;
			isNormal = true;
		} else if (count_hightemp == 10 && time_stamp - time_stamp_start_hightemp <= SUSTAIN_TIME_LIMIT) {
			cout << "\nAlert: Room temperature is high! \n";
			isNormal = false;
char* temp = ""./send_notification.pl -t -r 2ez4test@gmail.com -f html -u -T \"Temperature\" -D \"Temperature: \" -C \"WARNING: Room temperature is high\" -L \"http://192.168.0.101/pnp4nagios/index.php/graph?host=localhost&srv=Temperature&view=0\"");
System("./send_notification.pl -t -r 2ez4test@gmail.com -f html -u -T \"Temperature\" -D \"Ttp://192.168.0.101/pnp4nagios/index.php/graph?host=localhost&srv=Temperature&view=0\"");
			// Require PNP4Nagios to send notification
		} else if(time_stamp - time_stamp_start_hightemp > SUSTAIN_TIME_LIMIT) {
			count_hightemp = 0;
			isNormal = true;
		} else {
			isNormal = true;
		}
	}
	else {
		isNormal = true; }
	if(isNormal) cout << ".";
}
void drawGraph(long time_stamp, string activity, float heart_rate, float temperature) {
	ofstream filelog1;//,filelog2;
        filelog1.open("/usr/local/pnp4nagios/var/spool/heart_rate_" + to_string(time_stamp));
	//filelog2.open("/usr/local/pnp4nagios/var/spool/temp_" + to_string(time_stamp));
	filelog1 << "DATATYPE::SERVICEPERFDATA\tTIMET::" << to_string(time_stamp) << "\tHOSTNAME::localhost\tSERVICEDESC::Heart Rate\tSERVICEPERFDATA::User is " << activity << "=" << to_string(heart_rate) <<"\tSERVICECHECKCOMMAND::check_heart\tHOSTSTATE::UP\tHOSTSTATETYPE::HARD\tSERVICESTATE::0\tSERVICESTATETYPE::1\n";
	filelog1 << "DATATYPE::SERVICEPERFDATA\tTIMET::" << to_string(time_stamp) << "\tHOSTNAME::localhost\tSERVICEDESC::Temperature\tSERVICEPERFDATA::User is " << activity<< "=" << to_string(temperature) <<"\tSERVICECHECKCOMMAND::check_temp\tHOSTSTATE::UP\tHOSTSTATETYPE::HARD\tSERVICESTATE::0\tSERVICESTATETYPE::1";
	filelog1.close();
	//filelog2.close();
	}
string findMainActivity(int* motionCollector) {
	int index = 0;
	for (int i = 1; i < 5; i++) {
		if(motionCollector[i] > motionCollector[index]) {
			index = i;
		}
	}
	switch(index) {
		case 0: return "Walking";
		case 1: return "Sitting";
		case 2: return "Standing";
		case 3: return "Lying";
		case 4: return "Transitioning";
	}
}
int main () {
	float RHR = calibrateRHR();
	float MHR = calibrateMHR();
	vector<float*> HRZ = calculateHRZ(RHR, MHR);
	ifstream file ("/home/pi/centre/log.txt");
	string str = "";		
	long time_stamp = 0;
	string activity = "";
	float heart_rate, temperature;
	vector<string>data;
	getline(file,str);	
	data = getData(str);
	time_stamp = stoi(data[0]);
	long start_time = time_stamp;
	long duration = 0;
	int motionCollector [5] = {0, 0, 0, 0, 0};
	while(1) {
	  while (getline(file,str))
	  {
		data = getData(str);
		time_stamp = stoi(data[0]);
		activity = data[1];
		heart_rate = stof(data[2]);
		temperature = stof(data[3]);
		analyzeHeartRate(time_stamp, activity, heart_rate, RHR, HRZ[0]);
		analyzeTemperature(time_stamp, temperature);
		duration = time_stamp - start_time;
		if(activity.compare("Walking") == 0) motionCollector[0]++;
		else if(activity.compare("Sitting") == 0) motionCollector[1]++;
 		else if(activity.compare("Standing") == 0) motionCollector[2]++;
 		else if(activity.compare("Lying") == 0) motionCollector[3]++;
		else motionCollector[4]++;

		if(duration >= 30){
			drawGraph(time_stamp, findMainActivity(motionCollector), heart_rate, temperature);
			start_time = time_stamp;
			for(int j = 0; j < 5; j++) 
				motionCollector[j] = 0;
		}
		//this_thread::sleep_for (chrono::milliseconds(20));
	  }
	  if (!file.eof()) break;
          file.clear();
	  this_thread::sleep_for (chrono::milliseconds(1000));
	}
	file.close();
}
