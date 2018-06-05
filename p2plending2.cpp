/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include "p2plending2.hpp"
#include <time.h> 

using namespace eosio;
namespace p2p_lending {
struct impl {
   time_t timer;
   time(&timer);
   gmtime(&timer);
   unit64_t current_time= timer + 19800;//adding 5:30 hours to GMT Timing

   uint8_t  age_limit           = 21;
   unit32_t income_limit       = 300000;
   unit32_t lending_limit      = 500000;
   unit32_t borrowing_limit    = 500000;
   unit32_t per_lending_limit  = 20000;
   uint8_t  interest_limit     = 30;
   uint8_t  max_months         = 30;
   uint8_t  max_lratio         = 1;//max leverage ratio
   unit16_t cibil_limit        = 500;
   uint8_t  risk_limit         = 5;

   
 //some basic checks before any action can be included

  void on(const create_lender& c) {
      require_auth(c.lender_n);

      // Check if lender already exists
      lenders existing_lender(code_account, c.lender_n);
      auto itr = existing_lender.find( c.lender_n );
      eosio_assert(itr == existing_lender.end(), "lender already exists");

      existing_lender.emplace(adminl, [&]( auto& g ) {
         g.lender_n             = c.lender_n;
         g.pkey_l               = c.pkey_l;
         g.balance              = 0;
         g.lending_score        = 0;
         g.age                  = c.age;
         g.hash_aadhar          = c.hash_aadhar;
         g.hash_l               = c.hash_l;
         g.verified             = 0;
         g.epoch_ist            = 0;
      });
   }
   void on(const update_l& c) {
      require_auth(N(adminl));

      // Check if lender exists
      lenders existing_lender(code_account, c.pkey_l);
      auto itr = existing_lender.find( c.pkey_l );
      eosio_assert(itr != existing_lender.end(), "lender doesnot exists");

      existing_lender.modify(itr, admin_l, [&]( auto& g ) {
         g.balance              = c.balance;
         g.lending_score        = c.lending_score;
         g.age                  = c.age;
         g.hash_aadhar          = c.hash_aadhar;
         g.hash_l               = c.hash_l;
         g.verified             = c.verified;
         g.epoch_ist            = current_time;
      });
   }
  void on(const create_borrower& c) {
      require_auth(c.borrower_n);

      // Check if borrower already exists
      borrowers existing_borrower(code_account, c.borrower_n);
      auto itr = existing_borrower.find( c.borrower_n );
      eosio_assert(itr == existing_borrower.end(), "borrower already exists");

      existing_borrower.emplace(c.borrower_n, [&]( auto& g ) {
         g.borrower_n             = c.borrower_n;
         g.pkey_b                 = c.pkey_b;
         g.cibil_score            = 0;
         g.risk_category          = 5;
         g.age                    = c.age;
         g.income                 = c.income;
         g.leverage_ratio         = 0;
         g.hash_aadhar            = c.hash_aadhar;
         g.hash_b                 = c.hash_b;

      });
   }
  void on(const update_b& c) {
      require_auth(N(adminb));

      // Check if borrower  exists
      borrowers existing_borrower(code_account, c.pkey_b);
      auto itr = existing_borrower.find( c.pkey_b);
      eosio_assert(itr != existing_borrower.end(), "borrower doesnot exists");

      existing_borrower.modify(itr, adminb, [&]( auto& g ) {
         g.cibil_score            = g.cibil_score;
         g.risk_category          = g.risk_category;
         g.age                    = c.age;
         g.income                 = c.income;
         g.leverage_ratio         = c.leverage_ratio;
         g.hash_aadhar            = c.hash_aadhar;
         g.hash_b                 = c.hash_b;
         g.verified               = c.verified;
         g.epoch_ist              = current_time;

       });
   }

