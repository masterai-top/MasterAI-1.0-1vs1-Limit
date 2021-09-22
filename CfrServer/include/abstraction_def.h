#ifndef _ABSTRACTION_DEF_H_
#define _ABSTRACTION_DEF_H_

#include <memory>
#include <string>
#include <vector>

#include "param_factory.h"
#include "s_game.h"
#include "help_functions.h"

#include "pch.h"
#include "global_def.h"

class ActionAbstraction
{
public:
  ActionAbstraction(const Params &params) { action_abstraction_name_ = params.GetStringValue("BettingAbstractionName"); }
  virtual ~ActionAbstraction(void) {}
  const std::string &ActionAbstractionName(void) const
  {
    return action_abstraction_name_;
  }

private:
  std::string action_abstraction_name_;
};

class CardAbstraction
{
public:
  CardAbstraction(const Params &params)
  {
    card_abstraction_name_ = params.GetStringValue("CardAbstractionName");
    Split(params.GetStringValue("Bucketings").c_str(), ',', false,
          &bucketings_);
    int max_street = S_Game::MaxStreet();
    if ((int)bucketings_.size() < max_street + 1)
    {
      LOG(LT_ERROR_TRANS, DataModuleName.c_str(), "Expected at least %i bucketings", max_street + 1);
      exit(-1);
    }
  }
  ~CardAbstraction(void) {}
  const std::string &CardAbstractionName(void) const { return card_abstraction_name_; }
  const std::vector<std::string> &Bucketings(void) const { return bucketings_; }
  const std::string &Bucketing(int st) const { return bucketings_[st]; }

private:
  std::string card_abstraction_name_;
  std::vector<std::string> bucketings_;
};

class CFRAbstraction
{
public:
  CFRAbstraction(const Params &params) { cfr_abstraction_name_ = params.GetStringValue("CFRConfigName"); }
  ~CFRAbstraction(void) {}
  const std::string &CFRAbstractionName(void) const { return cfr_abstraction_name_; }

private:
  std::string cfr_abstraction_name_;
};

#endif