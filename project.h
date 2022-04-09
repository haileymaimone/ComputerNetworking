//Header details for the shared memory
//structure


#define SHMKEY 0x44000 + 01	// shared memory "key"
#define AF_DIRECTORY "/home/ftp/pub/cet/440/project/archive/"
#define AF_SUFFIX "dat"
#define PORTNUM 44001//port number defined
#define MAXHOSTNAME 64
#define BACKLOG 5
#define MAXMSGLEN 512

//Add the archieve directories
//Shared Memory data structure
struct sharedM
{
	int hour; //number of hours past in a day
	int min;  //number of mins past in a day
	int inTemp; //outside Tempature in F degrees *10
	int outTemp; //outside Tempature in F degrees *10
	int highTemp; //high temp since 0:00am for the day in F degrees*10
	int lowTemp; //low temp since 0:00am for the day in F degrees *10
	int windSpeed; //wind speed measured in mph
	int windDirect; //wind Direction 0-359 degrees
	int windChill; //wind Chill in F degrees *10
	int barometer; //barometic pressure in Hg *1000
	int inHumid; //inside humidity %
	int outHumid; //outside humidity %
	int dewPoint; //dew Point in F degrees *10	
	int totalRain; //total Rain measured in clicks
	
};//end struct
