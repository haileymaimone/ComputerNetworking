/*
 *  Weather Station Term Project - Phase III
 *  CET 440 - Hailey Maimone
 *
 *  Purpose: To communicate with the weather server on ryker using
 *           the wx protocol to retrieve current campus weather
 *           conditions and the daily recorded high wind speed
 *           and display the results to the user.
 *           
 */



#include <stdio.h> // for standard system I/O
#include <stdlib.h> // for exit()
#include <string.h> // for strlen(), memcpy()
#include <sys/types.h> // for system defined types
#include <sys/socket.h> // for definition of BSD socket structure
#include <netinet/in.h> // for Internet definitions and conversions
#include <arpa/inet.h> // for IP address conversion functions
#include <netdb.h> // for network database library def's
#include <unistd.h> // for userID
#include <time.h> // for date and time
#include "../project.h" // for port number

#define DEBUG 0 // for debugging


struct wd_struct {
    int hour; //number of hours past in a day
    int min;  //number of mins past in a day
    int inTemp; //outside Tempature in F degrees *10
    int outTemp; //outside Tempature in F degrees *10
    int windSpeed; //wind speed measured in mph
    int hWindSpeed; //today's highest wind speed measured in mph
    int windDirect; //wind Direction 0-359 degrees
    int windChill; //wind Chill in F degrees *10
    int barometer; //barometic pressure in Hg *1000
    int inHumid; //inside humidity %
    int outHumid; //outside humidity %
    int dewPoint; //dew Point in F degrees *10
    int totalRain; //total Rain measured in clicks
    int whHour; // hour for highest daily wind speed
    int whMin; // min for highest daily wind speed
} wd;

// read an entire line from socket (taken straight from svr2.c)
int sgetline(int sock, void *bufptr)
{
  char *bfr = (char *)bufptr;   // typecast to string ptr for use here
  char c;
  int rc, cnt=0;
  while ((rc = recv(sock, &c, 1, 0)) > 0) {
    if (c == '\n') break;               // newline
    else if ((c == 0x08) && (cnt >0))   // handle backspace
      bfr--, cnt--;
    else if (c != '\r')                 // skip carriage returns
      *bfr++=c, cnt++;                  // buffer & count regular character
  }
  *bfr = '\0';  // null terminate input string
  return (rc < 0) ? rc : cnt;
}

// function to take a string (command) to send to the server and receieve output
void retrData(char *msg) {
    char *hostp = "ryker.aet.calu.edu";
    char user[100];
    char host[100];
    char line[100];
    char buf[BUFSIZ+1];
    int sock;
    int len, i, windSpd, windHr, windMin;
    struct sockaddr_in sadd;
    struct hostent *hp;
    struct hostent *h;
    struct in_addr myIP;
    
    
    hp = gethostbyname(hostp);

    // get name of local host
    if (gethostname(host, sizeof(host)) != 0) {
        printf("gethostname() failed, exiting.\n");
        exit(1);
    }

    // get internet address info of local host
    if ((h = gethostbyname(host)) == NULL) {
        printf("gethostbyname() failed, exiting.\n");
        exit(2);
    }

    // pass copy of local host's IP to myIP struct
    myIP = *(struct in_addr *)h->h_addr;

    sprintf(user, "%i@%s\n", getuid(), host);
    
    // allocate a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    
    // setup socket address struct
    sadd.sin_family = AF_INET;
    sadd.sin_port = htons(PORTNUM);
    sadd.sin_addr = *(struct in_addr *)hp->h_addr;
    
    // make connection to remote server
    if (connect(sock, (struct sockaddr *)&sadd, sizeof sadd) < 0) {
        perror("connect");
        exit(1);
    }
    
    // send user@host string to server
    if (send(sock, user, strlen(user), 0) != strlen(user)) {
        fprintf(stderr, "%s: send error for user@host\n", user);
    }

    // send string (command) that was passed to function to the server
    if (send(sock, msg, strlen(msg), 0) != strlen(msg)) {
        fprintf(stderr, "%s send error for message request\n", user);
    }
    // if msg sent was "W", save output to struct variables
    if (strstr(msg, "W\n") != NULL) {
        while ((len = recv(sock, buf, BUFSIZ, 0)) > 0)
            sscanf(buf, "%i %i %i %i %i %i %i %i %i %i", &wd.inTemp, &wd.outTemp, &wd.windSpeed, 
            &wd.windDirect, &wd.windChill, &wd.barometer, &wd.inHumid, &wd.outHumid, 
            &wd.dewPoint, &wd.totalRain);

    // if msg sent contained an "A", save output to struct variables
    } else if (strstr(msg, "A ") != NULL) {
        wd.hWindSpeed = 0;
        wd.whHour = 00;
        wd.whMin = 00;
        while ((i = sgetline(sock, &line)) > 0) {
            sscanf(line, "%i:%i %*i %*i %i", &windHr, &windMin, &windSpd);
            if (windSpd > wd.hWindSpeed) {
                wd.hWindSpeed = windSpd;
                wd.whHour = windHr;
                wd.whMin = windMin;

            }
            #if DEBUG > 0
                printf("sgetline() returned %i |%s| %i\n", i, line, wd.hWindSpeed);
            #endif
          }
          #if DEBUG > 0
            printf("sgetline() returned %i |%s|\n", i, line);
          #endif
    } else {
        // shouldn't be here unless NAK is returned
            while ((len = recv(sock, buf, BUFSIZ, 0)) > 0)
                fwrite(buf, len, 1, stdout);
            puts("");
    }
    close(sock);
}

