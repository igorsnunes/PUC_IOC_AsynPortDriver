#include <iocsh.h>
#include <epicsExport.h>
#include <stdio.h>
#include <string.h>
#include "PortConnect.h"

PortConnect::PortConnect(const char* portName, const char * serialName) : asynPortDriver(portName, 0, 6, asynInt32Mask | asynOctetMask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynDrvUserMask, 0, 0, 1, 0, 0)
{   
   printf("Constructor\n");
   
   createParam(P_TemperatureSetPoint, asynParamFloat64, &P_TemperatureSP);
   createParam(P_TemperatureSensor1, asynParamFloat64, &P_TemperatureS1);
   createParam(P_TemperatureSensor2, asynParamFloat64, &P_TemperatureS2);
   createParam(P_TemperatureSensor3, asynParamFloat64, &P_TemperatureS3);
   createParam(P_TemperatureSensor4, asynParamFloat64, &P_TemperatureS4);
   createParam(P_SwitchState, asynParamInt32, &P_SState);
   
   asynStatus status = pasynOctetSyncIO->connect(serialName, 0, &user, NULL);
   
   if(status == asynSuccess) printf("Success: Connect to port\n");   
   else printf("Error: Connect to port");   
   
   pasynOctetSyncIO->flush(user);   
}

//Override method from AsynPortDriver
asynStatus PortConnect :: readFloat64(asynUser* pasynUser, epicsFloat64* value)
{
	printf("Read float64\n");
	asynStatus status = asynError;
	
	size_t wrote;
	
	printf("Sending request to read\n");
	
	int bytesToWrite;
	int simple=1; 		
	char * write = com.readVariable(0, pasynUser->reason, &bytesToWrite,simple);
	
	if (write[0] == 0) return asynSuccess;
		
	status = pasynOctetSyncIO->write(user, write, bytesToWrite, 5000, &wrote);
	
	if(status != asynSuccess) return status;
		
	//Read response from PUC
	//First, read the header, and after read the payload and checksum
		
	char * header;		
	char * payload;
	int size;
		
	size_t bytesRead;
	int eomReason;
		
	header = (char *) malloc(2*sizeof(char));
		
	printf("Reading\n");
	status = pasynOctetSyncIO->read(user, header, 2, 5000, &bytesRead, &eomReason);		
	if(status != asynSuccess) return status;
				
	size = com.checkSize(header[1]);
	payload = (char *) malloc((size+1)*sizeof(char));
		
	status = pasynOctetSyncIO->read(user, payload, size, 5000, &bytesRead, &eomReason);
	if(status != asynSuccess) return status;
		
		
	*value = com.readingVariable(header, payload,simple);
		
	return status;
	
}
/*
asynStatus PortConnect :: readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn)
{
	printf("readFloat64Array\n");
	asynStatus status;

	int offset = 0, address = 0, id = 0, size = 0;
	int bytesToWrite;
	size_t wrote;

	getIntegerParam(P_Address, &address);
	getIntegerParam(P_Size,    &size);
	getIntegerParam(P_Id,      &id);
	getIntegerParam(P_Offset,  &offset);

	char * write = com.readCurve(address, size, id, offset, &bytesToWrite);
	
	printf("Bytes to write: %d\n", bytesToWrite);

	status = pasynOctetSyncIO->write(user, write, bytesToWrite, 5000, &wrote);

	//Read response from PUC
	char * bufferRead;
	bufferRead = (char *) malloc(16391*sizeof(char));
	
	size_t bytesRead;
	int eomReason;

	status = pasynOctetSyncIO->read(user, bufferRead, 16391, 5000, &bytesRead, &eomReason);
	printf("Bytes read: %li", bytesRead);
	if(status != asynSuccess) return status;

	memcpy(value, com.readingCurve(bufferRead), 8192*sizeof(double));

	printf("Debug 2 values:\n");
	printf("value[100] = %f\n", value[100]);
  	printf("value[1000] = %f\n", value[1000]);

	*nIn = nElements; //Correct this
	
	fflush(stdout);
	return status;
}

//Override method from AsynPortDriver
asynStatus PortConnect :: writeFloat64Array(asynUser* pasynUser, epicsFloat64* value, size_t nElements)
{
	asynStatus status = asynError;
	printf("writeFloat64Array\n");
	size_t wrote;
	
	if(pasynUser->reason == P_Value)
	{
		int offset = 0, address = 0, id=0, size=0;
		int bytesToWrite;
		
		getIntegerParam(P_Address, &address);
		getIntegerParam(P_Size,    &size);
		getIntegerParam(P_Id,      &id);
		getIntegerParam(P_Offset, &offset);
		
		char * write = com.writeCurveBlock(address, size, id, offset, value, nElements, &bytesToWrite);
		
		printf("Bytes to write: %d\n", bytesToWrite);
		//////////////////Verify command
		status = pasynOctetSyncIO->write(user, write, bytesToWrite, 5000, &wrote);		
		//////////////////
		
		//Read response from PUC
		char * bufferRead;
		bufferRead = (char *) malloc(5*sizeof(char));
		
		size_t bytesRead;
		int eomReason;
		
		status = pasynOctetSyncIO->read(user, bufferRead, 5, 5000, &bytesRead, &eomReason);
		
		if(status != asynSuccess) return status;
		
		printf("Bytes read: %lu \n", bytesRead);
		printf("%u | %u | %u | %u | %u \n", bufferRead[0]&0xFF, bufferRead[1]&0xFF, bufferRead[2]&0xFF, bufferRead[3]&0xFF , bufferRead[4]&0xFF);			
		printf("Write: %d, Wrote: %li \n", bytesToWrite, wrote);		
		
		//TODO: Verify checksum and the response	
		
	}
	
	fflush(stdout);	
	return status;
}
*/
//Override method from AsynPortDriver
asynStatus PortConnect :: writeFloat64(asynUser* pasynUser, epicsFloat64 value)
{	
	asynStatus status = asynError;
	
	//User can modify only the value
	size_t wrote;
	printf("Data writing\n");	
	printf("Value = %f\n", value);
		
	int bytesToWrite;
	int simple = 1;
	char * write = com.writeVariable(0, sizeof(epicsFloat64), pasynUser->reason, (double) value, &bytesToWrite,simple);
		
	pasynOctetSyncIO->flush(user);
	status = pasynOctetSyncIO->write(user, write, bytesToWrite, 5000, &wrote);
		
	if(status != asynSuccess) return status;
		
	//Read response from PUC		
	char * bufferRead;
	bufferRead = (char *) malloc(5*sizeof(char));
		
	size_t bytesRead;
	int eomReason;
		
	//status = pasynOctetSyncIO->read(user, bufferRead, 5, 5000, &bytesRead, &eomReason);
		
	//if(status != asynSuccess) return status;
		
	//printf("Read: %d\n", bufferRead[2]);			
	printf("Write: %d, Wrote: %li \n", bytesToWrite, wrote);	


	return status;	
}

extern "C"{

int portConnectConfigure(const char * portName, const char * serialName)
{
   new PortConnect(portName, serialName);
   return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = {"portName",   iocshArgString};
static const iocshArg initArg1 = {"serialName", iocshArgString};

static const iocshArg * const initArgs [] = {&initArg0, &initArg1};

static const iocshFuncDef initFuncDef = {"portConnectConfigure", 2, initArgs};

static void initCallFunc(const iocshArgBuf *args)
{
   portConnectConfigure(args[0].sval, args[1].sval);
}

void portConnectRegister(void)
{
   iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(portConnectRegister);
}
