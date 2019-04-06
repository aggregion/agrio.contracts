/**
 *  @file
 *  @copyright defined in agr/LICENSE.txt
 */
#pragma once

#include <agriolib/asset.hpp>
#include <agriolib/agrio.hpp>

#include <string>

namespace agriosystem {
   class system_contract;
}

namespace agrio {

   using std::string;

   class [[agrio::contract("agrio.token")]] token : public contract {
      public:
         using contract::contract;

         [[agrio::action]]
         void create( name   issuer,
                      asset  maximum_supply);

         [[agrio::action]]
         void issue( name to, asset quantity, string memo );

         [[agrio::action]]
         void retire( asset quantity, string memo );

         [[agrio::action]]
         void transfer( name    from,
                        name    to,
                        asset   quantity,
                        string  memo );

         [[agrio::action]]
         void open( name owner, const symbol& symbol, name ram_payer );

         [[agrio::action]]
         void close( name owner, const symbol& symbol );

         static asset get_supply( name token_contract_account, symbol_code sym_code )
         {
            stats statstable( token_contract_account, sym_code.raw() );
            const auto& st = statstable.get( sym_code.raw() );
            return st.supply;
         }

         static asset get_balance( name token_contract_account, name owner, symbol_code sym_code )
         {
            accounts accountstable( token_contract_account, owner.value );
            const auto& ac = accountstable.get( sym_code.raw() );
            return ac.balance;
         }

      private:
         struct [[agrio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[agrio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         typedef agrio::multi_index< "accounts"_n, account > accounts;
         typedef agrio::multi_index< "stat"_n, currency_stats > stats;

         void sub_balance( name owner, asset value );
         void add_balance( name owner, asset value, name ram_payer );
   };

} /// namespace agrio
