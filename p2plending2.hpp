
/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/eosio.hpp>
#include <eosiolib/multi_index.hpp>
#include<stdio.h>
#include<string>

using std::string;
using namespace eosio;
namespace p2p_lending {
      static const account_name lending_account               = N(lendings);
      static const account_name lending_info_account          = N(lendinfo);
      static const account_name lender_account                = N(lenders);
      static const account_name borrower_account              = N(borrowers);
      static const account_name admin_lender                  = N(adminl);
      static const account_name admin_borrower                = N(adminb);
      static const account_name admin_lending                 = N(adminlen);


      static const account_name code_account = N(p2pl);
      
      
      //@abi table
      struct lender_type {
            account_name     lender_n;//unique account name for lender
            uint64_t         epoch_ist;//timestamp of last verification
            uint32_t         balance;//initializ to zero
            uint32_t         total_lending;//total lending so far,initialize to 0
            uint8_t          lending_score;//initilize to zero
            uint8_t          age;//should be more than 21
            bool             verified;
            
 
            auto primary_key()    const { return lender_n; }

            EOSLIB_SERIALIZE( lender_type, (lender_n)(epoch_ist)(balance)(total_lending)(lending_score)(age)(verified) )
      };
      //@abi action
      struct create_lender {
            account_name     lender_n;
            uint8_t          age;
      

            EOSLIB_SERIALIZE( create_lender, (lender_n)(age))
      };
       //@abi action
      struct update_l {
            account_name     lender_n;
            uint64_t         epoch_ist;
            uint32_t         balance;
            uint8_t          lending_score;
            uint8_t          age;
            bool             verified;
           

            EOSLIB_SERIALIZE( update_l, (lender_n)(epoch_ist)(balance)(lending_score)(age)(verified) )
      };
      //@abi action
      struct close_l {
            account_name     lender_n;

            EOSLIB_SERIALIZE( close_l, (lender_n) )
      };
      //@abi table
      struct borrower_type {
            account_name     borrower_n;//unique account name for borrower
            uint64_t         epoch_ist; //timestamp of last verification
            uint16_t         cibil_score; //initialize with 0, can only be changed by some authorizing authority
            uint8_t          age;//should be greater than or equal to 21
            uint32_t         income;//should be more than 3lakh
            bool             verified;//initialize with 0, only admin_b can change it
        
            
            auto primary_key()      const { return borrower_n; }


            EOSLIB_SERIALIZE( borrower_type, (borrower_n)(epoch_ist)(cibil_score)(age)(income)(verified) )
      };
      //@abi action
      struct create_borrower {
            account_name     borrower_n;
            uint8_t          age;
            uint32_t         income;

            EOSLIB_SERIALIZE( create_borrower, (borrower_n)(age)(income) )
      };
      //@abi action
      struct close_b {
            account_name     borrower_n;

            EOSLIB_SERIALIZE( close_b, (borrower_n) )
      };
      //@abi action
      struct update_b {
            account_name     borrower_n;
            uint64_t         epoch_ist;
            uint16_t         cibil_score;
            uint8_t          age;
            uint32_t         income;
            bool             verified;
        

            EOSLIB_SERIALIZE( update_b, (borrower_n)(epoch_ist)(cibil_score)(age)(income)(verified))
      };
      //@abi table
      struct lending {
            account_name      borrower_n;
            uint32_t          net_borrowed;
            uint32_t          amount_left;
            uint8_t           maturity;
            uint8_t           interest;
            uint32_t          emi;
            uint64_t          epoch_ist;//time of start of the lending

            auto primary_key()          const { return borrower_n; }
          

            EOSLIB_SERIALIZE( lending, (borrower_n)(net_borrowed)(amount_left)(maturity)(interest)(emi)(epoch_ist) )
      };
      struct lending_info {

            account_name      lender_n; 
            account_name      borrower_n;
            uint32_t          penalty;
            uint32_t          principal;
            uint8_t           months_left;
            uint64_t          epoch_ist;//time of start/last_update in lending

            auto primary_key()          const { return lender_n; }
            account_name get_borrower()       const {return borrower_n;}

            EOSLIB_SERIALIZE( lending_info, (lender_n)(borrower_n)(penalty)(principal)(months_left)(epoch_ist) )
      };

      //@abi action
      struct create_lending {
            account_name      borrower_n;
            uint32_t          net_borrowed;
            uint8_t           maturity;
            uint8_t           interest;
            uint32_t          emi;
            uint64_t          epoch_ist;//time of start of the lending
           

            EOSLIB_SERIALIZE( create_lending, (borrower_n)(net_borrowed)(maturity)(interest)(emi)(epoch_ist) )
      };
        struct create_lending_info {
            account_name      borrower_n;
            account_name      lender_n; 
            uint32_t          principal;
            uint64_t          epoch_ist;//time of start/last_update in lending

            EOSLIB_SERIALIZE( create_lending_info, (borrower_n)(lender_n)(principal)(epoch_ist) )
      };

      //@abi action
      struct update_lending {
            account_name   borrower_n;
            uint8_t        months_left;
            uint32_t       penalty;
            uint64_t       epoch_ist;
           

            EOSLIB_SERIALIZE( update_lending, (borrower_n)(months_left)(penalty)(epoch_ist) )
      };

      /**
      * @brief table definition, used to store existing lendings and their current state
      */
      typedef eosio::multi_index< lending_account, lending> lendings;
      typedef eosio::multi_index< lending_info_account, lending_info,eosio::indexed_by< N(byborrower), eosio::const_mem_fun<lending_info, account_name, &lending_info::get_borrower> > > lendinfo ;
      typedef eosio::multi_index< lender_account, lender_type >  lenders;
      typedef eosio::multi_index< borrower_account, borrower_type > borrowers;
}