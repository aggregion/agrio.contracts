/**
 *  @file
 *  @copyright defined in agr/LICENSE.txt
 */
#pragma once

#include <agrio.system/native.hpp>
#include <agriolib/asset.hpp>
#include <agriolib/time.hpp>
#include <agriolib/privileged.hpp>
#include <agriolib/singleton.hpp>
#include <agrio.system/exchange_state.hpp>

#include <string>
#include <type_traits>
#include <optional>

namespace agriosystem {

   using agrio::name;
   using agrio::asset;
   using agrio::symbol;
   using agrio::symbol_code;
   using agrio::indexed_by;
   using agrio::const_mem_fun;
   using agrio::block_timestamp;
   using agrio::time_point;
   using agrio::microseconds;
   using agrio::datastream;

   template<typename E, typename F>
   static inline auto has_field( F flags, E field )
   -> std::enable_if_t< std::is_integral_v<F> && std::is_unsigned_v<F> &&
                        std::is_enum_v<E> && std::is_same_v< F, std::underlying_type_t<E> >, bool>
   {
      return ( (flags & static_cast<F>(field)) != 0 );
   }

   template<typename E, typename F>
   static inline auto set_field( F flags, E field, bool value = true )
   -> std::enable_if_t< std::is_integral_v<F> && std::is_unsigned_v<F> &&
                        std::is_enum_v<E> && std::is_same_v< F, std::underlying_type_t<E> >, F >
   {
      if( value )
         return ( flags | static_cast<F>(field) );
      else
         return ( flags & ~static_cast<F>(field) );
   }

   struct [[agrio::table, agrio::contract("agrio.system")]] name_bid {
     name            newname;
     name            high_bidder;
     int64_t         high_bid = 0; ///< negative high_bid == closed auction waiting to be claimed
     time_point      last_bid_time;

     uint64_t primary_key()const { return newname.value;                    }
     uint64_t by_high_bid()const { return static_cast<uint64_t>(-high_bid); }
   };

   struct [[agrio::table, agrio::contract("agrio.system")]] bid_refund {
      name         bidder;
      asset        amount;

      uint64_t primary_key()const { return bidder.value; }
   };

   typedef agrio::multi_index< "namebids"_n, name_bid,
                               indexed_by<"highbid"_n, const_mem_fun<name_bid, uint64_t, &name_bid::by_high_bid>  >
                             > name_bid_table;

   typedef agrio::multi_index< "bidrefunds"_n, bid_refund > bid_refund_table;

   struct [[agrio::table("global"), agrio::contract("agrio.system")]] agrio_global_state : agrio::blockchain_parameters {
      uint64_t free_ram()const { return max_ram_size - total_ram_bytes_reserved; }

      uint64_t             max_ram_size = 64ll*1024 * 1024 * 1024;
      uint64_t             total_ram_bytes_reserved = 0;
      int64_t              total_ram_stake = 0;

      block_timestamp      last_producer_schedule_update;
      time_point           last_pervote_bucket_fill;
      int64_t              pervote_bucket = 0;
      int64_t              perblock_bucket = 0;
      uint32_t             total_unpaid_blocks = 0; /// all blocks which have been produced but not paid
      int64_t              total_activated_stake = 0;
      time_point           thresh_activated_stake_time;
      uint16_t             last_producer_schedule_size = 0;
      double               total_producer_vote_weight = 0; /// the sum of all producer votes
      block_timestamp      last_name_close;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      AGRLIB_SERIALIZE_DERIVED( agrio_global_state, agrio::blockchain_parameters,
                                (max_ram_size)(total_ram_bytes_reserved)(total_ram_stake)
                                (last_producer_schedule_update)(last_pervote_bucket_fill)
                                (pervote_bucket)(perblock_bucket)(total_unpaid_blocks)(total_activated_stake)(thresh_activated_stake_time)
                                (last_producer_schedule_size)(total_producer_vote_weight)(last_name_close) )
   };

   /**
    * Defines new global state parameters added after version 1.0
    */
   struct [[agrio::table("global2"), agrio::contract("agrio.system")]] agrio_global_state2 {
      agrio_global_state2(){}

      uint16_t          new_ram_per_block = 0;
      block_timestamp   last_ram_increase;
      block_timestamp   last_block_num; /* deprecated */
      double            total_producer_votepay_share = 0;
      uint8_t           revision = 0; ///< used to track version updates in the future.

      AGRLIB_SERIALIZE( agrio_global_state2, (new_ram_per_block)(last_ram_increase)(last_block_num)
                        (total_producer_votepay_share)(revision) )
   };

   struct [[agrio::table("global3"), agrio::contract("agrio.system")]] agrio_global_state3 {
      agrio_global_state3() { }
      time_point        last_vpay_state_update;
      double            total_vpay_share_change_rate = 0;

