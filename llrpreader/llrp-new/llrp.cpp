// llrp.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
//#include <afxwin.h>
#include <iostream>
#include <string>
 #include <sstream>
#include "CWatchDogFeeder.h"
#include "iReaderapi.h"
#include "llrp_MntServer.h"
#include "iAgent_executor.h"
#include "debug_print.h"
#include "muxserial.h"
#include "gpio.h"
#include "sqlite3.h"

using namespace std;

class base {
  public:
    base()
    { cout<<"Constructing base \n"; }
    ~base()
    { cout<<"Destructing base \n"; }
};

class derived: public base {
  public:
    derived()
    { cout<<"Constructing derived \n"; }
    ~derived()
    { cout<<"Destructing derived \n"; }
};

static void printBanner(void);
extern void process_cli_command(string cmd_string);
unsigned char test_char;

Muxserial *Ser1;
Muxserial *Ser2;

sqlite3 *rfid_db;

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(int argc, char* argv[])
{
	string input_string = "";
	//WSADATA wsaData;
	char *zErrMsg = 0;
	int rc;

#if 0
	UINT8 datalen[2];
	UINT16 dataLen;

	datalen[0] = 0x12;
	datalen[1] = 0x34;

	dataLen = (datalen[0] << 8) | datalen[1];

	printf("dataLen = %d"NL, dataLen);
#endif

	printBanner();

	 derived *d = new derived();
	 base *b = d;
	 delete d;

	//IReaderApiInitialize();
#if 0
	if (WSAStartup(0x202,&wsaData) != 0) {
		WSACleanup();
		cout << "Bummer, cannot start networking function...Bailout!!!";
		return 1;
	}
#endif

#if 0
	string word = "12384";

	string newWord;
	
	newWord.assign(word, 1, word.size());

	cout << newWord << endl;

	if (std::string::npos != newWord.find_first_not_of("0123456789"))
	{
		cout << "contains not digit" << endl;
	}
	else
	{
		cout << "contains all digit" << endl;
	}

	if (word[0] == '1')
		printf ("True"NL);
	else
		printf("False"NL);

	printf ("string size = %d"NL, word.size());

	typedef ios_base& (*Base)(ios_base&);
	Base  base = dec;

	int value;
	istringstream iss(word);
	iss >> *base >> value;
	printf("The value = %d"NL, value);

	if ( ! (istringstream(word) >> std::dec >> value))
		value = 9999;

	printf("The value = %d"NL, value);

	cout << word << endl;

#endif

	rc = sqlite3_open("rfidtag.db", &rfid_db);

	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(rfid_db));
		return(0);
	} else
	{
	    fprintf(stderr, "Opened database successfully\n");
	}

	// sqlite3_close(rfid_db);

	/* Create SQL statement */

	char *sql;
#if 0
	sql = "CREATE TABLE TAGINFO("  \
	      "TAGVALUE 	CHAR(24)     NOT NULL," \
	      "ANTID         INT     NOT NULL," \
	      "RSSI          DOUBLE," \
	      "FIRSTSEENTIME DATETIME," \
	      "LASTSEENTIME  DATETIME," \
	      "SEENCOUNT     INT," \
		  "PRIMARY KEY (" \
		  "TAGVALUE," \
		  "ANTID));";
#endif
	   sql = "CREATE TABLE COMPANY("  \
	      "ID INT PRIMARY KEY     NOT NULL," \
	      "NAME           TEXT    NOT NULL," \
	      "AGE            INT     NOT NULL," \
	      "ADDRESS        CHAR(50)," \
	      "SALARY         REAL );";

	   /* Execute SQL statement */
	   rc = sqlite3_exec(rfid_db, sql, callback, 0, &zErrMsg);

	   if( rc != SQLITE_OK ){
	      fprintf(stderr, "SQL error: %s\n", zErrMsg);
	      sqlite3_free(zErrMsg);
	   } else {
	      fprintf(stdout, "Table created successfully\n");
	   }

	   /* Create SQL statement */
	    sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
	          "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
	          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
	          "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
	          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
	          "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
	          "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
	          "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";

	    /* Execute SQL statement */
	    rc = sqlite3_exec(rfid_db, sql, callback, 0, &zErrMsg);

	    if( rc != SQLITE_OK ){
	       fprintf(stderr, "SQL error: %s\n", zErrMsg);
	       sqlite3_free(zErrMsg);
	    } else {
	       fprintf(stdout, "Records created successfully\n");
	    }


	   sqlite3_close(rfid_db);
//	LLRP_MntServer abc;

//	abc = LLRP_MntServer(1); testing calling the contructor

	if (NULL == IReaderApiInit("0.0.0.0", REGION_USA))
	{
		exit(-1);
	}
	GPIO::getInstance(0, 11);
	GPIO::getInstance(1, 74);

	if (LLRP_MntServer::initRegistry() == -1)
	{
		exit(-1);
	}

	// !!! Make sure IREADER object is created first at IReaderApiInit
	LLRP_MntServer *pLLRP_MntServer = LLRP_MntServer::getInstance(0);
	pLLRP_MntServer->run();

	IAgent_Executor *pIAgent_Executor = IAgent_Executor::getInstance();
	pIAgent_Executor->run();

	// int prio = OwTask::getPriority();
	unsigned int pid = pthread_self();    // cannot use unsigned int, pthread_self returns 64-bit value

	int  my_schedpolicy;
	sched_param  my_schedparam;
	pthread_getschedparam( pthread_self() , &my_schedpolicy, &my_schedparam );

	int prio =  my_schedparam.sched_priority;
	DBG_PRINT(DEBUG_INFO, "Main Tread PID = %d,  Priority = %d"NL, pid, prio);

	// CLI command processing loop
	OwTask::sleep(100);

	Ser1 = new Muxserial("/dev/ttyS1");
#if 0
	Ser2 = new Muxserial("/dev/ttyS2");

    // test the serial port
	Ser1->clearRcv();
	if (Ser1->getChar(&test_char) == 1)
	{
		printf("getChar = %02x\n", test_char);
	}
	else
	{
		printf("getChar fails\n");
	}
	Ser1->sendChar('a');

	if (Ser1->getChar(&test_char) == 1)
	{
		printf("getChar = %02x\n", test_char);
	}
	else
	{
		printf("getChar fails\n");
	}
	Ser1->sendChar('b');
	Ser1->getChar(&test_char);
	printf("Rev = %02x\n", test_char);
	Ser1->sendChar('c');
	Ser1->getChar(&test_char);
	printf("Rev = %02x\n", test_char);
	Ser1->sendChar('d');
	Ser1->getChar(&test_char);
	printf("Rev = %02x\n", test_char);

	Ser1->clearRcv();
#endif

	DBG_PRINT(DEBUG_INFO, NL);
	while (1)
	{
		cout << "llrp> ";
		getline(cin, input_string);
		if (input_string.size() != 0)
		{
			process_cli_command(input_string);
		}
	}
	return 0;
}

static void printBanner(void)
{
	cout << "**************************************************************" << endl;
	cout << "*                                                            *" << endl;
	cout << "*                   RFIDspan Technology                      *" << endl;
	cout << "*                   LLRP iReader-998 Device Server           *" << endl;
	cout << "*                   Build Date : " << __DATE__ "                 *"<< endl;
	cout << "*                   Build Time : " << __TIME__ "                    *"<< endl;
	cout << "*                                                            *" << endl;
	cout << "**************************************************************" << endl;
}

