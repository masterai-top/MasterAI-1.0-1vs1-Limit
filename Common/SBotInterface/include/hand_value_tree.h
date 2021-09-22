#ifndef _HAND_VALUE_TREE_H_
#define _HAND_VALUE_TREE_H_

#include "cards.h"

class HandValueTree {
public:
  static void Create(void);
  static void CreateAllTrees();
  static void Delete(void);
  static void DeleteAllTrees();
  static bool Created(void);
  static int Val(const Card *cards);
  static int ValByCount(const Card *cards,int count);
  static int Val(const int *board, const int *hole_cards);
  static int DiskRead(Card *cards);

  static int GetRiverVal(const int *board,const int *hole_cards);

  static std::string GetTurnVal(const int *board,const int *hole_cards,const std::vector<double> &samplePoints);
private:
  HandValueTree(void) {}

  static void ReadOne(void);
  static void ReadTwo(void);
  static void ReadThree(void);
  static void ReadFour(void);
  static void ReadFive(void);
  static void ReadSix(void);
  static void ReadSeven(void);

  static int num_board_cards_;
  static int num_cards_;
  static int *tree1_;
  static int **tree2_;
  static int ***tree3_;
  static int ****tree4_;
  static int *****tree5_;
  static int ******tree6_;
  static int *******tree7_;
};

#endif
