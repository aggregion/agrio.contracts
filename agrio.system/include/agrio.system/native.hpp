/**
 *  @file
 *  @copyright defined in agr/LICENSE.txt
 */
#pragma once

#include <agriolib/action.hpp>
#include <agriolib/public_key.hpp>
#include <agriolib/print.hpp>
#include <agriolib/privileged.h>
#include <agriolib/producer_schedule.hpp>
#include <agriolib/contract.hpp>
#include <agriolib/ignore.hpp>

namespace agriosystem {
   using agrio::name;
   using agrio::permission_level;
   using agrio::public_key;
   using agrio::ignore;

   struct permission_level_weight {
      permission_level  permission;
      uint16_t          weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      AGRLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
   };

   struct key_weight {
      agrio::public_key  key;
      uint16_t           weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      AGRLIB_SERIALIZE( key_weight, (key)(weight) )
   };

   struct wait_weight {
      uint32_t           wait_sec;
      uint16_t           weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      AGRLIB_SERIALIZE( wait_weight, (wait_sec)(weight) )
   };

   struct authority {
      uint32_t                              threshold = 0;
      std::vector<key_weight>               keys;
      std::vector<permission_level_weight>  accounts;
      std::vector<wait_weight>              waits;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      AGRLIB_SERIALIZE( authority, (threshold)(keys)(accounts)(waits) )
   };

   struct block_header {
      uint32_t                                  timestamp;
      name                                      producer;
      uint16_t                                  confirmed = 0;
      capi_checksum256                          previous;
      capi_checksum256                          transaction_mroot;
      capi_checksum256                          action_mroot;
      uint32_t                                  schedule_version = 0;
      std::optional<agrio::producer_schedule>   new_producers;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      AGRLIB_SERIALIZE(block_header, (timestamp)(producer)(confirmed)(previous)(transaction_mroot)(action_mroot)
                                     (schedule_version)(new_producers))
   };


   struct [[agrio::table("abihash"), agrio::contract("agrio.system")]] abi_hash {
      name              owner;
      capi_checksum256  hash;
      uint64_t primary_key()const { return owner.value; }

      AGRLIB_SERIALIZE( abi_hash, (owner)(hash) )
   };

   /*
    * Method parameters commented out to prevent generation of code that parses input data.
    */
   class [[agrio::contract("agrio.system")]] native : public agrio::contract {
      public:

         using agrio::contract::contract;

         /**
          *  Called after a new account is created. This code enforces resource-limits rules
          *  for new accounts as well as new account naming conventions.
          *
          *  1. accounts cannot contain '.' symbols which forces all acccounts to be 12
          *  characters long without '.' until a future account auction process is implemented
          *  which prevents name squatting.
          *
          *  2. new accounts must stake a minimal number of tokens (as set in system parameters)
          *     therefore, this method will execute an inline buyram from receiver for newacnt in
          *     an amount equal to the current new account creation fee.
          */
         [[agrio::action]]
         void newaccount( name             creator,
                          name             name,
                          ignore<authority> owner,
                          ignore<authority> active);


         [[agrio::action]]
         void updateauth(  ignore<name>  account,
                           ignore<name>  permission,
                           ignore<name>  parent,
                           ignore<authority> auth ) {}

         [[agrio::action]]
         void deleteauth( ignore<name>  account,
                          ignore<name>  permission ) {}

         [[agrio::action]]
         void linkauth(  ignore<name>    account,
                         ignore<name>    code,
                         ignore<name>    type,
                         ignore<name>    requirement  ) {}

         [[agrio::action]]
         void unlinkauth( ignore<name>  account,
                          ignore<name>  code,
                          ignore<name>  type ) {}

         [[agrio::action]]
         void canceldelay( ignore<permission_level> canceling_auth, ignore<capi_checksum256> trx_id ) {}

         [[agrio::action]]
         void onerror( ignore<uint128_t> sender_id, ignore<std::vector<char>> sent_trx ) {}

         [[agrio::action]]
         void setabi( name account, const std::vector<char>& abi );

         [[agrio::action]]
         void setcode( name account, uint8_t vmtype, uint8_t vmversion, const std::vector<char>& code ) {}
   };
}