      AGRLIB_SERIALIZE( agrio_global_state3, (last_vpay_state_update)(total_vpay_share_change_rate) )
   };

   struct [[agrio::table, agrio::contract("agrio.system")]] producer_info {
      name                  owner;
      double                total_votes = 0;
      agrio::public_key     producer_key; /// a packed public key object
      bool                  is_active = true;
      std::string           url;
      uint32_t              unpaid_blocks = 0;
      time_point            last_claim_time;
      uint16_t              location = 0;

      uint64_t primary_key()const { return owner.value;                             }
      double   by_votes()const    { return is_active ? -total_votes : total_votes;  }
      bool     active()const      { return is_active;                               }
      void     deactivate()       { producer_key = public_key(); is_active = false; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      AGRLIB_SERIALIZE( producer_info, (owner)(total_votes)(producer_key)(is_active)(url)
                        (unpaid_blocks)(last_claim_time)(location) )
   };

   struct [[agrio::table, agrio::contract("agrio.system")]] producer_info2 {
      name            owner;
      double          votepay_share = 0;
      time_point      last_votepay_share_update;

      uint64_t primary_key()const { return owner.value; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      AGRLIB_SERIALIZE( producer_info2, (owner)(votepay_share)(last_votepay_share_update) )
   };

   struct [[agrio::table, agrio::contract("agrio.system")]] voter_info {
      name                owner;     /// the voter
      name                proxy;     /// the proxy set by the voter, if any
      std::vector<name>   producers; /// the producers approved by this voter if no proxy set
      int64_t             staked = 0;

      /**
       *  Every time a vote is cast we must first "undo" the last vote weight, before casting the
       *  new vote weight.  Vote weight is calculated as:
       *
       *  stated.amount * 2 ^ ( weeks_since_launch/weeks_per_year)
       */
      double              last_vote_weight = 0; /// the vote weight cast the last time the vote was updated

      /**
       * Total vote weight delegated to this voter.
       */
      double              proxied_vote_weight= 0; /// the total vote weight delegated to this voter as a proxy
      bool                is_proxy = 0; /// whether the voter is a proxy for others


      uint32_t            flags1 = 0;
      uint32_t            reserved2 = 0;
      agrio::asset        reserved3;

      uint64_t primary_key()const { return owner.value; }

      enum class flags1_fields : uint32_t {
         ram_managed = 1,
         net_managed = 2,
         cpu_managed = 4
      };

      // explicit serialization macro is not necessary, used here only to improve compilation time
      AGRLIB_SERIALIZE( voter_info, (owner)(proxy)(producers)(staked)(last_vote_weight)(proxied_vote_weight)(is_proxy)(flags1)(reserved2)(reserved3) )
   };

   typedef agrio::multi_index< "voters"_n, voter_info >  voters_table;


   typedef agrio::multi_index< "producers"_n, producer_info,
                               indexed_by<"prototalvote"_n, const_mem_fun<producer_info, double, &producer_info::by_votes>  >
                             > producers_table;
   typedef agrio::multi_index< "producers2"_n, producer_info2 > producers_table2;

   typedef agrio::singleton< "global"_n, agrio_global_state >   global_state_singleton;
   typedef agrio::singleton< "global2"_n, agrio_global_state2 > global_state2_singleton;
   typedef agrio::singleton< "global3"_n, agrio_global_state3 > global_state3_singleton;

   //   static constexpr uint32_t     max_inflation_rate = 5;  // 5% annual inflation
   static constexpr uint32_t     seconds_per_day = 24 * 3600;

   class [[agrio::contract("agrio.system")]] system_contract : public native {
      private:
         voters_table            _voters;
         producers_table         _producers;
         producers_table2        _producers2;
         global_state_singleton  _global;
         global_state2_singleton _global2;
         global_state3_singleton _global3;
         agrio_global_state      _gstate;
         agrio_global_state2     _gstate2;
         agrio_global_state3     _gstate3;
         rammarket               _rammarket;

      public:
         static constexpr agrio::name active_permission{"active"_n};
         static constexpr agrio::name token_account{"agrio.token"_n};
         static constexpr agrio::name ram_account{"agrio.ram"_n};
         static constexpr agrio::name ramfee_account{"agrio.ramfee"_n};
         static constexpr agrio::name stake_account{"agrio.stake"_n};
         static constexpr agrio::name bpay_account{"agrio.bpay"_n};
         static constexpr agrio::name vpay_account{"agrio.vpay"_n};
         static constexpr agrio::name names_account{"agrio.names"_n};
         static constexpr agrio::name saving_account{"agrio.saving"_n};
         static constexpr symbol ramcore_symbol = symbol(symbol_code("RAMCORE"), 4);
         static constexpr symbol ram_symbol     = symbol(symbol_code("RAM"), 0);

         system_contract( name s, name code, datastream<const char*> ds );
         ~system_contract();

