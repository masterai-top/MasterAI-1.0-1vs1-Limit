#ifndef _CFR_STREET_VALUES_H_
#define _CFR_STREET_VALUES_H_

#include <memory>

#include "global_def.h"

class ClusterHands;
class Node;
class Reader;
class Writer;

class AbstractCFRStreetValues {
 public:
  virtual ~AbstractCFRStreetValues(void) {}
  virtual void AllocateAndClear(Node *node, int p) = 0;
  virtual void GetOneStrategyVals(int p, int nt, int offset,  int num_succs, unsigned int *vals) const = 0;
  virtual bool Players(int p) const = 0;
  virtual void ReadNode(Node *node, Reader *reader, void *decompressor) = 0;
  virtual CFRValueType MyType(void) const = 0;
};

template <typename T>
class CFRStreetValues : public AbstractCFRStreetValues {
public:
  CFRStreetValues(int st, const bool *players, int num_holdings, int *num_nonterminals,
		  CFRValueType file_value_type);
  virtual ~CFRStreetValues(void);
  CFRValueType MyType(void) const;
  int Street(void) const {return st_;}
  bool Players(int p) const {return players_[p];}
  int NumHoldings(void) const {return num_holdings_;}
  int NumNonterminals(int p) const {return num_nonterminals_[p];}
  T *AllValues(int p, int nt) const {return data_[p] ? data_[p][nt] : nullptr;}
  void AllocateAndClear(Node *node, int p);
  void GetOneStrategyVals(int p, int nt, int offset,  int num_succs, unsigned int *vals) const;
  void Set(int p, int nt, int h, int num_succs, T *vals);
  void InitializeValuesForReading(int p, int nt, int num_succs);
  void ReadNode(Node *node, Reader *reader, void *decompressor);
protected:
  void AllocateAndClear2(Node *node, int p);
  unsigned char ***GetUnsignedCharData(void);
  
  int st_;
  std::unique_ptr<bool []> players_;
  int num_holdings_;
  std::unique_ptr<int []> num_nonterminals_;
  T ***data_;
  CFRValueType file_value_type_;
};

#endif
