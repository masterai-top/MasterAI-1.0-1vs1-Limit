#ifndef _BOARD_TREE_H_
#define _BOARD_TREE_H_

#include <memory>

#include "cards.h"
#include "s_game.h"

class PCardTree {
public:
  static void Create(void);
  static void Delete(void);
  static const Card *Board(int st, int bd)
  {
    if (st == 0)
      return NULL;
    int num_board_cards = S_Game::NumBoardCards(st);
    return &boards_[st][bd * num_board_cards];
  }
  static int SuitGroups(int st, int bd) {return suit_groups_[st][bd];}
  static int NumVariants(int st, int bd) { return board_variants_[st][bd]; }
  static int LocalIndex(int root_st, int root_bd, int st, int gbd);
  static int GlobalIndex(int root_st, int root_bd, int st, int lbd);
  static int NumLocalBoards(int root_st, int root_bd, int st);
  static int NumBoards(int st) { return num_boards_[st]; }
  static int SuccBoardBegin(int root_st, int root_bd, int st)
  {
    return succ_board_begins_[root_st][st][root_bd];
  }
  static int SuccBoardEnd(int root_st, int root_bd, int st) {
    return succ_board_ends_[root_st][st][root_bd];
  }
  static void CreateLookup(void);
  static void DeleteLookup(void);
  static int LookupBoard(const Card *board, int st);
  static int LookupBoard_SBot(const Card *board,int st,int &bdIdx);
  static void BuildBoardCounts(void);
  static void DeleteBoardCounts(void);
  static int BoardCount(int st, int bd) { return board_counts_[st][bd]; }
  static void BuildPredBoards(void);
  static void DeletePredBoards(void);
  static int PredBoard(int msbd, int pst) {
    return pred_boards_[msbd * max_street_ + pst];
  }

  static int GetLookUp(int st,int code)
  {
    return lookup_[st][code];
  }

private:
  PCardTree(void) {}
  
  static void Count(int st, const Card *prev_board, int prev_sg);
  static void Build(int st, const std::unique_ptr<Card []> &prev_board, int prev_sg);
  static void DealRawBoards(Card *board, int st);
  static void BuildPredBoards(int st, int *pred_bds);

  static int max_street_;
  static std::unique_ptr<int[]> num_boards_;//[max_street_+1],每一轮的board数,等效的board归为一类
  static int **board_variants_;//[max_street_+1][num_boards],每一轮的每个board有多少个等效的board
  //这两个变量指示的是当前轮次的bd在接下来轮次的bd的起始范围
  static int ***succ_board_begins_;//[max_street_][max_street_+1][num_boards]
  static int ***succ_board_ends_;//[max_street_][max_street_+1][num_boards]
  static Card **boards_;//[max_street_+1][num_boards * num_board_cards],每一轮,每个bd对应的具体board
  static int **suit_groups_;//[max_street_ + 1][num_boards]
  static int *bds_;//[max_street_+1]
  static int **lookup_;//[max_street_+1][num_codes],通过原始board寻找对应的等效bd
  static int **board_counts_;
  static int *pred_boards_;//[num_ms_boards * max_street_]
};

#endif
