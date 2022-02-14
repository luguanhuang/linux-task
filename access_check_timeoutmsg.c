
#include "../util/access_global.h"
#include "../interface/access_protocol.h"
//#include "../include/ydt_log.h"
#include "access_comm_client_snd.h"
#include "access_comm_service_srv_rcv.h"
#include "access_comm_client_rcv.h"
#include "access_epoll.h"
#include "access_heartbeat_detectthread.h"
#include "access_thread_pool.h"
#include "access_check_timeoutmsg.h"

extern Server_Conf_Info g_srv_conf_info;
extern StClientInfo_Hashtable g_clientinfo_hash;
extern StTmp_clientSock_Hash g_sndclient_tmp_sockhash;
static void C_DelClientInfoFromHash(int nSock)
{	
	INFO("C_DelCloseClientInfoFromHash: func begin%s", "");
	int i = 0;
	int nSize = 0;
	StHash_Item *pItem = NULL;
	StHash_Item *pLast_item = NULL;
	Client_Info *pClient_info = NULL;
	
	StHash_Table *pHashtable = g_clientinfo_hash.pClient_info_hash;
	nSize = pHashtable->nBucket_size;
	for (i=0; i<nSize; i++)
	{
		pItem = pHashtable->ppItem[i];
		pLast_item = pItem;
		while (NULL != pItem)
		{
			pClient_info = (Client_Info *)pItem->pMatch_msg;		
			if (nSock == pClient_info->nClient_socket)
			{
				INFO("C_DelCloseClientInfoFromHash: client is disconnect with access server"
					" so we will delete the client info item from client info hashtable [client socket]=%d [hashtable length]=%d", \
					nSock, pHashtable->nHashtable_len);
				if (pLast_item == pItem)
				{
					pHashtable->ppItem[i] = pItem->pNext;
					pLast_item = pItem->pNext;
					MM_FREE(pItem->pKey);
					MM_FREE(pItem->pMatch_msg);
					MM_FREE(pItem);	
					pItem = pLast_item;
				}
				else
				{
					pLast_item->pNext = pItem->pNext;
					MM_FREE(pItem->pKey);
					MM_FREE(pItem->pMatch_msg);
					MM_FREE(pItem);
					pItem = pLast_item->pNext;
				}

				if (pHashtable->nHashtable_len > 0)
				{
					pHashtable->nHashtable_len--;
				}
			}
			else
			{
				pLast_item = pItem;
				pItem = pItem->pNext;
			}
		}
	}
	
	INFO("C_DelCloseClientInfoFromHash: func end%s", "");
}

static void C_DelCloseClientSockInfo(int nSock)
{
	INFO("C_DelCloseClientSockInfo: func begin%s", "");
	int nRet = 0;
	
	nRet = DelSockEventFromepoll(nSock);
	if (FALSE == nRet)
	{
		INFO("C_DelCloseClientSockInfo: Call DelSockEventFromepoll error%s", "");
	}
	else
	{
		INFO("C_DelCloseClientSockInfo: Call DelSockEventFromepoll succeed%s", "");
	}
	
	C_DelClientInfoFromHash(nSock);
	
	DelClientSockFromQueue(nSock);
	DelTmpClientSockFromHashtable(nSock);	
	DelClientMsgBufFromHash(nSock);
	DeleteHeartbeatDetectItem(nSock);			//ɾ�������������һ��
	INFO("C_DelCloseClientSockInfo: func end%s", "");
}


//������;: ��ʱ��⺯��,   ��Ҫ�������ҵ�������æ��� (ͨ���Ƚϵ�ǰʱ������Ϣ�����͵�ʱ��)
//�������:  ��
//�������:  ��
//����ֵ	: ��

