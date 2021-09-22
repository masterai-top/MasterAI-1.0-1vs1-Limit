#ifndef _DEF_H
#define _DEF_H

#include <vector>
#include <string.h>
#include <stdio.h>
#include <string>

// 固定的一些数据
const std::vector<int> gNumCodes = {0,140608,7311616,380204032};//每一轮的code的数(0,52的3次方,52的四次方,52的5次方)
const std::vector<unsigned int> gNumHands = {1326U,2063880U,71281704U,2761583136U};//每一轮的手牌数(bdCount*hcpCount) 

const int NUM_STREET                                = 4;
const int MAX_BOADRD_NUM                            = 5;
const int NUM_HCP                                   = 2;
const int NUM_PLAYER                                = 2;
const int NUM_CARDS                                 = 52;
const std::vector<int> STREET_BOARD_CNT             = {0,3,4,5};

const std::vector<std::string> STREET_NAME          = {"preflop","flop","turn","river"};

const std::string LABEL_FILTER_FLAG                 = "filter_flag";
const std::string LABEL_SUMPROB_FLAG                = "sumprob_flag";
const std::string LABEL_FRM_TH                      = "frm_th";
const std::string LABEL_POT_TH                      = "pot_th";
const std::string LABEL_RULE_TYPE                   = "rule_type";
const std::string LABEL_HANDLE_NFRM                 = "handle_nfrm";


const std::string LABEL_STREET_SEPATOR              = ",";


struct LogInfo
{
    LogInfo()
    {
        trans_id = "null";
        role_id = -1;
        hand_id = -1;
    }
    LogInfo(std::string trans_id_,int role_id_,int hand_id_)
    {
        trans_id = trans_id_;
        role_id = role_id_;
        hand_id = hand_id_;
    }

    std::string trans_id;
    int role_id;
    int hand_id;
};


struct ActionParam
{
    ActionParam()
    {
        for(int i = 0; i < NUM_STREET; i++) {
            filter_flags[i] = true;
            handle_nfrom[i] = false;
            rule_types[i] = 0;
            frm_ths[i] = 1.0;
            pot_ths[i] = 3.0;
        }
    }
    
    bool filter_flags[NUM_STREET];
    bool handle_nfrom[NUM_STREET];
    int  rule_types[NUM_STREET];
    double frm_ths[NUM_STREET];
    double pot_ths[NUM_STREET];

};



struct ActionInfo
{
    ActionInfo()
    {
        for (int i = 0; i < MAX_BOADRD_NUM; i++)
        {
            board_cards[i] = 99;
        }
        for (int i = 0; i < NUM_HCP; i++)
        {
            hcp_cards[i] = 99;
        }
        action_seq = "null";
        st = -1;
        bet_to = 0;
        num_street_bet = -1;
    }

    std::string ToString() const {
        char szBuf[128] = {0};
        snprintf(szBuf, sizeof(szBuf)-1,"bc:%.02d,%.02d,%.02d,%.02d,%.02d  hc:%.02d,%.02d, st:%d, bet_to:%d, num_bet:%d, seq:%s",
            board_cards[0], board_cards[1], board_cards[2], board_cards[3], board_cards[4],
            hcp_cards[0], hcp_cards[1], st, bet_to, num_street_bet, action_seq.c_str());
        return std::string(szBuf);
    }

    int FromString(const char *szStr) {
        const char *p = strstr(szStr, "bc:");
        if(NULL == p || strlen(p) < 20) {
            return -1;
        }

        p += 3;
        for(int i = 0; i < MAX_BOADRD_NUM; i++) {
            board_cards[i] = std::atoi(p);
            p += 3;
        }

        p = strstr(p, "hc:");
        if(NULL == p || strlen(p) < 10) {
            return -2;
        }

        p += 3;
        for(int i = 0; i < NUM_HCP; i++) {
            hcp_cards[i] = std::atoi(p);
            p += 3;
        }

        p = strstr(p, "st:");
        if(NULL == p) {
            return -3;
        }
        p += 3;
        st = std::atoi(p);

        p = strstr(p, "bet_to:");
        if(NULL == p) {
            return -4;
        }
        p += 7;
        bet_to = std::atoi(p);

        p = strstr(p, "num_bet:");
        if(NULL == p) {
            return -5;
        }
        p += 8;
        num_street_bet = std::atoi(p);

        p = strstr(p, "seq:");
        if(NULL == p) {
            return -6;
        }
        p += 4;
        action_seq = p;

        return 0;
    }
    
    int board_cards[MAX_BOADRD_NUM];
    int hcp_cards[NUM_HCP];
    int st;
    std::string action_seq;
    int bet_to;
    int num_street_bet;
};

struct CFRKey
{
    CFRKey()
    {
        st = -1;
        node_id = -1;
        pa = -1;
        offset = -1;
        num_succ = -1;
    }
    int st;
    int node_id;
    int pa;
    int offset;
    int num_succ;
};

struct ModelParam
{
    ModelParam()
    {
        game_param_path    = "";
        card_param_path    = "";
        static_dir         = "";
        random_flag        = false;
        for(int i = 0; i < NUM_STREET; i++) {
            sumprob_flag[i] = false;
        }
    }
    std::string game_param_path;
    std::string card_param_path;
    std::string static_dir;
    std::string cfr_param;              //透传给cfrsvr的参数
    bool random_flag;
    bool sumprob_flag[NUM_STREET];   
};

class NodeInfo
{
public:
    NodeInfo(void){ _node_id = -1; }
    NodeInfo(int node_id, unsigned char street, unsigned char acting_player,
             const std::vector<std::string> &child_node)
    {
        _node_id = node_id;
        _street = street;
        _acting_player = acting_player;
        _child_node = child_node;
    }
    ~NodeInfo(){}

    void SetNodeId(int node_id) {_node_id = node_id;}
    void SetStreet(unsigned char street) {_street = street; }
    void SetActingPlayer(unsigned char acting_player) {_acting_player = acting_player;}
	void SetChildNode(const std::vector<std::string>& child_node) {_child_node = child_node;}

    int GetNodeId() const { return _node_id;}
    int GetStreet() const { return _street;}
    int GetActingPlayer() const { return _acting_player;}
    int GetChildNodeCount() const {return _child_node.size();}
    std::vector<std::string> GetChildNode() const {return _child_node;}


private:
    unsigned char _street;
    unsigned char _acting_player;
    int _node_id;
    std::vector<std::string> _child_node;
};


#endif