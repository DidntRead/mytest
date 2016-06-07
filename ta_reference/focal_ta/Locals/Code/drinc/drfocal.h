#ifndef __DRFOCAL_H__
#define __DRFOCAL_H__


#define MAX_ID_NAME_SIZE 128
#define MAX_ID_LIST_SIZE 5

struct tlfocalTpl{
	int id;
	char *name;
};

struct tlAliApiDevice{
	char cmd;
	unsigned int version;
	unsigned int authenticatorVersion;
	int idsCount;
	uint32_t ids[MAX_ID_LIST_SIZE];
	int index;
	int lastPassid;
	uint32_t idStampLow;
	uint32_t idStampHigh;
	struct tlfocalTpl focalTpl[MAX_ID_LIST_SIZE];
};

struct tlfocalApiDevice{
	char cmd;
	unsigned int version;
	unsigned int authenticatorVersion;
	int idsCount;
	uint32_t *ids;
	int index;
	int lastPassid;
	struct tlfocalTpl *focalTpl;
	//int id;
	//char *name;
};

enum cmdID{
	FOCAL_INIT_DATA,
	FOCAL_SET_VERSION,
	FOCAL_SET_IDNAME,
	FOCAL_SET_IDS,
	FOCAL_SET_LASTPASSID,
	FOCAL_ADD_TPL,
	FOCAL_DEL_TPL,
	
	ALI_GET_VERSION,
	ALI_GET_AUTHVERSION,
	ALI_GET_NAME,
	ALI_GET_IDENTIFY_RESULT,
	ALI_GET_IDS,
	ALI_GET_AUTHSIGN,
	
	LAST_CMD
};

enum retStatusCode{
	ALI_STATUS_NULLPTR = 1,
	
};

enum tplCmd{
	FOCAL_DB_TPL_ADD,
	FOCAL_DB_TPL_DEL
};

#endif //__DRFOCAL_H__