void *CheckTimeoutMsg(void *pParam)
{
	INFO("CheckTimeoutMsg: func begin%s", "");
	pthread_detach(pthread_self());
	long lCur_time = 0;
	int i = 0;
	int nDiff = 0;
	StHash_Item *pItem = NULL;
	StHash_Item *pLast_item = NULL;
	Client_Info *pClient_info = NULL;
	int nClient_sock = 0;
	Server_Client_Msg srv_client_msg;
	int nPool_clientmsg_interval = 0;
	int nClientmsg_timeout_time = 0;
	int nRet = 0;
	int nSnd_clientmsg_fail = FALSE;

	pthread_mutex_lock(&g_srv_conf_info.conf_info_mutex);
	nPool_clientmsg_interval = g_srv_conf_info.nPool_clientmsg_interval;
	nClientmsg_timeout_time = g_srv_conf_info.nClientmsg_timeout_time;
	pthread_mutex_unlock(&g_srv_conf_info.conf_info_mutex);

	DEBUG("CheckTimeoutMsg: [pool client message time interval]=%d [client message timeout time]=%d", \
	nPool_clientmsg_interval, nClientmsg_timeout_time);


	/*��ѭ����Ҫ�Ǽ��ͻ��˵ĳ�ʱ��Ϣ��
	  *���ҵ�������æ,  �ͷ���һ��������æ��Ϣ���ͻ���
	  *�Ա���ͻ���æ��
	  */
	
	while (TRUE)
	{
		//��ѯ��ʱ��Ϣ��ʱ����
		sleep(nPool_clientmsg_interval);
		pthread_mutex_lock(&g_clientinfo_hash.client_info_mutex);				//�Թ�ϣ�����
		CHECK_TIMEOUT_ITEM_BEGIN:
		//�����ͻ��˷��͹�������Ϣ
		for (i=0; i<g_clientinfo_hash.pClient_info_hash->nBucket_size; i++)
		{	
			pItem = g_clientinfo_hash.pClient_info_hash->ppItem[i];
			pLast_item = pItem;					//����ͷ�ڵ�
			while (pItem != NULL)
			{
				pClient_info = (Client_Info *)pItem->pMatch_msg;
				lCur_time = time((time_t *)NULL);
				nDiff = lCur_time - pClient_info->lSent_time;

				//����ͻ��˵���Ϣ����TIMEOUT_TIMEʱ�仹û���ͳ�ȥ�Ļ�, �ͷ���
				//һ����ʱ��Ϣ�����ͻ��� 
				if (nDiff > nClientmsg_timeout_time)
				{
					DEBUG("CheckTimeoutMsg: [client message time different]=%d [client message timeout time]=%d"
						" [client info hashtable length]=%d", \
						nDiff, nClientmsg_timeout_time, g_clientinfo_hash.pClient_info_hash->nHashtable_len);

					memset(&srv_client_msg, 0, sizeof(srv_client_msg));
					nRet = GetServerErrorPacket(&srv_client_msg, pClient_info);
					if (TRUE != nRet)
					{
						goto DELETE_TIMEOUT_ITEM_BEGIN;
					}

					nClient_sock = pClient_info->nClient_socket;
		
					//���ͳ�ʱ��Ϣ�����ͻ���
					nRet = HandleClientSock(pClient_info->nClient_socket, &g_sndclient_tmp_sockhash);
					if (CLIENT_SOCK_EXIST == nRet)
					{
						INFO("CheckTimeoutMsg: the socket is exist yet in the temp socket hashtable, so we will get next"
										   " item from client info hashtable to snd timeout message%s", "");
						pItem = pItem->pNext;
						continue;
					}
					else if (FALSE == nRet)
					{
						
						ERROR("CheckTimeoutMsg: Call HandleSndClientSock error%s", "");
						continue;
					}
				  
					nRet = SendClientMsg(pClient_info->nClient_socket, &srv_client_msg, RESPONSE_CLIENT_MSG);
					if (FALSE == nRet)
					{
						ERROR("CheckTimeoutMsg: Call SendClientMsg error send timeout error packet to client error%s", "");
						close(nClient_sock);
						nSnd_clientmsg_fail = TRUE;
					}
					else
					{
				
					}

					MM_FREE(srv_client_msg.pData);
					
					//�ѹ�ϣ������ĳ�ʱ��Ϣ��ɾ�� 
					
					DELETE_TIMEOUT_ITEM_BEGIN:
					if (pLast_item == pItem) 	//�����ͷ��㳬ʱ�� (���pLast_item ����pItem �ͱ�ʾ��ɾ����ͷ�ڵ�)
					{
						g_clientinfo_hash.pClient_info_hash->ppItem[i] = pItem->pNext;
						pLast_item = pItem->pNext;
						MM_FREE(pItem->pKey);
						MM_FREE(pItem->pMatch_msg);
						MM_FREE(pItem);
						pItem = pLast_item;
					}
					else
					{
						pLast_item->pNext = pItem->pNext;
						MM_FREE(pItem->pKey);
						MM_FREE(pItem->pMatch_msg);
						MM_FREE(pItem);
						pItem = pLast_item->pNext;
					}

					if (g_clientinfo_hash.pClient_info_hash->nHashtable_len > 0)
					{
						g_clientinfo_hash.pClient_info_hash->nHashtable_len--;
					}
					
					if (TRUE == nSnd_clientmsg_fail)
					{
						WARN("CheckTimeoutMsg: as client is disconnect with access server so we will delete some info relate with client"
							" [socket id]=%d", nClient_sock);
						C_DelCloseClientSockInfo(nClient_sock);
						nSnd_clientmsg_fail = FALSE;
						goto CHECK_TIMEOUT_ITEM_BEGIN;
						
					}

					DelSndClientTmpSockFromHash(nClient_sock);
				}
				else
				{
					pLast_item = pItem;
					pItem = pItem->pNext;
				}
			}
		}

		pthread_mutex_unlock(&g_clientinfo_hash.client_info_mutex);
	}

	INFO("CheckTimeoutMsg: func end%s", "");
	return NULL;
}





