
#include "../include/shstd.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <iostream>

using namespace std;



struct TValue
{   
    char    m_szBuf[20];

    const TValue & operator=(const TValue &v) {
        memcpy(m_szBuf, v.m_szBuf, sizeof(m_szBuf));
        return *this;
    }

    static uint32_t MyHash(const int &key)    {
        return key;
    }
};


void test_hashmap()
{
    shstd::hashmap::CTHaspMap<int, TValue> hm;

    TValue tv;
    int err;
    int dotimes = 0;
    
    const uint32_t nSize = 18;
    err = hm.Init(nSize, TValue::MyHash, 6);
    if(0 != err) {
        printf("init failed| err=%d\r\n", err);
        exit(0);
    }

    for(int i = 0; i < nSize+2; i++)
    {
        sprintf(tv.m_szBuf, "value_%d", i);
        err = hm.Set(i, tv);
        if(0 != err) 
        {
            printf("set failed| key=%d| value=%s| err=%d\r\n", i, tv.m_szBuf, err);
            hm.Set(i - 10, tv);
        }
    }

    hm.Del(7);
    hm.Del(1);
    
    hm.Del(12);
    hm.Del(6);
    hm.Del(0);

    hm.Del(3);
    hm.Del(15);
    hm.Del(9);

    hm.Del(14);
    hm.Del(2);
    printf("size=%d| free=%d\n", hm.Size(), hm.Free());


    #if 0
    AGAIN:
    printf("\n********* dotimes=%d, free=%d, size=%d **************\n", dotimes + 1, hm.Free(), hm.Size());
    for(int i = 0; i < 10; i++)
    {
        int key = i*6 + 5;
        sprintf(tv.m_szBuf, "value_%d", i);
        err = hm.Set(key, tv);
        if(0 != err) 
        {
            printf("set failed| key=%d| value=%s| err=%d\r\n", key, tv.m_szBuf, err);
            hm.Set(i - 10, tv);
        }
    }
    
    hm.Del(17);
    hm.Del(5);
    hm.Del(11);
    hm.Del(11);
    hm.Del(14);

    for(int i = 0; i < nSize; i++) 
    {
        err = hm.Get(i, tv);
        if(0 == err)
        {
            printf("Get succ| key=%d| value=%s| size=%d| free=%d\r\n", i, tv.m_szBuf, hm.Size(), hm.Free());
        }
        else
        {
            //printf("Get no recore| key=%d| err=%d\r\n", i, err);
        }
    } 
    

    if(++dotimes < 1)
    {
        goto AGAIN;
    }
    
    hm.Set(30, tv);
    hm.Set(32, tv);
    for(int i = 0; i < 33; i++) 
    {
        err = hm.Get(i, tv);
        if(0 == err)
        {
            printf("Get succ| key=%d| value=%s| size=%d| free=%d\r\n", i, tv.m_szBuf, hm.Size(), hm.Free());
        }
        else
        {
            //printf("Get no recore| key=%d| err=%d\r\n", i, err);
        }
    } 
    #endif
    printf("\n\n****************** Get ptr ******************\n");
    TValue *pTV = hm.Get(17);
    if(NULL != pTV) {
        printf("modify before key=17| value=%s\r\n", pTV->m_szBuf);
        snprintf(pTV->m_szBuf, sizeof(pTV->m_szBuf) - 1, "%s", "new value");
    }

    pTV = hm.Get(17);
    if(NULL != pTV) {
        printf("modify after key=17| value=%s\r\n", pTV->m_szBuf);
    }
    
    printf("\n\n****************** Traservse ******************\n");

    for(int i = 0; i < nSize+2; i++)
    {
        err = hm.Get(i, tv);
        if(0 == err)
        {
            printf("Get succ| key=%d| value=%s| size=%d| free=%d\r\n", i, tv.m_szBuf, hm.Size(), hm.Free());
        }
    } 
    
    shstd::hashmap::CTHaspMap<int, TValue>::CHNode *node = hm.Begin();
    dotimes = 1;
    while(NULL != node)
    {
        printf("the %.02d record| key=%.02d| value=%s\r\n", dotimes, node->key, node->val.m_szBuf);
        node = hm.Next();
        dotimes++;
    }
    
    return ;

    for(int i = 0; i < nSize; i++) 
    {
        err = hm.Get(i, tv);
        if(0 == err)
        {
            printf("Get succ| key=%d| value=%s| size=%d| free=%d\r\n", i, tv.m_szBuf, hm.Size(), hm.Free());
        }
        else
        {
            //printf("Get no recore| key=%d| err=%d\r\n", i, err);
        }
    }
    
    for(int i = 5; i < 10; i++)
    {   
        int key = rand()%nSize;
        hm.Del(key);
        printf("Del node| key=%d\n", key);
    }
    printf("after del node ...\r\n");

    for(int i = 0; i < nSize; i++) 
    {
        err = hm.Get(i, tv);
        if(0 == err)
        {
            printf("Get succ| key=%d| value=%s| size=%d| free=%d\r\n", i, tv.m_szBuf, hm.Size(), hm.Free());
        }
        else
        {
            //printf("Get no recore| key=%d| err=%d\r\n", i, err);
        }
    }    
}

int main( int argc, const char * *argv )
{
    test_hashmap();
    return 0;

	/*if (argc > 1) 
		{
		times = atoi(argv[1]);
		if (times <= 0) times = 5;
		}

	writedata();
	readdata();

	return 0;*/

    shstd::log::Init( "./logs", "mon", "debug,info,warn,error", "abc" );
    shstd::log::SetAsynMode();
    
    time_t ts = time( NULL );
    int times = 500 * 10000;

    for( int i = 0; i < times; i++ )
    {
        //LOG_DEBUG_TRANS(LT_DEBUG, "i=%010d", i );
        LOG(LT_INFO_TRANS, "tasnid_n02", "ACPC_TASK| Recv packet| fd=%d| cmd=%u| len=%d| time=%d| queue_num=%d", 10000, 20000, 300, 1000, i);
    }
    int ust = (int)(time(NULL) - ts);
    printf( "cnt=%d| time=%ds| ", times, ust);
    if(ust > 0) {
        printf("rate=%d| single_use_time=%d", times/ust, (ust*1000*1000)/times);
    }
    
    printf("\n");


    return 0;
}


