/*
Copyright (C) 2011 by the Computer Poker Research Group, University of Alberta
*/

#ifndef _GAME_H
#define _GAME_H
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "rng.h"
#include "stdbool.h"


#define VERSION_MAJOR 2
#define VERSION_MINOR 0
#define VERSION_REVISION 0


#define MAX_ROUNDS 4
#define MAX_PLAYERS 10
#define MAX_BOARD_CARDS 7
#define MAX_HOLE_CARDS 3
#define MAX_NUM_ACTIONS 64
#define MAX_SUITS 4
#define MAX_RANKS 13
#define MAX_LINE_LEN 4096

#define NUM_ACTION_TYPES 3

enum BettingType { limitBetting, noLimitBetting };
enum ActionType { a_fold = 0, a_call = 1, a_raise = 2,
		  a_invalid = NUM_ACTION_TYPES };

typedef struct {
  enum ActionType type; /* is action a fold, call, or raise? */
  int32_t size; /* for no-limit raises, we need a size
		   MUST BE 0 IN ALL CASES WHERE IT IS NOT USED */
} Action;

typedef struct {

  /* stack sizes for each player */  //每个玩家的最大筹码
  int32_t stack[ MAX_PLAYERS ];

  /* entry fee for game, per player */ //每位玩家游戏的入场费
  int32_t blind[ MAX_PLAYERS ];

  /* size of fixed raises for limitBetting games */ //限注, 固定加注大小(每轮设置一个值)
  int32_t raiseSize[ MAX_ROUNDS ];

  /* general class of game, 限注&不限注 */ 
  enum BettingType bettingType;  

  /* number of players in the game 玩家数*/
  uint8_t numPlayers;

  /* number of betting rounds 轮数*/
  uint8_t numRounds;

  /* first player to act in a round , 第一个参加一轮比赛的玩家(每轮开始投注的玩家?)*/
  uint8_t firstPlayer[ MAX_ROUNDS ];

  /* number of bets/raises that may be made in each round, 每轮可能进行的投注/加注次数 */
  uint8_t maxRaises[ MAX_ROUNDS ];

  /* number of suits and ranks in the deck of cards */
  uint8_t numSuits; //花色4
  uint8_t numRanks; //大小A-K

  /* number of private player cards, 私牌的数量 */
  uint8_t numHoleCards;

  /* number of shared public cards each round, 每轮公牌的数量 */
  uint8_t numBoardCards[ MAX_ROUNDS ];
} Game;

typedef struct {
  uint32_t handId;

  /* largest bet so far, including all previous rounds, 迄今为止最大的赌注，包括之前的所有回合 */
  int32_t maxSpent;

  /* minimum number of chips a player must have spend in total to raise
     only used for noLimitBetting games , [noLimitBetting类型]玩家最小下注筹码 */
  int32_t minNoLimitRaiseTo;

  /* spent[ p ] gives the total amount put into the pot by player p , 某一玩家的投注筹码 */
  int32_t spent[ MAX_PLAYERS ];

  /* action[ r ][ i ] gives the i'th action in round r, 动作[r] [i]在第r轮给出第i个动作 */
  Action action[ MAX_ROUNDS ][ MAX_NUM_ACTIONS ];

  /* actingPlayer[ r ][ i ] gives the player who made action i in round r
     we can always figure this out from the actions taken, but it's
     easier to just remember this in multiplayer (because of folds) , actingPlayer [r] [i]给出了在第r轮中进行动作的玩家, 对应action值 */
  uint8_t actingPlayer[ MAX_ROUNDS ][ MAX_NUM_ACTIONS ];

  /* numActions[ r ] gives the number of actions made in round r , numActions [r]给出在round r中进行的动作的数量*/
  uint8_t numActions[ MAX_ROUNDS ];

  /* current round: a value between 0 and game.numRounds-1
     a showdown is still in numRounds-1, not a separate round */
  uint8_t round;

  /* finished is non-zero if and only if the game is over */
  uint8_t finished;

  /* playerFolded[ p ] is non-zero if and only player p has folded , 弃牌则为非0 */
  uint8_t playerFolded[ MAX_PLAYERS ];

  /* public cards (including cards which may not yet be visible to players) , 公牌*/
  uint8_t boardCards[ MAX_BOARD_CARDS ];

  /* private cards , 私牌 */
  uint8_t holeCards[ MAX_PLAYERS ][ MAX_HOLE_CARDS ];

  int32_t roundSpent[MAX_ROUNDS];       //各轮的下注值
} State;

typedef struct {
  State state;
  uint8_t viewingPlayer;        //brainsvr使用--机器人的位置
} MatchState;

void LogGame(const Game *game, char *logBuf, int len);

/* returns a game structure, or NULL on failure */
Game *readGame( FILE *file );

void printGame( FILE *file, const Game *game );

/* initialise a state so that it is at the beginning of a hand
   DOES NOT DEAL OUT CARDS */
void initState( const Game *game, const uint32_t handId, State *state );

/* shuffle a deck of cards and deal them out, writing the results to state */
void dealCards( const Game *game, rng_state_t *rng, State *state );

