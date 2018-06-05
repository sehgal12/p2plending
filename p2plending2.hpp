
/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosiolib/eosio.hpp>
#include <ctime> 
#include<stdio.h>

using namespace eosio;
namespace p2p_lending {
      static const account_name lending_account  = N(lendings);
      static const account_name lender_account   = N(lenders);
      static const account_name borrower_account = N(borrowers);
      static const account_name admin_lender     = N(adminl);
      static const account_name admin_borrower   = N(adminb);
      static const account_name admin_lending    = N(adminlen);


      static const account_name code_account = N(p2plend);
      
      
      //@abi table
      struct lender_type {
            account_name     lender_n;//unique account name for lender
            uint32_t         pkey_l; //unique key for lender
            uint32_t         balance;//initializ to zero
            uint8_t          lending_score;//initilize to zero
            uint8_t          age;//should be more than 21
            std::string      hash_aadhar;
            std::string      hash_l;
            bool             verified;
            uint64_t         epoch_ist;//timestamp of last verification
 
            auto primary_key()      const { return pkey_l; }
            account_name get_name() const { return lender_n;}
            std::string get_hash()       const {return hash_aadhar;}
            std::string get_hash_l()     const {return hash_l;}
            uint64_t get_time()     const {return epoch_ist;}


            EOSLIB_SERIALIZE( lender_type, (lender_n)(pkey_l)(balance)(lending_score)(age)(hash_aadhar)(hash_l)(verified)(epoch_ist) )
      };
      //@abi action
      struct create_lender {
            account_name     lender_n;
            uint32_t         pkey_l;
            uint8_t          age;
            std::string      hash_aadhar;
            std::string      hash_l;
            

            EOSLIB_SERIALIZE( create_lender, (lender_n)(pkey_l)(age)(hash_aadhar)(hash_l) )
      };
       //@abi action
      struct update_l {
            uint32_t         pkey_l;
            uint32_t         balance;
            uint8_t          lending_score;
            uint8_t          age;
            std::string      hash_aadhar;
            std::string      hash_l;
            bool             verified;
            uint64_t         epoch_ist;

            EOSLIB_SERIALIZE( update_l, (pkey_l)(balance)(lending_score)(age)(hash_aadhar)(hash_l)(verified)(epoch_ist) )
      };
      //@abi action
      struct close_l {
            account_name     lender_n;
            uint32_t         pkey_l;

            EOSLIB_SERIALIZE( close_l, (lender_n)(pkey_l) )
      };
      //@abi table
      struct borrower_type {
            account_name     borrower_n;//unique account name for borrower
            uint32_t         pkey_b;
            uint16_t         cibil_score; //initialize with 0, can only be changed by some authorizing authority
            uint8_t          risk_category;//integer from 1 to 5 with 5 being the worst, can only be changed by authorizing authority
            uint8_t          age;//should be greater than or equal to 21
            uint32_t         income;//should be more than 3lakh
            uint16_t         leverage_ratio;//should be less than 2
            std::string      hash_aadhar;//hash of aadhar, will be used to prove the identity of user in case of information loss
            std::string      hash_b;// hash of all the static borrower information
            bool             verified;//initialize with 0, only admin_b can change it
            uint64_t         epoch_ist; //timestamp of last verification
            
            auto primary_key()      const { return pkey_b; }
            account_name get_name() const {return borrower_n;}
            std::string get_hash()       const {return hash_aadhar;}
            std::string get_hash_b()     const {return hash_b;}
            uint64_t get_time()     const {return epoch_ist;}

            EOSLIB_SERIALIZE( borrower_type, (borrower_n)(pkey_b)(cibil_score)(risk_category)(age)(income)(leverage_ratio)(hash_aadhar)(hash_b)(verified)(epoch_ist) )
      };
      //@abi action
      struct create_borrower {
            account_name     borrower_n;
            uint32_t         pkey_b;
            uint8_t          age;
            uint32_t         income;
            std::string      hash_aadhar;
            std::string      hash_b;

            EOSLIB_SERIALIZE( create_borrower, (borrower_n)(pkey_b)(age)(income)(hash_aadhar)(hash_b) )
      };
      //@abi action
      struct close_b {
            account_name     borrower_n;
            uint32_t         pkey_b;

            EOSLIB_SERIALIZE( close_b, (borrower_n)(pkey_b) )
      };
      //@abi action
      struct update_b {
            uint32_t         pkey_b;
            uint16_t         cibil_score; 
            uint8_t          risk_category;
            uint8_t          age;
            uint32_t         income;
            uint16_t         leverage_ratio;
            std::string      hash_aadhar;
            std::string      hash_b;
            bool             verified;
            uint64_t         epoch_ist;

            EOSLIB_SERIALIZE( update_b, (pkey_b)(cibil_score)(risk_category)(age)(income)(leverage_ratio)(hash_aadhar)(hash_b)(verified)(epoch_ist))
      };
      //@abi table
      struct lending {
            lending() {  }
            /**lending(account_name borrower_account, account_name lender_account)
                  : borrower(borrower_account), lender(lender_account) {
            }*/

            uint32_t          pkey_b;
            uint32_t          pkey_l;
            uint32_t          net_borrowed;
            uint32_t          principal;
            uint32_t          penalty;
            uint8_t           maturity;
            uint8_t           months_left;
            uint8_t           emi;
            double            interest;
            uint32_t          unique_serial_number;
            uint32_t          loan_id;
            uint64_t          epoch_ist;//time of start/last_update in lending

            auto primary_key()    const { return unique_serial_number; }
            uint32_t get_pkeyl()  const { return pkey_l;}
            uint32_t get_pkeyb()  const { return pkey_b;}
            uint32_t get_loanid() const { return loan_id;}
            uint64_t get_time()   const { return epoch_ist;}
            uint8_t  get_mleft()  const { return months_left;}

            EOSLIB_SERIALIZE( lending, (pkey_b)(pkey_l)(net_borrowed)(principal)(penalty)(maturity)(months_left)(emi)(interest)(unique_serial_number)(loan_id)(epoch_ist) )
      };

      //@abi action
      struct create_lending {
            uint32_t          pkey_b;
            uint32_t          pkey_l;
            uint32_t          net_borrowed;//total borrowed amount from all lenders
            uint32_t          principal;
            uint8_t           maturity;
            uint8_t           emi;
            uint8_t           interest;
            uint32_t          unique_serial_number;
            uint32_t          loan_id;
            uint64_t          epoch_ist;

            EOSLIB_SERIALIZE( create_lending, (pkey_b)(pkey_l)(net_borrowed)(principal)(maturity)(emi)(interest)(unique_serial_number)(loan_id)(epoch_ist) )
      };

      //@abi action
      struct close_lending {
            
           
            uint32_t          loan_id;//check if the net_borrowed amount is paid back fully for all lender with same load id

            EOSLIB_SERIALIZE( close_lending, (loan_id) )
      };
      //@abi action
      struct update_lending {
            
            uint8_t        months_left;
            uint32_t       penalty;
            uint32_t       loan_id;
            uint64_t       epoch_ist;
           

            EOSLIB_SERIALIZE( update_lending, (months_left)(penalty)(loan_id)(epoch_ist) )
      };

      

      /**
      * @brief table definition, used to store existing lendings and their current state
      */
      typedef eosio::multi_index< lending_account, lending > lendings;
      typedef eosio::multi_index< lender_account, lender_type > lenders;
      typedef eosio::multi_index< borrower_account, borrower_type > borrowers;
}