// function to convert and print time and date for sreen output
void convertTime(int hour, int mins, int stat) {
    // for current time
    if (stat == 1) {
         // convert 24hr time to 12hr time for time output
        if(hour > 12)
            printf("               The time is:  %02d:%02d PM\n", hour-12, mins);
        else if(hour == 0)
            printf("               The time is:  %02d:%02d AM\n", hour+12, mins);
        else if(hour == 12)
            printf("               The time is: %02d:%02d PM\n", hour, mins);
        else
            printf("               The time is:  %02d:%02d AM\n", hour, mins);
            // for wind speed recorded time
    } else if (stat == 2) {
        // convert 24hr time for recorded wind speed and print daily high
        if(hour > 12)
            printf("Record High Wind Speed:  %i mph recorded at %02d:%02d PM\n", wd.hWindSpeed, hour-12, mins);
        else if(hour == 0)
            printf("Record High Wind Speed:  %i mph recorded at %02d:%02d AM\n", wd.hWindSpeed, hour-12, mins);
        else if(hour == 12)
            printf("Record High Wind Speed:  %i mph recorded at %02d:%02d PM\n", wd.hWindSpeed, hour, mins);
        else
            printf("Record High Wind Speed:  %i mph recorded at %02d:%02d AM\n", wd.hWindSpeed, hour, mins);
    } else {
        // otherwise don't print anything
        puts("");
    }

}

int main() {
    char *dir_flag;
    short wd_dir;
    time_t clock = time(NULL);
    struct tm *mytime = localtime(&clock);
    char archive[20];
    
    // generate current date and save it to string with "A" command
    sprintf(archive, "A %4d%02d%02d\n", mytime->tm_year + 1900,
            mytime->tm_mon + 1, mytime->tm_mday);
    
    
    // compass point directions
    char *wd_dr[] = {
        "N",
        "NNE",
        "NE",
        "ENE",
        "E",
        "ESE",
        "SE",
        "SSE",
        "S",
        "SSW",
        "SW",
        "WSW",
        "W",
        "WNW",
        "NW",
        "NNW",
    };
    
    // for the date - arrays for days of the week and month
    char *days[]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    char *months[]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    
    //save date values to variables
    int wday = mytime->tm_wday;
    int month = mytime->tm_mon + 1;
    int mday = mytime->tm_mday;
    int year = mytime->tm_year + 1900;
    int hour = mytime->tm_hour;
    int mins = mytime->tm_min;
    
    // retrieve weather station data
    retrData("W\n");
    // retrieve daily high windspeed from archive file
    retrData(archive);
    
    // convert weather direction from degrees to compass point
            
    // split 360 degrees into sections and cast to integer
    wd_dir = (int)((wd.windDirect + 11.25) / 22.5) % 16;
            
    if (wd.windSpeed == 0) {
        dir_flag = "no direction (0 wind speed)"; // if wind speed is 0
    } else {
        dir_flag = wd_dr[wd_dir];
    }
    
    
    // OUTPUT FOR DATE AND TIME
    printf("\n");
    printf("--------------------------------------------------\n");
    printf("                  WELCOME TO THE\n");
    printf("               WEATHERLINK STATION!\n");
    printf("--------------------------------------------------\n\n");
    printf("            Hello user %i!\n", getuid());
            
    printf("       Today is %s - %s %d, %d \n", days[mytime->tm_wday], months[mytime->tm_mon],
           mytime->tm_mday, mytime->tm_year+1900);
    convertTime(hour, mins, 1);

    // OUTPUT FOR WEATHER CONDITIONS
    printf("\n--------------------------------------------------\n");
    printf("              CalU Weather Conditions\n");
    printf("--------------------------------------------------\n");
    printf("    Inside Temperature:  %3.1f F\n", (float)wd.inTemp / 10);
    printf("   Outside Temperature:  %3.1f F\n", (float)wd.outTemp/10);
    printf("        Wind Direction:  %s\n", dir_flag);
    printf("            Wind Chill:  %i F\n", wd.windChill/10);
    printf("            Wind Speed:  %i mph\n", wd.windSpeed);
    printf("   Barometric Pressure:  %4.3f Hg\n", (float)wd.barometer/1000);
    printf("       Inside Humidity:  %i %\n", wd.inHumid);
    printf("      Outside Humidity:  %i %\n", wd.outHumid);
    printf("             Dew Point:  %i F\n", wd.dewPoint/10);
    printf("        Total Rainfall:  %3.2f\n", (float)wd.totalRain);
    convertTime(wd.whHour, wd.whMin, 2);
		

    return 0;
}
