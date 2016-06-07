 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <stdlib.h>

#include "MobiCoreDriverApi.h"
#include "tlcApisec.h"

#define LOG_TAG "TLC SPI"
#include "log.h"

static const uint32_t DEVICE_ID = MC_DEVICE_ID_DEFAULT;
dapc_tciMessage_t *tci;
dapc_tciMessage_t *dci;
mcSessionHandle_t spi_sessionHandle;
//mcSessionHandle_t spi_sessionHandle1;

// -------------------------------------------------------------
static mcResult_t executeCmd(uint32_t *index, uint32_t *result, tciCommandId_t cmd)
{
	mcResult_t  ret;    

	if(NULL == tci) 
	{
	    printf("TCI has not been set up properly - exiting\n");
	    return MC_DRV_ERR_NO_FREE_MEMORY;
	}

	tci->cmd.header.commandId = cmd;
	tci->cmd.len = 0;
	tci->cmd.respLen = 0;
	tci->index= *index; 

	printf("Preparing command message in TCI\n");

	printf("Notifying the trustlet\n");
	ret = mcNotify(&sessionHandle);

	if (MC_DRV_OK != ret)
	{
	    printf("Notify failed: %d\n", ret);
	    goto exit;
	}

	printf("Waiting for the Trustlet response\n");
	ret = mcWaitNotification(&sessionHandle, -1);

	if (MC_DRV_OK != ret) 
	{
	    printf("Wait for response notification failed: 0x%x\n", ret);
	    goto exit;
	}

	*result = tci->result;

	printf("Verifying that the Trustlet sent a response.\n");
	if (RSP_ID(cmd) != tci->rsp.header.responseId) 
	{
	    printf("Trustlet did not send a response: %d\n",
	        tci->rsp.header.responseId);
	    ret = MC_DRV_ERR_INVALID_RESPONSE;
	    goto exit;
	}

	if (RET_OK != tci->rsp.header.returnCode) 
	{
	    printf("Trustlet did not send a valid return code: %d\n",
	        tci->rsp.header.returnCode);
	    ret = tci->rsp.header.returnCode;
	}

	exit:
	return ret;
}

// -------------------------------------------------------------
static size_t getFileContent(
    const char* pPath,
    uint8_t** ppContent)
{
	FILE*   pStream;
	long    filesize;
	uint8_t* content = NULL;

	/*
	 * The stat function is not used (not available in WinCE).
	 */

	/* Open the file */
	pStream = fopen(pPath, "rb");
	if (pStream == NULL)
	{
		fprintf(stderr, "Error: Cannot open file: %s.\n", pPath);
		return 0;
	}

	if (fseek(pStream, 0L, SEEK_END) != 0)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
		goto error;
	}

	filesize = ftell(pStream);
	if (filesize < 0)
	{
		fprintf(stderr, "Error: Cannot get the file size: %s.\n", pPath);
		goto error;
	}

	if (filesize == 0)
	{
		fprintf(stderr, "Error: Empty file: %s.\n", pPath);
		goto error;
	}

	/* Set the file pointer at the beginning of the file */
	if (fseek(pStream, 0L, SEEK_SET) != 0)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
		goto error;
	}

	/* Allocate a buffer for the content */
	content = (uint8_t*)malloc(filesize);
	if (content == NULL)
	{
		fprintf(stderr, "Error: Cannot read file: Out of memory.\n");
		goto error;
	}

	/* Read data from the file into the buffer */
	if (fread(content, (size_t)filesize, 1, pStream) != 1)
	{
		fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
		goto error;
	}

	/* Close the file */
	fclose(pStream);
	*ppContent = content;

	/* Return number of bytes read */
	return (size_t)filesize;

	error:
	if (content  != NULL)
	{
		free(content);
	}
	fclose(pStream);
	return 0;

}

