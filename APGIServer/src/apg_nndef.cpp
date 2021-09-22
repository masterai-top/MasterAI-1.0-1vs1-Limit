#include "apg_nndef.h"

namespace nndef
{
    namespace nncard
    {
        card_t getNNType(card_t card)
        {
            return card & 0x00f0;
        }

        card_t getNNValue(card_t card)
        {
            return (card & 0x000f) > 10 ? 10 : (card & 0x000f);
        }

        card_t getNNNum(card_t card)
        {
            return card & 0x000f;
        }
    };

    namespace nninvalid
    {
        using namespace nnuser;
        using namespace nncard;

        guid_t nil_uid  = static_cast<guid_t>(-1);
        cid_t nil_cid   = static_cast<cid_t>(-1);
        card_t nil_card = static_cast<card_t>(-1);

        E_NN_ACT        nil_act = static_cast<E_NN_ACT>(-1);
        E_NN_TYPE       nil_nntype = static_cast<E_NN_TYPE>(-1);
        E_NN_GUESS      nil_guess = static_cast<E_NN_GUESS>(-1);
        E_NN_STATE      nil_nnstate = static_cast<E_NN_STATE>(-1);
    };
};

