#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <string>
#include <vector>

#include "card_abstraction.h"
#include "global_variant.h"
#include "files.h"
#include "s_game.h"
#include "reader.h"
#include "param_factory.h"

#include "help_functions.h"

using std::string;
using std::unique_ptr;
using std::vector;

CardAbstraction::CardAbstraction(const Params &params)
{
  card_abstraction_name_ = params.GetStringValue("CardAbstractionName");
  Split(params.GetStringValue("Bucketings").c_str(), ',', false,
        &bucketings_);
  int max_street = S_Game::MaxStreet();
  if ((int)bucketings_.size() < max_street + 1)
  {
    fprintf(stderr, "Expected at least %i bucketings\n", max_street + 1);
    exit(-1);
  }
}
