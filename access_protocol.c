
#include "access_protocol.h"
#include "../util/access_global.h"

int GetServerErrorXmlData(WORD wSeq, WORD wError_code, char **ppXML_data, WORD *pXML_len)
{
	INFO("GetServerErrorXmlData: func begin%s", "");
	char szXML_data[1024] = {0};
	
	snprintf(szXML_data, sizeof(szXML_data) - 1, "<mythlink><sequence><![CDATA[%d]]></sequence><errorCode><![CDATA[%d]]></errorCode></mythlink>", wSeq, wError_code); 
	int nLen = strlen(szXML_data);
	
	*ppXML_data = (char *)MM_MALLOC_WITH_DESC(nLen + 1, \
	"GetServerErrorXmlData: Call  func for error xml data");
	if (NULL == (*ppXML_data))
	{
		FATAL("GetServerErrorXmlData: Call malloc error%s", "");
		return OUT_OF_MEMORY_ERROR;
	}

	memset(*ppXML_data, 0, nLen + 1);
	memcpy(*ppXML_data, szXML_data, nLen);
	*pXML_len = nLen;

	DEBUG("GetServerErrorXmlData: [xml data]=%s [xml len]=%d", *ppXML_data, *pXML_len);
	INFO("GetServerErrorXmlData: func end%s", "");
	return TRUE;
}

int GetServerErrorPacket(Server_Client_Msg *pSrv_client_msg, void *pClient_msg_info)
{
	INFO("GetServerErrorPacket: func begin%s", "");
	if (NULL == pSrv_client_msg || NULL == pClient_msg_info)
	{
		ERROR("GetServerErrorPacket: func param error%s", "");
		return FUNC_PARAM_ERROR;
	}

	Client_Info *pClient_info = (Client_Info *)pClient_msg_info;
	int nRet = 0;
	char *pXML_data = NULL;
	WORD wXML_len = 0;
	int nData_len = 0;
	
	Server_Error server_error;
	memset(&server_error, 0, sizeof(server_error));

	nRet = GetServerErrorXmlData(pClient_info->nSeq, TIMEOUT_ERROR, &pXML_data, &wXML_len);
	if (TRUE != nRet)
	{
		ERROR("GetServerErrorPacket: Call GetServerErrorXmlData error%s", "");
		return nRet;
	}	

	server_error.bVersion = pClient_info->bService_version;
	server_error.wCmd_id = htons(SERVER_ERR_MSG);
	server_error.wXML_len = htons(wXML_len);
	server_error.pXML_data = pXML_data;
	memcpy(server_error.arrCheck_code, pClient_info->arrService_check_code, 4);

	DEBUG("GetServerErrorPacket: server timeout error packet [xml data]=%s [xml len]=%d", \
						server_error.pXML_data, wXML_len);

	pSrv_client_msg->bVersion = ACCESS_SRV_VERSION;
	pSrv_client_msg->bMsg_type = SERVER_CLIENT_MSG;
	pSrv_client_msg->bMain_service_code = pClient_info->bMain_service_code;
	pSrv_client_msg->bSub_service_code = pClient_info->bSub_service_code;
	pSrv_client_msg->wAccess_server_seq = htons(ACCESS_SRV_SEQ);
	pSrv_client_msg->nMsg_counter = htonl(pClient_info->nMsg_counter);
	nData_len = sizeof(Server_Error) - 8 + wXML_len + 4;
	pSrv_client_msg->nData_len = htonl(nData_len);
	pSrv_client_msg->pData = (char *)MM_MALLOC_WITH_DESC(nData_len, \
	"GetServerErrorPacket: Call  func for service layer data");
	if (NULL == pSrv_client_msg->pData)
	{
		FATAL("GetServerErrorPacket: Call MM_MALLOC_WITH_DESC error%s", "");
		MM_FREE(server_error.pXML_data);
		return OUT_OF_MEMORY_ERROR;
	}

	int nTmp_len = sizeof(server_error) - 8;
	memcpy(pSrv_client_msg->pData, &server_error, nTmp_len);
	memcpy(pSrv_client_msg->pData + nTmp_len, server_error.pXML_data, wXML_len);
	memcpy(pSrv_client_msg->pData + nData_len - 4, server_error.arrCheck_code, 4);
	memcpy(pSrv_client_msg->arrCheckout_code, pClient_info->arrAccess_check_code, 4);

	char arrAcc_checkcode[5] = {0};
	memcpy(arrAcc_checkcode, pSrv_client_msg->arrCheckout_code, 4);
	char arrService_checkcode[5] = {0};
	memcpy(arrService_checkcode, pClient_info->arrService_check_code, 4);

	DEBUG("GetServerErrorPacket: [version]=%d [msg type]=%d [main service code]=%d [sub service code]=%d"
						" [access server seq]=%d [msg counter]=%d [data len]=%d [access check code]=%s service data:"
						" [version]=%d [cmd_id]=%d [xml data]=%s [xml len]=%d [service_checkcode]=%s", \
						pSrv_client_msg->bVersion, pSrv_client_msg->bMsg_type, pSrv_client_msg->bMain_service_code, \
						pSrv_client_msg->bSub_service_code, ntohs(pSrv_client_msg->wAccess_server_seq), \
						ntohl(pSrv_client_msg->nMsg_counter), ntohl(pSrv_client_msg->nData_len), arrAcc_checkcode, \
						server_error.bVersion, ntohs(server_error.wCmd_id), server_error.pXML_data, ntohs(server_error.wXML_len), arrService_checkcode);	

	MM_FREE(server_error.pXML_data);			
	INFO("GetServerErrorPacket: func end%s", "");
	return TRUE;
}


void FreeMsgBuffer(void *pMsg)
{
	INFO("FreeMsgBuffer: func begin%s", "");
	if (NULL == pMsg)
	{
		ERROR("FreeMsgBuffer: func param error%s", "");
		return;
	}
	BYTE bMsg_type = 0;
	char *pTmp_msg = (char *)pMsg;
	Server_Client_Msg *pServer_client_msg = NULL;
	Forward_Srv_Client_Msg *pForward_srv_client_msg = NULL;
	
	pTmp_msg++;
	bMsg_type = *pTmp_msg;
	if (SERVER_CLIENT_MSG == bMsg_type)
	{
		INFO("FreeMsgBuffer: we will free server client message buffer%s", "");
		pServer_client_msg = (Server_Client_Msg *)pMsg;
		MM_FREE(pServer_client_msg->pData);
		MM_FREE(pServer_client_msg);
	}
	
	else if (FORWARD_SRV_CLIENT_MSG == bMsg_type)
	{
		INFO("FreeMsgBuffer: we will free forward server client message buffer%s", "");
		pForward_srv_client_msg = (Forward_Srv_Client_Msg *)pMsg;
		MM_FREE(pForward_srv_client_msg->pData);
		MM_FREE(pForward_srv_client_msg);
	}
	
	INFO("FreeMsgBuffer: func end%s", "");
}


