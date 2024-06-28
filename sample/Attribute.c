#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "IotClient.h"
#include "IotClient_ErrCode.h"

int main(int argc, char** argv)
{

	char* authCode = "adf022d0824dc28c";
	char* userId = "";
    char* userPw = "";

	char* siteId = "C000000037";
	char* thingName = "999999.1324";

	IotClient* client = createIotClient(authCode, userId, userPw, siteId, thingName);
	if(client != NULL)
	{
		if( client->connect() == ERR_OK )
		{ // connected

			char* msgCode = NULL;
			char* baseStr = "{\"sample1\": %d, \"sample2\": %d, \"sample3\": %d}";

			char dataStr[LONG_STR_LEN] = {0, };

			while(1) {
				int value1 = rand() % 31 + 10;
				int value2 = rand() % 31 + 10;
				int value3 = rand() % 31 + 10;
			
				sprintf(dataStr, baseStr, value1, value2, value3);
				client->sendAttributes(msgCode, dataStr);
				
				// 10 sec
				sleep(10);
			}

		}

		destroyIotClient(client);
	}

	return 0;
}
