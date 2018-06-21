/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include "p2plending2.hpp"
#include<stdio.h>

using namespace eosio;
using namespace std;
namespace p2p_lending {


struct impl
{

   uint8_t  age_limit           = 21;
   uint32_t income_limit       = 300000;
   uint32_t lending_limit      = 500000;
   uint32_t borrowing_limit    = 500000;
   uint32_t per_lending_limit  = 20000;
   uint8_t  interest_limit     = 30;
   uint8_t  max_months         = 30;
   uint16_t cibil_limit        = 500;

   
 //some basic checks before any action can be included

  void on(const create_lender& c) {
     // require_auth(c.lender_n);
      //age check
      eosio_assert(c.age >= age_limit, "lender is not eligible by age");

      // Check if lender already exists
      lenders existing_lender(code_account, code_account);
      auto itr = existing_lender.find( c.lender_n );
      eosio_assert(itr == existing_lender.end(), "lender already exists");
      //check if any borrower has same account name as lender
      borrowers existing_borrower(code_account, code_account);
      auto itr1 =existing_borrower.find(c.lender_n);
      eosio_assert(itr1==existing_borrower.end(),"a borrower exists with the same account name");

      existing_lender.emplace(N(adminl), [&]( auto& g ) {
         g.lender_n             = c.lender_n;
         g.balance              = 0;
         g.total_lending          = 0;
         g.lending_score        = 0;
         g.age                  = c.age;
         g.verified             = 0;
      });
   }
   void on(const update_l& c) {
      require_auth(N(adminl));
       //age check
      eosio_assert(c.age >= age_limit, "lender is not eligible by age");

      // Check if lender exists
      lenders existing_lender(code_account, code_account);
      auto itr = existing_lender.find( c.lender_n );
      eosio_assert(itr != existing_lender.end(), "lender does not exists");

      existing_lender.modify(itr, N(adminl), [&]( auto& g ) {
         g.balance              = c.balance;
         g.lending_score        = c.lending_score;
         g.age                  = c.age;
         g.verified             = c.verified;
         
      });
   }
   void on(const close_l& c) {
      require_auth(N(adminl));

      // Check if lender exists
      lenders existing_lender(code_account, code_account);
      auto itr = existing_lender.find( c.lender_n );
      eosio_assert(itr != existing_lender.end(), "lender doesnot exists");
      eosio_assert(itr->balance==0,"lender has some balance, cannot close the account");
      eosio_assert(itr->total_lending==0, "lender has some lending, cannot close before lending are closed");
      
      existing_lender.erase(itr);
   }
  void on(const create_borrower& c) {

      //require_auth(c.borrower_n);
      //age check
      eosio_assert(c.age >= age_limit, "borrower is not eligible by age");

      //income limit check
      eosio_assert(c.income >= income_limit, "borrower is not eligible by income");

      // Check if borrower already exists
      borrowers existing_borrower(code_account, code_account);
      auto itr = existing_borrower.find( c.borrower_n );
      eosio_assert(itr == existing_borrower.end(), "borrower already exists");

      //check if any lender has account name  same as borrower to be created
      lenders existing_lender(code_account, code_account);
      auto itr1 = existing_lender.find( c.borrower_n );
      eosio_assert(itr1 == existing_lender.end(), "lender already exist with the same account name");

      existing_borrower.emplace(N(adminb), [&]( auto& g ) {
         g.borrower_n             = c.borrower_n;
         g.cibil_score            = 0;
         g.age                    = c.age;
         g.income                 = c.income;
         g.verified               = 0;


      });
   }
  void on(const update_b& c) {
      require_auth(N(adminb));

      //age check
      eosio_assert(c.age >= age_limit, "borrower is not eligible by age");

      //income limit check
      eosio_assert(c.income >= income_limit, "borrower is not eligible by income");

      // Check if borrower  exists
      borrowers existing_borrower(code_account, code_account);
      auto itr = existing_borrower.find( c.borrower_n);
      eosio_assert(itr != existing_borrower.end(), "borrower does not exists");

      existing_borrower.modify(itr, N(adminb), [&]( auto& g ) {
         g.cibil_score            = c.cibil_score;
         g.age                    = c.age;
         g.income                 = c.income;
         g.verified               = c.verified;

       });
   }
   void on(const close_b& c) {
      require_auth(N(adminb));

      // Check if borrower exists
      borrowers existing_borrower(code_account, code_account);
      auto itr = existing_borrower.find( c.borrower_n );
      eosio_assert(itr != existing_borrower.end(), "borrower doesnot exists");

      //check if borrower has any current lendings
      lendings existing_b_lendings(code_account, code_account);
      auto itr2 = existing_b_lendings.find(c.borrower_n);
      eosio_assert(itr2 == existing_b_lendings.end(), "borrower has some lendings, cannot close before lending is closed");
      
      existing_borrower.erase(itr);
   }

