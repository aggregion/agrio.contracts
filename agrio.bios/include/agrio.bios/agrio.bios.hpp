#pragma once
#include <agriolib/action.hpp>
#include <agriolib/crypto.h>
#include <agriolib/agrio.hpp>
#include <agriolib/privileged.hpp>
#include <agriolib/producer_schedule.hpp>

namespace agrio {
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

   class [[agrio::contract("agrio.bios")]] bios : public contract {
      public:
         using contract::contract;
         [[agrio::action]]
         void newaccount( name             creator,
                          name             name,
                          ignore<authority> owner,
                          ignore<authority> active){}


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
         void setcode( name account, uint8_t vmtype, uint8_t vmversion, const std::vector<char>& code ) {}

         [[agrio::action]]
         void setpriv( name account, uint8_t is_priv ) {
            require_auth( _self );
            set_privileged( account.value, is_priv );
         }

         [[agrio::action]]
         void setalimits( name account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight ) {
            require_auth( _self );
            set_resource_limits( account.value, ram_bytes, net_weight, cpu_weight );
         }

         [[agrio::action]]
         void setglimits( uint64_t ram, uint64_t net, uint64_t cpu ) {
            (void)ram; (void)net; (void)cpu;
            require_auth( _self );
         }

         [[agrio::action]]
         void setprods( std::vector<agrio::producer_key> schedule ) {
            (void)schedule; // schedule argument just forces the deserialization of the action data into vector<producer_key> (necessary check)
            require_auth( _self );

            constexpr size_t max_stack_buffer_size = 512;
            size_t size = action_data_size();
            char* buffer = (char*)( max_stack_buffer_size < size ? malloc(size) : alloca(size) );
            read_action_data( buffer, size );
            set_proposed_producers(buffer, size);
         }

         [[agrio::action]]
         void setparams( const agrio::blockchain_parameters& params ) {
            require_auth( _self );
            set_blockchain_parameters( params );
         }

         [[agrio::action]]
         void reqauth( name from ) {
            require_auth( from );
         }

         [[agrio::action]]
         void setabi( name account, const std::vector<char>& abi ) {
            abi_hash_table table(_self, _self.value);
            auto itr = table.find( account.value );
            if( itr == table.end() ) {
               table.emplace( account, [&]( auto& row ) {
                  row.owner = account;
                  sha256( const_cast<char*>(abi.data()), abi.size(), &row.hash );
               });
            } else {
               table.modify( itr, same_payer, [&]( auto& row ) {
                  sha256( const_cast<char*>(abi.data()), abi.size(), &row.hash );
               });
            }
         }

         struct [[agrio::table]] abi_hash {
            name              owner;
            capi_checksum256  hash;
            uint64_t primary_key()const { return owner.value; }

            AGRLIB_SERIALIZE( abi_hash, (owner)(hash) )
         };

         typedef agrio::multi_index< "abihash"_n, abi_hash > abi_hash_table;
   };

} /// namespace agrio
