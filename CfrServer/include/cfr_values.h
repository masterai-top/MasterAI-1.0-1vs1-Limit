#ifndef _CFR_VALUES_H_
#define _CFR_VALUES_H_

#include <memory>
#include <string>

#include "action_tree.h"
#include "cfr_street_values.h"
#include "global_def.h"
#include "error_code.h"
#include <mutex>
#include "s_game.h"

class ActionTree;
class ActionTrees;
class ClusterHands;
class Node;

class CFRValues
{
public:
  CFRValues(const bool *players, const bool *streets, int root_bd, int root_bd_st,
            const ClusterHands &buckets, const ActionTree *betting_tree);
  virtual ~CFRValues(void);
  AbstractCFRStreetValues *StreetValues(int st) const { return street_values_[st]; }
  void CreateStreetValues(int st, CFRValueType value_type, bool quantize);
  int Read_MultiThread(const char *dir, const std::vector<int> &its, const ActionTree *betting_tree,
                        const std::string &action_sequence, int only_p, const std::vector<bool> &sumprobs, bool quantize);

  void GetOneStrategyVals(int st, int p, int nt, int offset, int num_succs,unsigned int *vals) const
  {
    street_values_[st]->GetOneStrategyVals(p, nt, offset, num_succs, vals);
  }

  void ReadNode(Node *node, Reader *reader, void *decompressor)
  {
    street_values_[node->Street()]->ReadNode(node, reader, decompressor);
  }

  bool Player(int p) const { return players_[p]; }
  bool Street(int st) const { return streets_[st]; }
  int NumHoldings(int st) const { return num_holdings_[st]; }
  int RootSt(void) const { return root_bd_st_; }
  int RootBd(void) const { return root_bd_; }
  int NumNonterminals(int st,int p) const
  {
    int index = p * (S_Game::MaxStreet()+1) + st;
    return num_nonterminals_[index];
  }

protected:
  void Read(Node *node, Reader ***readers, void ***decompressors, int p);
  void ReadSingleFile_Task(Node *node, Reader ***readers, void ***decompressors, int p, int st);
  void ReadSingleFile(Node *node, Reader ***readers, void ***decompressors, int p, int st);
  Reader *InitializeReader(const char *dir, int p, int st, int it,
                           const std::string &action_sequence, int root_bd_st, int root_bd,
                           bool sumprobs, CFRValueType *value_type);
  void Initialize(const bool *players, const bool *streets, int root_bd, int root_bd_st,
                  const ClusterHands &buckets);

  std::unique_ptr<AbstractCFRStreetValues *[]> street_values_;
  std::unique_ptr<bool[]> players_; 
  std::unique_ptr<bool[]> streets_;
  int root_bd_;
  int root_bd_st_;
  std::unique_ptr<int[]> num_holdings_;     
  std::unique_ptr<int[]> num_nonterminals_; 
  std::mutex readFile_lock_;
  int file_count_;
};

#endif
