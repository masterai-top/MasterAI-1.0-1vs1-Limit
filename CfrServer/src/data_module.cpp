#include "data_module.h"

#include "error_code.h"
#include "global_def.h"
#include "conf.h"
#include "pch.h"
#include "s_game.h"

// S_Bot Including begin
#include "files.h"
#include "param_factory.h"
#include "s_game.h"
#include "p_card_tree.h"
#include "abstraction_def.h"
#include "help_functions.h"
#include "cluster_hands.h"
#include "cfr_values.h"
// S_Bot Including end

#include "json/json.h"

#include <stdio.h>
#include <time.h>
#include "param_factory.h"

std::shared_ptr<CFRValues> DataModule::cfr_values;
bool DataModule::_init_flag = false;

int DataModule::Init()
{
    LOG(LT_INFO, DataModuleName.c_str(), "Init Data Module Begin");

    if (CConf::Instance()->m_bRandomFlag)
    {
        LOG(LT_INFO, DataModuleName.c_str(), "Init Random Model");
        _init_flag = true;
        return ERR_OK;
    }


    LOG(LT_INFO, DataModuleName.c_str(), "Setting Files");
    Files::Init(CConf::Instance()->m_strCfrDir, CConf::Instance()->m_strStaticDir);

    // 初始化game
    LOG(LT_INFO, DataModuleName.c_str(), "Setting Game");
    std::unique_ptr<Params> game_params = ParamsFactory::CreateGameParams();
    game_params->ReadFromFile(CConf::Instance()->m_strGameParamPath.c_str());
    int flag = S_Game::Initialize(*game_params);
    if(flag != ERR_OK)
    {
        LOG(LT_ERROR, DataModuleName.c_str(), "Init Game Failed err =%d!", flag);
        return flag;
    }

    // build betting Trees
    LOG(LT_INFO, DataModuleName.c_str(), "Building Betting Trees");
    std::unique_ptr<Params> betting_params = ParamsFactory::CreateBettingAbstractionParams();
    betting_params->ReadFromFile(CConf::Instance()->m_strBettingParamPath.c_str());
    std::unique_ptr<ActionAbstraction> betting_abstraction;
    betting_abstraction.reset(new ActionAbstraction(*betting_params));
    std::unique_ptr<ActionTrees> betting_trees;
    betting_trees.reset(new ActionTrees(*betting_abstraction));

    // boardTree 建立
    LOG(LT_INFO, DataModuleName.c_str(), "Building Board Tree");
    PCardTree::Create();
    PCardTree::CreateLookup();

    //初始化card
    LOG(LT_INFO, DataModuleName.c_str(), "Building ClusterHands");
    std::unique_ptr<Params> card_params = ParamsFactory::CreateCardAbstractionParams();
    card_params->ReadFromFile(CConf::Instance()->m_strCardParamPath.c_str());
    std::unique_ptr<CardAbstraction> card_abstraction;
    card_abstraction.reset(new CardAbstraction(*card_params));
    std::shared_ptr<ClusterHands> buckets;
    buckets.reset(new ClusterHands(*card_abstraction, false));

    // 初始化CFR
    LOG(LT_INFO, DataModuleName.c_str(), "Reading Cfr");
    std::unique_ptr<Params> cfr_params = ParamsFactory::CreateCFRParams();
    cfr_params->ReadFromFile(CConf::Instance()->m_strCfrParamPath.c_str());
    std::unique_ptr<CFRAbstraction> cfr_config;
    cfr_config.reset(new CFRAbstraction(*cfr_params));
    cfr_values.reset(new CFRValues(nullptr, nullptr, 0, 0, *buckets,
                                   betting_trees->GetBettingTree()));

    char dir[500];
    sprintf(dir, "%s/%s.%u.%s.%u.%u.%u.%s.%s", Files::CFRBase(),
            S_Game::GameName().c_str(), S_Game::NumPlayers(),
            (*card_abstraction).CardAbstractionName().c_str(),
            S_Game::NumRanks(), S_Game::NumSuits(), S_Game::MaxStreet(),
            (*betting_abstraction).ActionAbstractionName().c_str(),
            (*cfr_config).CFRAbstractionName().c_str());
    // Note assumption that we can use the betting tree for position 0
    LOG(LT_INFO, DataModuleName.c_str(), "Start Reading CFV from %s", dir);
    flag = cfr_values->Read_MultiThread(dir, CConf::Instance()->m_vecIter,
                                        betting_trees->GetBettingTree(), "x", -1, CConf::Instance()->m_vecSumprobFlag, false);

    if(flag != ERR_OK)
    {
        LOG(LT_ERROR, DataModuleName.c_str(), "Read CFR File FAILED code=%d!", flag);
        return flag;
    }

    LOG(LT_INFO, DataModuleName.c_str(), "Init Data Module Done");
    _init_flag = true;
    return ERR_OK;
}

