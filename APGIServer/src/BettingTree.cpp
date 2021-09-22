#include "pch.h"
#include "BettingTree.h"
#include "comm_define.h"
#include "string_utils.h"
#include <fstream>
#include <algorithm>


CBettingTree::CBettingTree()
{
}

CBettingTree::~CBettingTree()
{
    m_oTaskBTRedis.Restore();
}

bool  CBettingTree::Init(uint32 nBTRecords, const string &strBTRedis, const string &strBTRedisPass, uint32 nTaskRedisCnt, const string &strBTFile)
{
    if(0 == nBTRecords || 0 == nTaskRedisCnt || 0 == strBTFile.length())
    {
        return false;
    }

    m_nBTRecords = nBTRecords;
    m_nInitState = 0;
    m_strBTFile  = strBTFile;

    /*try{
        if(!m_oTaskBTRedis.Init(strBTRedis, nTaskRedisCnt, strBTRedisPass))
        {
            printf("INIT_BTREE| task_redis init failed\r\n");
            LOG(LT_ERROR, "INIT_BTREE| task_redis init failed! redis=%s", strBTRedis.c_str());
            return false;
        }
    }
    catch(Exception &e)
    {
        LOG(LT_ERROR, "INIT_BTREE| task_redis init exception| msg=%s\r\n", e.what());
        printf("INIT_BTREE| task_redis init exception| task_redis=%s| msg=%s\r\n", strBTRedis.c_str(), e.what());
        return false;
    }
    catch(...)
    {
        LOG(LT_ERROR, "INIT_BTREE| task_redis init exception all\r\n");
        printf("INIT_BTREE| task_redis init exception all\r\n");
        return false;
    }*/

    pthread_t tid = 0;
    if(0 != pthread_create(&tid, NULL, &CBettingTree::InitBTreeIt, (void *)this))
    {
        fprintf(stderr, "INIT_BTREE| init| create thread failed\r\n");
        LOG(LT_ERROR, "INIT_BTREE| Init| create thread failed");
        return false;
    }


    LOG(LT_INFO, "INIT_BTREE| init succ| init_state=%d| bt_records=%u| bt_file=%s", m_nBTRecords, m_nInitState, m_strBTFile.c_str());

    return true;
}

int CBettingTree::GetNode(const char *szTransID, const string &strActionSeq, NodeInfo &node)
{
    string sk;
    string sv;
    if(0 == strActionSeq.length())
    {
        sk = g_strBTKeyPre + "root";
    }
    else
    {
        sk = g_strBTKeyPre + strActionSeq;
    }

    //初始化完成,从内存查找
    if(1 == m_nInitState)
    {
        unordered_map<string, NodeInfo>::iterator it = m_BTHashTable.find(sk);
        if(it == m_BTHashTable.end())
        {
            LOG(LT_ERROR_TRANS, szTransID, "Local Get betting tree node failed| k=%s", sk.c_str());
            return -106;
        }

        node = it->second;
        LOG(LT_DEBUG_TRANS, szTransID, "Get betting tree from memory| k=%s", sk.c_str());

        return 0;
    }

    LOG(LT_ERROR_TRANS, szTransID, "Get betting tree| not init| k=%s", sk.c_str());
    return -116;

    // -------- 从redis查找 --------
    if(!m_oTaskBTRedis.GetByKey(sk, sv))
    {
        LOG(LT_ERROR_TRANS, szTransID, "Redis Get allow actions failed| k=%s", sk.c_str());
        return -118;
    }

    int err = ParseStrToNode(sv, node);
    if(0 != err)
    {
        LOG(LT_ERROR_TRANS, szTransID, "Redis to node failed| k=%s| v=%s| err=%d", sk.c_str(), sv.c_str(), err);
        return -119;
    }

    LOG(LT_DEBUG_TRANS, szTransID, "Get betting tree from redis| k=%s| v=%s", sk.c_str(), sv.c_str());

    return 0;
}



