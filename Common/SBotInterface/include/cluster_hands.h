#ifndef _BUCKETS_H_
#define _BUCKETS_H_

#include <memory>

class CardAbstraction;

class ClusterHands {
public:
  ClusterHands(const CardAbstraction &ca, bool numb_only);
  ClusterHands(void);
  ~ClusterHands(void);
  bool None(int st) const { return none_[st]; }
  int GetClusterIdx(int st, unsigned int h) const
  {
    if (short_flag_[st])
    {
      return (int)short_flag_[st][h];
    }
    else
    {
      return int_cluster_[st][h];
    }
  }
  const int *NumClusters(void) const {return num_cluster_.get();}
  int NumClusters(int st) const {return num_cluster_[st];}
private:
  std::unique_ptr<bool []> none_;
  unsigned short **short_flag_;
  int **int_cluster_;
  std::unique_ptr<int []> num_cluster_;
};

#endif