  void on(const create_lending& c) {
      

      // Check if lender exists
      lenders existing_lender(code_account, c.pkey_l);
      auto itr1 = existing_lender.find( c.pkey_l);
      eosio_assert(itr1 != existing_lender.end(), "lender doesnot exists");

      require_auth(itr->lender_n);

      //age check
      eosio_assert(itr1->age >age_limit, "lender is not eligible by age");

       // Check if borrower exists
      borrowers existing_borrower(code_account, c.pkey_b);
      auto itr2 = existing_borrower.find( c.pkey_b );
      eosio_assert(itr2 != existing_borrower.end(), "borrower doesnot exists");

      //age check
      eosio_assert(itr2->age > age_limit, "borrower is not eligible by age");

      //income limit check
      eosio_assert(itr2->income > income_limit, "borrower is not eligible by income");

      //cibil score limit
      eosio_assert(itr2->cibil_score > cibil_limit, "borrower is not eligible by cibil score");

      //risk category limit
      eosio_assert(itr2->risk_category < risk_limit, "borrower is not eligible by risk limit");

      //leverage ratio check
      eosio_assert(itr2->leverage_ratio < max_lratio, "borrower is not eligible by risk limit");
    
      //total lending check
      lendings existing_lender_lendings(code_account, c.pkey_l);
      auto itr3 = existing_lender_lendings.find( c.pkey_l);
      uint32_t net_lending = 0;
      while (itr3 != existing_lender_lendings.end() && itr3->pkey_l == c.pkey_l){
          
          net_lending = net_lending + itr3->principal;
          itr3++;
      }
      
      eosio_assert(net_lending + c.principal > lending_limit, "lender is crossing lending limit");
         
      //no borrowing before this borrowing on this platform
      lendings existing_b_lendings(code_account, c.pkey_b);
      auto itr4 = existing_b_lendings.find( c.pkey_b);
      auto itr5 = existing_b_lendings.find( c.loan_id);
      if(itr4 != existing_b_lendings.end()){
         eosio_assert(itr5!= existing_b_lendings.end(),"borrower has already borrowed money and is yet to pay back");
         eosio_assert(itr4== itr5,"borrower has already borrowed money and is yet to pay back");
         while (itr5!=existing_b_lendings.end() && itr5-> loan_id == c.loan_id){ itr5++;}
         eosio_assert(itr5 == existing_b_lendings.end(),"borrower has already borrowed money and is yet to pay back");
      }
  
         //borrowing limit check
      eosio_assert(c.net_borrowed < borrowing_limit, "borrower crossed borrowing limit");
      // lender exposore to borrower should not be more than 20% of net borrowed or more than per lending limit

      eosio_assert(c.principal < per_lending_limit && c.principal < net_borrowed/5, " lender has crossed per lending limits")

         //total time period check on current lending
      eosio_assert(c.maturity < max_months, "maturity limit has been crossed");
         //interest rate check
      eosio_assert(c.interest < interest_limit, "interest limit has been crossed");

        
      existing_lender_lendings.emplace(adminlen, [&]( auto& g ) {
         g.pkey_l               = c.pkey_l;
         g.pkey_b               = c.pkey_b;
         g.net_borrowed         = c.net_borrowed;
         g.principal            = c.principal;
         g.penalty              = 0;
         g.maturity             = c.maturity;
         g.months_left          = c.maturity;
         g.emi                  = c.emi;
         g.interest             = c.interest;
         g.unique_serial_number = c.unique_serial_number;
         g.loan_id              = c.loan_id;
         g.epoch_ist            = current_time;
      });
   }
    
    /**
    * @brief Apply close action
    * @param c - action to be applied
    */
  void on(const close_lending& c) {
      

      // Check if lending exist
      lendings existing_lendings(code_account, c.loan_id);
      auto itr = existing_lendings.find( c.loan_id );
      eosio_assert(itr != existing_lendings.end(), "lending does not exists");
      //penalty check
      eosio_assert(itr->penalty==0, "penalty is still due");
      //months left check
      eosio_assert(itr->months_left==0, "emi is still due")
      // Remove lending
      while (itr!= existing_lendings.end() && itr->loan_id== c.loan_id){
          existing_lendings.erase(itr);
          itr++;
      }
      
   }

   /**
    * @brief Apply update action
    * @param m - action to be applied
    */
    void on(const update_lending& c) {
      require_auth(N(adminlen));

      // Check if lending exists
      lendings existing_lendings(code_account, c.loan_id);
      auto itr = existing_lendings.find( c.loan_id );
      eosio_assert(itr != existing_lendings.end(), "lending doesn't exists");
      while(itr!=existing_lendings.end() &&  itr->loan_id== c.loan_id){
          // updatelending
          existing_lendings.modify(itr, adminlen, [&]( auto& g ) {
          g.months_left          = c.months_left;
          g.penalty              = c.penalty*g.principal/g.net_borrowed; //dividing penalty by the ratio of money lended
          g.epoch_ist            = current_time;
          });
          
          itr++;
      }
      
    
   }

   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t /*receiver*/, uint64_t code, uint64_t action ) {

      if (code == code_account) {
         if (action == N(createl)) {
            impl::on(eosio::unpack_action_data<p2p_lending::create_lender>());
         } else if (action == N(updatel)) {
            impl::on(eosio::unpack_action_data<p2p_lending::update_l>());
         } else if (action == N(createb)) {
            impl::on(eosio::unpack_action_data<p2p_lending::create_borrower>());
         } else if (action == N(updateb)) {
            impl::on(eosio::unpack_action_data<p2p_lending::update_b>());
         } else if (action == N(createlen)) {
            impl::on(eosio::unpack_action_data<p2p_lending::create_lending>());
         } else if (action == N(updatelen)) {
            impl::on(eosio::unpack_action_data<p2p_lending::update_lending>());
         } else if (action == N(closelen)) {
            impl::on(eosio::unpack_action_data<p2p_lending::close_lending>());
         } 
      }
   }

};
}

/**
*  The apply() methods must have C calling convention so that the blockchain can lookup and
*  call these methods.
*/
extern "C" {

   using namespace p2p_lending;
   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      impl().apply(receiver, code, action);
   }

} // extern "C"