int CBettingTree::ParseStrToNode(const std::string &info_str, NodeInfo &node)
{
    std::vector<std::string> split_str;
    StringUtils::SplitString(info_str, split_str, ",");
    if (split_str.size() < 4)
    {
        LOG(LT_ERROR, "Err Info Str| info=%s |err=%d", info_str.c_str(), -1);
        return -1;//ERR info_str
    }

    int vals[4];
    for (int i = 0; i < 4; i++)
    {
        if (sscanf(split_str[i].c_str(), "%d", &(vals[i])) != 1)
        {
            LOG(LT_ERROR, "Err Info Str| info=%s |err=%d", info_str.c_str(), -2);
            return -2;//Err Str
        }
    }
    int node_id  = vals[0];
    int street   = vals[1];
    int num_succ = vals[2];
    int pa       = vals[3];
    if (int(split_str.size()) < 4 + num_succ)
    {
        LOG(LT_ERROR, "Err Info Str| info=%s |err=%d", info_str.c_str(), -3);
        return -3; //No Enough vals
    }
    std::vector<std::string> child_node(split_str.begin() + 4, split_str.begin() + 4 + num_succ);
    node = NodeInfo(node_id, street, pa, child_node);
    return 0;
}


void *CBettingTree::InitBTreeIt(void *arg)
{
    pthread_detach( pthread_self() );
    CBettingTree *pThis = (CBettingTree *)arg;

    LOG(LT_INFO, "INIT_BTREE_IT thread start| pid=%u", pthread_self());
    NodeInfo oNodeInfo;
    int err;

    for ( ;; )
    {
        //已成功初始化
        if(1 == pThis->m_nInitState)
        {
            //是否需要退出线程?
            LOG(LT_INFO, "INIT_BTREE_IT| already init succ| records=%u", pThis->m_BTHashTable.size());
            sleep(60);
            continue;
        }

        LOG(LT_INFO, "INIT_BTREE_IT| start init | file=%s", pThis->m_strBTFile.c_str());
        const std::string SEPARATOR = ",";
        std::string line;
        std::ifstream in_file(pThis->m_strBTFile);
        if (!in_file)
        {
            LOG(LT_ERROR, "INIT_BTREE_IT| open file failed| flie=%s", pThis->m_strBTFile.c_str());
            sleep(10);
            _exit(0);
        }

        // line:actioseq,node_id,street,num_succ,pa,child_node...
        int cnt = 0;
        while (std::getline(in_file, line))
        {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
            size_t pos = line.find(SEPARATOR);
            if (pos == std::string::npos)
            {
                LOG(LT_ERROR, "INIT_BTREE_IT| Invalid line| record=%s", line.c_str());
                sleep(10);
                _exit(0);
            }

            //Key加上前缀
            std::string strActionKey    = line.substr(0, pos);
            std::string strActionNode   = line.substr(pos + 1);
            if (strActionKey == "")
            {
                strActionKey = "root";
            }
            strActionKey = g_strBTKeyPre + strActionKey;

            //更新值
            if(0 != (err = pThis->ParseStrToNode(strActionNode, oNodeInfo)))
            {
                LOG(LT_ERROR, "INIT_BTREE_IT| betting tree to node failed| k=%s| v=%s| err=%d", strActionKey.c_str(), strActionNode.c_str(), err);
                continue;
            }


            pThis->m_BTHashTable.insert(std::pair<string, NodeInfo>(strActionKey, oNodeInfo));
            cnt++;

            LOG(LT_INFO, "INIT_BTREE_IT| the %.08d record| key=%s| value=%s", cnt, strActionKey.c_str(), strActionNode.c_str());
        }

        //记录数校验
        if(pThis->m_nBTRecords != pThis->m_BTHashTable.size())
        {
            LOG(LT_ERROR, "INIT_BTREE_IT| records invalid| need_cnt=%u| real_cnt=%u", pThis->m_nBTRecords, pThis->m_BTHashTable.size());
            sleep(10);
            _exit(0);
        }

        pThis->m_nInitState = 1;
        LOG(LT_INFO, "INIT_BTREE_IT| already init succ| records=%u", pThis->m_BTHashTable.size());
    }
}




