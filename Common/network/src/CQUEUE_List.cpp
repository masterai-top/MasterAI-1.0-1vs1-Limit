/**********************************************************************
* Program Name:  CQUEUE_List.cpp
* Description:   
* Create by:
* Last Modified in: 2010.5.20
* Demo:
***********************************************************************/


#include "CQUEUE_List.h"

CQUEUE_List::CQUEUE_List()
{
	int i;

	for ( i = 0; i<MAXQUEUE; i++ )
	{
		n_queue_node[i] = (int *) malloc( sizeof(int) );
		if( n_queue_node[i] == NULL)
		{
			return ;
		}
	}

	/*
	 * .
	 */
	n_total_node = 0;
	n_hash_node  = 0;
	for(i = 0; i<MAXQUEUE; i++ )
	{
		*n_queue_node[i] = 0;
	}

	/*
	 * .
	 */
	for (int i=0 ; i< MAXHASHLIST; i++ )
	{
        queue_hash[i] = (LISTSMG *)NULL;
	}

}

CQUEUE_List::~CQUEUE_List()
{
	int i;

	for(i=0;i<MAXQUEUE;i++)
	{
		free(n_queue_node[i]);
	}
}

int CQUEUE_List::CreateQueue( int MaxNumber )
{
	LISTSMG *p, *q;
	int i;
	
	/*
	 */
	if( n_total_node > 0 ) 
	{
		return 1;
	}
	
	if( MaxNumber <= 0 )
	{
		return 2;
	}

	/*
	 */
	n_total_node    = MaxNumber;
	*n_queue_node[0] = MaxNumber;
	n_hash_node     = 0;
	for(i = 1; i<MAXQUEUE; i++ )
	{
		*n_queue_node[i] = 0;
	}

	/*
	 */
	area_head = (LISTSMG *) calloc( MaxNumber, sizeof(LISTSMG) );
	if( area_head == NULL)
	{
		return 1;
	}

	/*
	 */
	pthread_mutex_init(&queue_hash_mutex,NULL);
	for( i = 0; i< MAXQUEUE; i++ )
	{
		pthread_mutex_init(&queue_mutex[i],NULL);
		pthread_cond_init(&queue_cond[i],NULL);
		queue_head[i] = (LISTSMG *)malloc( sizeof(LISTSMG) );	
		if( queue_head[i] == NULL )
		{
			return (i+1);
		}
		queue_head[i]->next     =  NULL;
		queue_head[i]->previous = NULL;
	}

	/*
	 */
	p = area_head;
	q = queue_head[0];
	q->previous = NULL;
	q->cAddBuffer = NULL;
	for ( i=0 ; i< n_total_node; i++ )
	{
		p->cAddBuffer = NULL;
		p->pRespBuffer = NULL;
		q->next = p;
		p->previous = q;
		q = p ++;
	}
	(p-1)->next = queue_head[0]->next;
	queue_head[0]->next->previous = q;
	return 0;
}

int CQUEUE_List::FreeQueue( void )
{
	int i;

	if ( n_total_node <= 0 )
	{
		return 1;
	}

	/*
	 */
	pthread_mutex_destroy(&queue_hash_mutex);
	for( i = 0; i< MAXQUEUE; i++ )
	{
		pthread_mutex_destroy(&queue_mutex[i]);
		pthread_cond_destroy(&queue_cond[i]);
		free ( queue_head[i] );
	}

	/*
	 */
	free(area_head);


	/*
	 */
	n_total_node = 0;
	n_hash_node  = 0;
	for(i = 0; i<MAXQUEUE; i++ )
	{
		*n_queue_node[i] = 0;
	}

	return 0;
}

int CQUEUE_List::CheckQueue( int queue_num )
{
	int recode;

	/*
	 */
	if ( queue_num <0 || queue_num >= MAXQUEUE )
	{
		return 0;
	}

	pthread_mutex_lock( &queue_mutex[queue_num] );
	if ( queue_head[queue_num]->next == NULL )
	{
		recode = 0;
	}
	else
	{
		recode = 1;
	}
	pthread_mutex_unlock( &queue_mutex[queue_num] );
	
	return recode;
}

int CQUEUE_List::GetQueueNum( int queue_num ,int *o_num)
{
	if ( o_num == NULL || queue_num <0 || queue_num >= MAXQUEUE )
	{
		return -1;
	}

	pthread_mutex_lock( &queue_mutex[queue_num] );
	*o_num = *n_queue_node[queue_num];
	pthread_mutex_unlock( &queue_mutex[queue_num] );
	
	return 0;
}

LISTSMG * CQUEUE_List::GetBlockNode( int queue_num , int *o_num )
{
	LISTSMG *p,*head;


	if ( queue_num <0 || queue_num >= MAXQUEUE )
	{
		return NULL;
	}
	
	pthread_mutex_lock( &queue_mutex[queue_num] );
	while (  queue_head[queue_num]->next == NULL )
	{
		pthread_cond_wait( &queue_cond[queue_num],&queue_mutex[queue_num] );
	}

	(*n_queue_node[queue_num])--;

	
	head = queue_head[queue_num];
	p = head->next->previous;
	if( p == head->next )
	{
		head->next = NULL;
		head->previous = NULL;
		*o_num = *n_queue_node[queue_num];
		pthread_mutex_unlock( &queue_mutex[queue_num] );
		return( p );
	}
	p->previous->next = head->next;
	head->next->previous = p->previous;
	*o_num = *n_queue_node[queue_num];
	pthread_mutex_unlock( &queue_mutex[queue_num] );
	return (p);
}

