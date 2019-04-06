#pragma once
#include <agriolib/agrio.hpp>
#include <agriolib/ignore.hpp>
#include <agriolib/transaction.hpp>

namespace agrio {

   class [[agrio::contract("agrio.msig")]] multisig : public contract {
      public:
         using contract::contract;

         [[agrio::action]]
         void propose(ignore<name> proposer, ignore<name> proposal_name,
               ignore<std::vector<permission_level>> requested, ignore<transaction> trx);
         [[agrio::action]]
         void approve( name proposer, name proposal_name, permission_level level,
                       const agrio::binary_extension<agrio::checksum256>& proposal_hash );
         [[agrio::action]]
         void unapprove( name proposer, name proposal_name, permission_level level );
         [[agrio::action]]
         void cancel( name proposer, name proposal_name, name canceler );
         [[agrio::action]]
         void exec( name proposer, name proposal_name, name executer );
         [[agrio::action]]
         void invalidate( name account );

      private:
         struct [[agrio::table]] proposal {
            name                            proposal_name;
            std::vector<char>               packed_transaction;

            uint64_t primary_key()const { return proposal_name.value; }
         };

         typedef agrio::multi_index< "proposal"_n, proposal > proposals;

         struct [[agrio::table]] old_approvals_info {
            name                            proposal_name;
            std::vector<permission_level>   requested_approvals;
            std::vector<permission_level>   provided_approvals;

            uint64_t primary_key()const { return proposal_name.value; }
         };
         typedef agrio::multi_index< "approvals"_n, old_approvals_info > old_approvals;

         struct approval {
            permission_level level;
            time_point       time;
         };

         struct [[agrio::table]] approvals_info {
            uint8_t                 version = 1;
            name                    proposal_name;
            //requested approval doesn't need to cointain time, but we want requested approval
            //to be of exact the same size ad provided approval, in this case approve/unapprove
            //doesn't change serialized data size. So, we use the same type.
            std::vector<approval>   requested_approvals;
            std::vector<approval>   provided_approvals;

            uint64_t primary_key()const { return proposal_name.value; }
         };
         typedef agrio::multi_index< "approvals2"_n, approvals_info > approvals;

         struct [[agrio::table]] invalidation {
            name         account;
            time_point   last_invalidation_time;

            uint64_t primary_key() const { return account.value; }
         };

         typedef agrio::multi_index< "invals"_n, invalidation > invalidations;
   };

} /// namespace agrio