  void on(const create_lending& c) {
      
       require_auth(N(adminlen));
       
       // Check if borrower exists
      borrowers existing_borrower(code_account, code_account);
      auto itr2 = existing_borrower.find( c.borrower_n );
      eosio_assert(itr2 != existing_borrower.end(), "borrower doesnot exists");
      eosio_assert(itr2->verified, "borrower should be verified");
      
      //cibil score limit
      eosio_assert(itr2->cibil_score >= cibil_limit, "borrower is not eligible by cibil score");
     
          
      //no current borrowing before this borrowing on this platform
      lendings existing_b_lendings(code_account, code_account);
      auto itr4 = existing_b_lendings.find( c.borrower_n);
      eosio_assert(itr4 ==existing_b_lendings.end(),"borrower already has some current lending, cannot create another lending");
      

         //total time period check on current lending
      eosio_assert(c.maturity < max_months, "maturity limit has been crossed");

            //interest rate check
      eosio_assert(c.interest < interest_limit, "interest limit has been crossed");

       //borrowing limit check
      eosio_assert(c.net_borrowed < borrowing_limit, "borrower crossed borrowing limit");
        
      existing_b_lendings.emplace(N(adminlen), [&]( auto& g ) {
         g.borrower_n           = c.borrower_n;
         g.net_borrowed         = c.net_borrowed;
         g.amount_left          = c.net_borrowed;
         g.maturity             = c.maturity;
         g.emi                  = c.emi;
         g.interest             = c.interest;
      });

      
   }
     void on(const create_lending_info& c) {
      
       //require_auth(c.lender_n);
        // Check if lender exists
      lenders existing_lender(code_account, code_account);
      auto itr1 = existing_lender.find(c.lender_n);
      eosio_assert(itr1 != existing_lender.end(), "lender doesnot exists");
      eosio_assert(itr1->verified, "lender should be verified");
      eosio_assert(itr1->balance >= c.principal,"lender has insufficient funds");
      eosio_assert(itr1->total_lending + c.principal < lending_limit, "lender is crossing lending limits" );
   
      
      lendings existing_lendings(code_account,code_account);
      auto itr= existing_lendings.find(c.borrower_n);
      eosio_assert( itr!=existing_lendings.end(),"no lending with current borrower is created");
      eosio_assert( itr->amount_left >= c.principal, "borrower can't borrow more than he/she intends to originally");  // borrower is not borrowing more than he intends to 

      
      // lender exposore to borrower should not be more than 20% of net borrowed or more than per lending limit

      eosio_assert(c.principal < per_lending_limit && c.principal <= itr->net_borrowed/5, " lender is crossing per lending limits");

      lendinfo existing_lendings6(code_account, c.borrower_n);
      existing_lendings6.emplace(N(adminlen), [&]( auto& g1 ) {
         g1.borrower_n           = c.borrower_n;
         g1.lender_n             = c.lender_n;
         g1.principal            = c.principal;
         g1.penalty              = 0;
         g1.months_left          = itr->maturity;
      });

      existing_lendings.modify(itr, N(adminb), [&]( auto& g) {
        g.amount_left            = g.amount_left - c.principal;
      });

      existing_lender.modify(itr1, N(adminl), [&]( auto& g2 ) {
       
         g2.balance              = g2.balance - c.principal;
         g2.total_lending        = g2.total_lending + c.principal;
      });
   }
    
   

   /**
    * @brief Apply update action
    * @param m - action to be applied
    */
    void on(const update_lending& c) {
      require_auth(N(adminlen));

      lendings existing_lendings1(code_account, code_account);
      auto itr1 = existing_lendings1.find( c.borrower_n );
      eosio_assert(itr1 != existing_lendings1.end(), "lending does not exists");

      //close lending if the penalty and months left are zero in the input
      

      lendinfo existing_lendings(code_account, c.borrower_n);
      auto itr = existing_lendings.begin();
      eosio_assert(itr->months_left >= c.months_left, "months left should be less than previous value");

      while(itr!=existing_lendings.end() &&  itr->borrower_n== c.borrower_n){

          //updating balance in lender's profile
            lenders existing_lender(code_account,code_account);
            auto iter = existing_lender.find(itr->lender_n);
           
            existing_lender.modify(iter, N(adminl), [&]( auto&g1){
                g1.balance = g1.balance + itr1->emi*(itr->principal)/(itr1->net_borrowed)*(itr->months_left - c.months_left);
            });
           //close lending if the penalty and months left are zero in the input
          if(c.penalty==0 && c.months_left==0){

                    existing_lender.modify(iter, N(adminl), [&]( auto&g1){
                    g1.total_lending = g1.total_lending - itr->principal;
                     });
                    auto itr2 = itr;    //dereferencing object to be deleted
                    itr2++;
                    existing_lendings.erase(itr);
                    itr=itr2;
          }else{
                //updating lending
                    
                    existing_lendings.modify(itr, N(adminlen), [&]( auto& g ) {
                        g.months_left          = c.months_left;
                        g.penalty              = c.penalty*(itr->principal)/(itr1->net_borrowed); //dividing penalty by the ratio of money lender
                    });
                                        itr++;
                }
        
      }

      if(c.penalty==0 && c.months_left==0){
         existing_lendings1.erase(itr1);
      }
      
    
   }

   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t /*receiver*/, uint64_t code, uint64_t action ) {

      if (code == code_account) {
         if (action == N(createl)) {
            impl::on(eosio::unpack_action_data<p2p_lending::create_lender>());
         } else if (action == N(updatel)) {
            impl::on(eosio::unpack_action_data<p2p_lending::update_l>());
         } else if (action == N(closel)) {
            impl::on(eosio::unpack_action_data<p2p_lending::close_l>());
         } else if (action == N(createb)) {
            impl::on(eosio::unpack_action_data<p2p_lending::create_borrower>());
         } else if (action == N(updateb)) {
            impl::on(eosio::unpack_action_data<p2p_lending::update_b>());
         } else if (action == N(closeb)) {
            impl::on(eosio::unpack_action_data<p2p_lending::close_b>());
         } else if (action == N(createlen)) {
            impl::on(eosio::unpack_action_data<p2p_lending::create_lending>());
         } else if (action == N(createinfo)) {
            impl::on(eosio::unpack_action_data<p2p_lending::create_lending_info>());
         } else if (action == N(updatelen)) {
            impl::on(eosio::unpack_action_data<p2p_lending::update_lending>());
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



//EOSIO_ABI (impl, (create_lender)(update_l)(close_l)(create_borrower)(update_b)(close_b)(create_lending)(update_lending)(close_lending))