int statesEqual( const Game *game, const State *a, const State *b );

int matchStatesEqual( const Game *game, const MatchState *a,
		      const MatchState *b );

/* check if a raise is possible, and the range of valid sizes
   returns non-zero if raise is a valid action and sets *minSize
   and maxSize, or zero if raise is not valid */
int raiseIsValid( const Game *game, const State *curState,
		  int32_t *minSize, int32_t *maxSize );

/* check if can do fold action
   returns non-zero if final action/size is valid for state, 0 otherwise */
int isCanFold(const Game *game, const State *curState);

/* check if an action is valid
   if tryFixing is non-zero, try modifying the given action to produce
   a valid action, as in the AAAI rules.  Currently this only means
   that a no-limit raise will be modified to the nearest valid raise size

   returns non-zero if final action/size is valid for state, 0 otherwise */
int isValidAction( const Game *game, const State *curState,
		   const int tryFixing, Action *action );

/* record the given action in state
    does not check that action is valid */
void doAction( const Game *game, const Action *action, State *state, bool *bCurRoundEnd);

/* returns non-zero if hand is finished, zero otherwise */
#define stateFinished( constStatePtr ) ((constStatePtr)->finished)

/* get the current player to act in the state */
uint8_t currentPlayer( const Game *game, const State *state );

/* number of raises in the current round */
uint8_t numRaises( const State *state );

/* number of players who have folded */
uint8_t numFolded( const Game *game, const State *state );

/* number of players who have called the current bet (or initiated it)
   doesn't count non-acting players who are all-in */
uint8_t numCalled( const Game *game, const State *state );

/* number of players who are all-in */
uint8_t numAllIn( const Game *game, const State *state );

/* number of players who can still act (ie not all-in or folded) */
uint8_t numActingPlayers( const Game *game, const State *state );

/* get the index into array state.boardCards[] for the first board
   card in round (where the first round is round 0) */
uint8_t bcStart( const Game *game, const uint8_t round );

/*  get the total number of board cards dealt out after (zero based) round 
    round从 0-round 的公共牌
*/
uint8_t sumBoardCards( const Game *game, const uint8_t round );

/* get the total number of action
*/
uint16_t sumActions(const Game *game, const State *state);

//get the total spent
uint32_t sumSpent(const Game *game, const State *state);

/* return the value of a finished hand for a player
   returns a double because pots may be split when players tie
   WILL HAVE UNDEFINED BEHAVIOUR IF HAND ISN'T FINISHED
   (stateFinished(state)==0) */
double valueOfState( const Game *game, const State *state,
		      const uint8_t player );

/* returns number of characters consumed on success, -1 on failure
   state will be modified even on a failure to read */
int readState( const char *string, const Game *game, State *state );

/* returns number of characters consumed on success, -1 on failure
   state will be modified even on a failure to read */
int readMatchState( const char *string, const Game *game, MatchState *state );

/* print actions to a string
   returns number of characters printed to string, or -1 on failure
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printBetting2(const Game *game, const State *state, const int maxLen, char *string, char cDelimiter);

/* print a state to a string, as viewed by viewingPlayer
   returns the number of characters in string, or -1 on error
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printState( const Game *game, const State *state, const int maxLen, char *string);

/* print a state all msg to a string, as viewed by viewingPlayer
   returns the number of characters in string, or -1 on error
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printStateDetailed(const Game *game, const State *state, const int maxLen, char *string);

int printDetailedBetting( const Game *game, const State *state, const int maxLen, char *string );

/* print a state to a string, as viewed by viewingPlayer
   returns the number of characters in string, or -1 on error
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printMatchState( const Game *game, const MatchState *state,
		     const int maxLen, char *string );

/* read an action, returning the action in the passed pointer
   action and size will be modified even on a failure to read
   returns number of characters consumed on succes, -1 on failure */
int readAction( const char *string, const Game *game, Action *action );

/* print an action to a string
   returns the number of characters in string, or -1 on error
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printAction( const Game *game, const Action *action,
		 const int maxLen, char *string );

/* returns number of characters consumed, or -1 on error
   on success, returns the card in *card */
int readCard( const char *string, uint8_t *card );

/* read up to maxCards cards
   returns number of cards successfully read
   returns number of characters consumed in charsConsumed */
int readCards( const char *string, const int maxCards,
	       uint8_t *cards, int *charsConsumed );

/* print a card to a string
   returns the number of characters in string, or -1 on error
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printCard( const uint8_t card, const int maxLen, char *string );

/* print out numCards cards to a string
   returns the number of characters in string
   DOES NOT COUNT FINAL 0 TERMINATOR IN THIS COUNT!!! */
int printCards( const int numCards, const uint8_t *cards,
		const int maxLen, char *string );

int toPythonCards(const Game *game, const State *state, uint8_t player, char *cards, int len);

int printAllCards(const Game *game, const State *state, char *cards);

#define rankOfCard( card ) ((card)/MAX_SUITS)
#define suitOfCard( card ) ((card)%MAX_SUITS)
#define makeCard( rank, suit ) ((rank)*MAX_SUITS+(suit))

#endif