void DataModule::GetVals(const std::string &request_str, std::string &resp_str)
{
    if (!_init_flag)
    {
        resp_str = JointStr("Error", "Not Init Yet", "null");
        return;
    }


    Json::Reader reader;
    Json::Value value;
    if (reader.parse(request_str.c_str(), value))
    {
        int mode = value["mode"].asInt();
        if (mode == 1)
        {
            int nt = value["nt"].asInt();
            int st = value["st"].asInt();
            int pa = value["pa"].asInt();
            int offset = value["offset"].asInt();
            int num_succ = value["num_succ"].asInt();
            std::string version_buf = value["version"].asString();
            if(!CheckVersion(version_buf))
            {
                resp_str = JointStr("Error", "Mismatch Version", "null");
                return;
            }
            if (CConf::Instance()->m_bRandomFlag)
            {
                std::string random_str = GenRandomValStr(num_succ);
                resp_str = JointStr("OK", "null", random_str);
            }
            else
            {
                int flag = CheckKeys(nt, st, pa, offset, num_succ);
                if (flag != ERR_OK)
                {
                    resp_str = JointStr("Error", "Invalid Key", "null");
                }
                else
                {
                    std::vector<unsigned int> vals;
                    vals.resize(num_succ);
                    cfr_values->GetOneStrategyVals(st, pa, nt, offset, num_succ, vals.data());

                    std::string val_str = ConvertValToStr(vals);
                    resp_str = JointStr("OK", "null", val_str);
                }
            }
        }
        else
        {
            resp_str = JointStr("Error", "Wrong Mode", "null");
        }
    }
    else
    {
        resp_str = JointStr("Error", "Can't parse Recv Msg", "null");
    }
}

bool DataModule::CheckVersion(const std::string &version_str)
{
    if (version_str == CConf::Instance()->m_strFinalVersion)
    {
        return true;
    }
    else
    {
        return false;
    }

}

int DataModule::CheckKeys(int nt, int st, int pa, int offset, int num_succ)
{
    if(st < 0 || st > S_Game::MaxStreet())
    {
        return ERR_INVALID_KEY;
    }
    if (pa < 0 || pa >= S_Game::NumPlayers())
    {
        return ERR_INVALID_KEY;
    }

    int max_nt = cfr_values->NumNonterminals(st, pa);
    if (nt < 0 || nt >= max_nt)
    {
        return ERR_INVALID_KEY;
    }

    int num_buckets = cfr_values->NumHoldings(st);
    int max_idx = num_buckets * num_succ;
    if (offset < 0 || (offset + num_succ > max_idx))
    {
        return ERR_INVALID_KEY;
    }

    return ERR_OK;
}

std::string DataModule::ConvertValToStr(const std::vector<unsigned int> &vals)
{
    std::string val_str;
    int num_succ = int(vals.size());
    for (int i = 0; i < num_succ; i++)
    {
        val_str += std::to_string(vals[i]);
        if (i < num_succ - 1)
        {
            val_str += ",";
        }
    }
    return val_str;
}

std::string DataModule::GenRandomValStr(int num_succ)
{
    static unsigned int val = 0;
    srand(val++);
    int rand_idx = rand() % (num_succ);

    std::string val_str;
    for (int i = 0; i < num_succ; i++)
    {
        if (i == rand_idx)
        {
            val_str += std::to_string(0);
        }
        else
        {
            val_str += std::to_string(i * 100 + 100);
        }
        if (i < num_succ - 1)
        {
            val_str += ",";
        }
    }
    return val_str;
}

std::string DataModule::JointStr(const std::string &res,
                                 const std::string &note, const std::string &val_str)
{
    char buf[1024] = {0};
    snprintf(buf, sizeof(buf), "{\"res\":\"%s\", \"note\":\"%s\", \"vals\":\"%s\"}",
             res.c_str(), note.c_str(), val_str.c_str());
    return std::string(buf);
}