         static symbol get_core_symbol( name system_account = "agrio"_n ) {
            rammarket rm(system_account, system_account.value);
            const static auto sym = get_core_symbol( rm );
            return sym;
         }

         // Actions:
         [[agrio::action]]
         void init( unsigned_int version, symbol core );
         [[agrio::action]]
         void onblock( ignore<block_header> header );

         [[agrio::action]]
         void setalimits( name account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight );

         [[agrio::action]]
         void setacctram( name account, std::optional<int64_t> ram_bytes );

         [[agrio::action]]
         void setacctnet( name account, std::optional<int64_t> net_weight );

         [[agrio::action]]
         void setacctcpu( name account, std::optional<int64_t> cpu_weight );

         // functions defined in delegate_bandwidth.cpp

         /**
          *  Stakes SYS from the balance of 'from' for the benfit of 'receiver'.
          *  If transfer == true, then 'receiver' can unstake to their account
          *  Else 'from' can unstake at any time.
          */
         [[agrio::action]]
         void delegatebw( name from, name receiver,
                          asset stake_net_quantity, asset stake_cpu_quantity, bool transfer );


         /**
          *  Decreases the total tokens delegated by from to receiver and/or
          *  frees the memory associated with the delegation if there is nothing
          *  left to delegate.
          *
          *  This will cause an immediate reduction in net/cpu bandwidth of the
          *  receiver.
          *
          *  A transaction is scheduled to send the tokens back to 'from' after
          *  the staking period has passed. If existing transaction is scheduled, it
          *  will be canceled and a new transaction issued that has the combined
          *  undelegated amount.
          *
          *  The 'from' account loses voting power as a result of this call and
          *  all producer tallies are updated.
          */
         [[agrio::action]]
         void undelegatebw( name from, name receiver,
                            asset unstake_net_quantity, asset unstake_cpu_quantity );


         /**
          * Increases receiver's ram quota based upon current price and quantity of
          * tokens provided. An inline transfer from receiver to system contract of
          * tokens will be executed.
          */
         [[agrio::action]]
         void buyram( name payer, name receiver, asset quant );
         [[agrio::action]]
         void buyrambytes( name payer, name receiver, uint32_t bytes );

         /**
          *  Reduces quota my bytes and then performs an inline transfer of tokens
          *  to receiver based upon the average purchase price of the original quota.
          */
         [[agrio::action]]
         void sellram( name account, int64_t bytes );

         /**
          *  This action is called after the delegation-period to claim all pending
          *  unstaked tokens belonging to owner
          */
         [[agrio::action]]
         void refund( name owner );

         // functions defined in voting.cpp

         [[agrio::action]]
         void regproducer( const name producer, const public_key& producer_key, const std::string& url, uint16_t location );

         [[agrio::action]]
         void unregprod( const name producer );

         [[agrio::action]]
         void setram( uint64_t max_ram_size );
         [[agrio::action]]
         void setramrate( uint16_t bytes_per_block );

         [[agrio::action]]
         void voteproducer( const name voter, const name proxy, const std::vector<name>& producers );

         [[agrio::action]]
         void regproxy( const name proxy, bool isproxy );

         [[agrio::action]]
         void setparams( const agrio::blockchain_parameters& params );

         // functions defined in producer_pay.cpp
         [[agrio::action]]
         void claimrewards( const name owner );

         [[agrio::action]]
         void setpriv( name account, uint8_t is_priv );

         [[agrio::action]]
         void rmvproducer( name producer );

         [[agrio::action]]
         void updtrevision( uint8_t revision );

         [[agrio::action]]
         void bidname( name bidder, name newname, asset bid );

         [[agrio::action]]
         void bidrefund( name bidder, name newname );

      private:
         // Implementation details:

         static symbol get_core_symbol( const rammarket& rm ) {
            auto itr = rm.find(ramcore_symbol.raw());
            agrio_assert(itr != rm.end(), "system contract must first be initialized");
            return itr->quote.balance.symbol;
         }

         //defined in agrio.system.cpp
         static agrio_global_state get_default_parameters();
         static time_point current_time_point();
         static block_timestamp current_block_time();

         symbol core_symbol()const;

         void update_ram_supply();

         //defined in delegate_bandwidth.cpp
         void changebw( name from, name receiver,
                        asset stake_net_quantity, asset stake_cpu_quantity, bool transfer );

         //defined in voting.hpp
         void update_elected_producers( block_timestamp timestamp );
         void update_votes( const name voter, const name proxy, const std::vector<name>& producers, bool voting );

         // defined in voting.cpp
         void propagate_weight_change( const voter_info& voter );

         double update_producer_votepay_share( const producers_table2::const_iterator& prod_itr,
                                               time_point ct,
                                               double shares_rate, bool reset_to_zero = false );
         double update_total_votepay_share( time_point ct,
                                            double additional_shares_delta = 0.0, double shares_rate_delta = 0.0 );
   };

} /// agriosystem