LISTSMG * CQUEUE_List::GetNode( int queue_num , int *o_num )
{
	LISTSMG *p,*head;
//	time_t curr_time;

	/*
	 */

	if ( queue_num <0 || queue_num >= MAXQUEUE )
	{
		return NULL;
	}

	/*
	 */
	pthread_mutex_lock( &queue_mutex[queue_num] );
	while (  queue_head[queue_num]->next == NULL )
	{
		pthread_cond_wait( &queue_cond[queue_num],&queue_mutex[queue_num] );
	}

	(*n_queue_node[queue_num])--;
	/*
	 */
	head = queue_head[queue_num];
	p = head->next->previous;
	if( p == head->next )
	{
		head->next = NULL;
		head->previous = NULL;
		*o_num = *n_queue_node[QUEUE_CLIENT_READ] + *n_queue_node[QUEUE_CLIENT_WRITE];
		pthread_mutex_unlock( &queue_mutex[queue_num] );
	/*	
		fprintf(fax_fp," : p=head->next NUM  %d  = %d  \n",queue_num,*n_queue_node[queue_num]);
		fflush(fax_fp);
	*/
	
		return( p );
	}
	p->previous->next = head->next;
	head->next->previous = p->previous;
	*o_num = *n_queue_node[QUEUE_CLIENT_READ] + *n_queue_node[QUEUE_CLIENT_WRITE];
	pthread_mutex_unlock( &queue_mutex[queue_num] );

	
	return (p);
}



LISTSMG * CQUEUE_List::GetNoBlockNode( int queue_num ,int *o_num )
{
	LISTSMG *p,*head;

	/*
	 */
	if ( queue_num <0 || queue_num >= MAXQUEUE )
	{
		return NULL;
	}

	/*
	 */
	pthread_mutex_lock( &queue_mutex[queue_num] );
	if ( queue_head[queue_num]->next == NULL )
	{			
		*o_num = 0;
		pthread_mutex_unlock( &queue_mutex[queue_num] );
		return NULL;
	}

	(*n_queue_node[queue_num])--;
	/*
	 */
	head = queue_head[queue_num];
	p = head->next->previous;

	if( p == head->next )
	{
		head->next = NULL;
		head->previous = NULL;
		*o_num = *n_queue_node[queue_num];
		pthread_mutex_unlock( &queue_mutex[queue_num] );       

		if(QUEUE_FREE == queue_num) {
    	    p->Reset();
    	}
    	
		return( p );
	}
	p->previous->next = head->next;
	head->next->previous = p->previous;
	*o_num = *n_queue_node[queue_num];
	pthread_mutex_unlock( &queue_mutex[queue_num] );

	if(QUEUE_FREE == queue_num) {
	    p->Reset();
	}

	return (p);
}

int CQUEUE_List::SetNode( LISTSMG * p,int queue_num )
{
	LISTSMG *q,*head;

    if(QUEUE_SEND == queue_num && p->connfd > 0) 
    {
        queue_num = QUEUE_SEND;//2019.8.29  + p->connfd%cfg.n_send_threads;
    }
    
	if ( p == NULL || queue_num <0 || queue_num >= MAXQUEUE )
	{
	    //LOG(LT_ERROR, "WANING: set node to queue queue_id=%d", queue_num);
		return 1;
	}

	p->next = NULL;
	p->previous = NULL;
	

	if ( queue_num == QUEUE_FREE)	
	{
		//p->Reset();

		if ( p->cAddBuffer != NULL)
		{
			free(p->cAddBuffer);
			p->cAddBuffer = NULL;
		}
		if (p->pRespBuffer != NULL)
		{
			free(p->pRespBuffer);
			p->pRespBuffer = NULL;
		}
	}


	/*
	 */
	pthread_mutex_lock( &queue_mutex[queue_num] );

	(*n_queue_node[queue_num])++;
	/*
	 */
	head = queue_head[queue_num];
	if( head->next != NULL )
	{
		q = head->next;
		q->previous->next = p;
		p->previous = q->previous;
		q->previous = p;
		p->next = q;
		head->next = p;
	}
	else
	{
		head->next = p;
		p->next = p;
		p->previous = p;
	}

	pthread_cond_signal( &queue_cond[queue_num] );
	pthread_mutex_unlock( &queue_mutex[queue_num] );
	return 0;
}



void CQUEUE_List::GetQueueNum( QUEUE_DATA *in_data )
{
	int i;

	in_data->packet_num = n_total_node;
	in_data->hash_num   = n_hash_node;

    for(i = 0; i<MAXQUEUE; i++ )
    {
        in_data->queue[i] = *n_queue_node[i];
    }
	
	return ;
}