// -------------------------------------------------------------
mcResult_t tlcOpen(void)
{
    mcResult_t mcRet;
    mcVersionInfo_t versionInfo;
    uint8_t* pTrustletData = NULL;
    uint32_t nTrustletSize;

    printf("Opening <t-base device\n");
    mcRet = mcOpenDevice(DEVICE_ID);
    if (MC_DRV_OK != mcRet) 
    {
	    printf("Error opening device: %d\n", mcRet);
	    return mcRet;
    }

    mcRet = mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
    if (MC_DRV_OK != mcRet) 
    {
        printf("mcGetMobiCoreVersion failed %d\n", mcRet);
        mcCloseDevice(DEVICE_ID);
        return mcRet;
    }
    LOG_I("productId        = %s", versionInfo.productId);
    LOG_I("versionMci       = 0x%08X", versionInfo.versionMci);
    LOG_I("versionSo        = 0x%08X", versionInfo.versionSo);
    LOG_I("versionMclf      = 0x%08X", versionInfo.versionMclf);
    LOG_I("versionContainer = 0x%08X", versionInfo.versionContainer);
    LOG_I("versionMcConfig  = 0x%08X", versionInfo.versionMcConfig);
    LOG_I("versionTlApi     = 0x%08X", versionInfo.versionTlApi);
    LOG_I("versionDrApi     = 0x%08X", versionInfo.versionDrApi);
    LOG_I("versionCmp       = 0x%08X", versionInfo.versionCmp);

    tci = (dapc_tciMessage_t*)malloc(sizeof(dapc_tciMessage_t));
    dci = (dapc_tciMessage_t*)malloc(sizeof(dapc_tciMessage_t));
    
    if (tci == NULL) 
    {
        printf("Allocation of TCI failed\n");
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(tci, 0, sizeof(dapc_tciMessage_t));
    
    if (dci == NULL) 
    {
        printf("Allocation of DCI failed\n");
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_NO_FREE_MEMORY;
    }
    memset(dci, 0, sizeof(dapc_tciMessage_t));      

    nTrustletSize = getFileContent(
                        "05200000000000000000000000000000.tlbin",
                        &pTrustletData);                      
                        
    if (nTrustletSize == 0) 
    {
        printf("Trustlet not found\n");
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }

    printf("Opening the session\n");
    memset(&sessionHandle, 0, sizeof(sessionHandle));
    sessionHandle.deviceId = DEVICE_ID; // The device ID (default device is used)
    mcRet = mcOpenTrustlet(
            &sessionHandle,
            MC_SPID_RESERVED_TEST, /* mcSpid_t */
            pTrustletData,
            nTrustletSize,
            (uint8_t *) tci,
            sizeof(dapc_tciMessage_t));
            
    if (MC_DRV_OK != mcRet) 
    {
        printf("Open session failed: %d\n", mcRet);
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
    }
    else 
    {
        printf("open() succeeded\n");
    }
            
    nTrustletSize = getFileContent(
                        "05190000000000000000000000000000.drbin",
                        &pTrustletData);                      
                        
    if (nTrustletSize == 0) 
    {
        printf("Driver not found\n");
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }
	
    // Whatever the result is, free the buffer
    free(pTrustletData);

    if (MC_DRV_OK != mcRet) 
    {
        printf("Open session failed: %d\n", mcRet);
        free(tci);
        tci = NULL;
        mcCloseDevice(DEVICE_ID);
    }
    else 
    {
        printf("open() succeeded\n");
    }

    return mcRet;
}

// -------------------------------------------------------------


// -------------------------------------------------------------
mcResult_t getDevinfo(uint32_t *index, uint32_t *result)
{
    mcResult_t  ret;

    
    ret = executeCmd(index, result, CMD_DEVINFO_GET );
    if (ret != RET_OK) 
    {
        printf("Unable to execute CMD_FOO_SUB command: %d", ret);
        goto exit;
    }   

    printf("The result is %d \n", *result);

exit:    
    return ret;
}


// -------------------------------------------------------------
void tlcClose(void)
{
    mcResult_t ret;

    LOG_I("Closing the session");
    ret = mcCloseSession(&sessionHandle);
    if (MC_DRV_OK != ret) 
    {
        LOG_E("Closing session failed: %d", ret);
        /* continue even in case of error */
    }

    LOG_I("Closing <t-base device");
    ret = mcCloseDevice(DEVICE_ID);
    if (MC_DRV_OK != ret) 
    {
        LOG_E("Closing <t-base device failed: %d", ret);
        /* continue even in case of error */;
    }
    free(tci);
    tci = NULL;
}
