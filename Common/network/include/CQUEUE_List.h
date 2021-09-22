
/**********************************************************
 * name              : CQUEUE_List.h
 * Description:      : 
 * Create by         :
 * Last Modified int : 2019-8-30
 *********************************************************/

#ifndef _CQUEUE_List_H_
#define _CQUEUE_List_H_
#include "netdef.h"
#include "time_piece.h"
#include "typedef.h"
#include <cstdlib>
#include <stdio.h>  
#include <unistd.h>  
#include <pthread.h>


#define	MAXQUEUE            (40)        // 0:空闲队列 1-10:其他队列 11-n_send_threads:发送队列
#define	MAXHASHLIST	        (5999)      //
//#define MAXBUFFERLENGTH     (8*1024)    //长度

typedef enum
{
	QUEUE_FREE          = 0,	// 空闲队列
	QUEUE_CLIENT_READ   = 1,	// 客户端读队列 
	QUEUE_CLIENT_WRITE  = 2,    // 客户端写队列
    QUEUE_SEND          = 11,	// 客户端数据发送队列,含多个队列, 从 11~(11+n_send_threads)
} QUEUE_NAME;

enum {
    PKT_TYPE_DEFAULT    = 0,
    PKT_TYPE_DISCARD    = 1,    //内部产生,处理完就丢弃,不需要应答
    PKT_TYPE_DELAY_DONE = 2,    //需要延时处理的报文
}; //DEFINE_PACKET_TYPE

typedef struct listsmg
{   
    uint32  nCmd;                   //命令
    uint32  nErrCode;               //错误码
    char    szTransID[TID_LEN];     //任务ID[模块ID+主机ID+唯一子串_序列子串]
    int     cPacketType;            //类型(见DEFINE_PACKET_TYPE定义)
    int     connfd;                 //链接fd      
    uint32  nUniqueID;              //链接唯一标记(为每个链接分配一个唯一标记)
    uint64  nCliConnID;             //客户端(玩家终端)连接唯一ID(api/brain根据该值匹配指定的客户端)
                                    //   1.请求-应答类消息: 服务端按原样返回
                                    //   2.服务端主动通知类消息:(acpcsvr)需要找到玩家在redis的对应值
    uint32  nRoleID;                //角色ID
    uint32  nRemoteServerID;        //连接对端的ServerID
        
    char    sAddr[32];              //IP地址
    unsigned short  nPort;          //端口    
    CTimePiece  recv_time;          //接收时间
    uint32      nDelayTime;         //延时时长(单位:毫秒) cPacketType=PKT_TYPE_DELAY_DONE有效
    uint32      nDelayTimes;        //延时次数 cPacketType=PKT_TYPE_DELAY_DONE有效
    
    unsigned short len;                     //包体长度                                    
    unsigned short recv_len;                //已接收长度
    char cPacketBuffer[PACKAGE_LEN_MAX];    //包体内容
    char *pRespBuffer;                      //返回内容指针(暂时不用)
	char *cAddBuffer;                       //新增内容指针(暂时不用)
    struct listsmg * next;                  // next node           
    struct listsmg * previous;              // previous node        

    void Reset() {
        nCmd                = 0;
        nErrCode            = 0;
        szTransID[0]        = 0;
        cPacketType         = PKT_TYPE_DEFAULT;
        connfd              = 0;
        sAddr[0]            = 0;
        nPort               = 0;
        len                 = 0;
        cPacketBuffer[0]    = 0;
        nUniqueID           = 0;
        nCliConnID          = 0;
        nRoleID             = 0;
        nRemoteServerID     = 0;
        recv_len            = 0;
        nDelayTime          = 0;
        nDelayTimes         = 0;
        
        recv_time.Restart();
    }
}LISTSMG;

typedef struct queue_data
{
    int packet_num; 
    int hash_num; 
    int queue[MAXQUEUE]; 
}QUEUE_DATA;


class CQUEUE_List
{

public:
	LISTSMG *area_head;
	LISTSMG *queue_head[MAXQUEUE];
	int     n_total_node;
	int     *n_queue_node[MAXQUEUE];
	int     n_hash_node;

public:
	LISTSMG *queue_hash[MAXHASHLIST];

private:
	pthread_mutex_t queue_hash_mutex;
	pthread_mutex_t queue_mutex[MAXQUEUE];
	pthread_cond_t  queue_cond[MAXQUEUE];
    CQUEUE_List();
    
public:
	static CQUEUE_List* Instance()
    {
        static CQUEUE_List m_instance;
        return &m_instance;
    }
    
	~CQUEUE_List();

	int CreateQueue(int MaxNumber );

	int FreeQueue( void );

	int CheckQueue( int queue_num );

	int GetQueueNum( int queue_num ,int *o_num);

	LISTSMG * GetBlockNode( int queue_num ,int *o_num);

	LISTSMG * GetNoBlockNode( int queue_num ,int *o_num);
	LISTSMG * GetNode( int queue_num ,int *o_num);

	int SetNode( LISTSMG * p,int queue_num );


	void GetQueueNum( QUEUE_DATA *in_data );

}; /* end class CQUEUE_List */

#endif	/* _CQUEUE_List_H */

