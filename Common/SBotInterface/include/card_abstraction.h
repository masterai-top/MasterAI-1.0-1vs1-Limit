#ifndef _CARD_ABSTRACTION_H_
#define _CARD_ABSTRACTION_H_

#include <memory>
#include <string>
#include <vector>

class Params;

class CardAbstraction {
 public:
  CardAbstraction(const Params &params);
  ~CardAbstraction(void) {}
  const std::string &CardAbstractionName(void) const {return card_abstraction_name_;}
  const std::vector<std::string> &Bucketings(void) const {return bucketings_;}
  const std::string &Bucketing(int st) const {return bucketings_[st];}
 private:
  std::string card_abstraction_name_;
  std::vector<std::string> bucketings_;
};

#endif
