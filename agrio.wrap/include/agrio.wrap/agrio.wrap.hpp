#pragma once

#include <agriolib/agrio.hpp>
#include <agriolib/ignore.hpp>
#include <agriolib/transaction.hpp>

namespace agrio {

   class [[agrio::contract("agrio.wrap")]] wrap : public contract {
      public:
         using contract::contract;

         [[agrio::action]]
         void exec( ignore<name> executer, ignore<transaction> trx );

   };

} /// namespace agrio
