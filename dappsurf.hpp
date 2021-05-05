#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/crypto.hpp>
#include <eosio/time.hpp>


#define CURRENT_PLATFORM_VERSION 1

#define DEFAULT_WS_METADATA_TIER 1
#define DEFAULT_WS_METADATA_MODE 1
#define DEFAULT_WS_METADATA_SETTINGS 1
#define DEFAULT_WS_METADATA_LANGUAGE 1


#define DEBUG_CONTRACT_ADMIN "dappsurfweb1"_n


using namespace eosio;

CONTRACT dappsurf : public contract {
  public:
    using contract::contract;
    dappsurf(name receiver, name code, datastream<const char *> ds)
        : contract(receiver, code, ds),
          tbl_websites(receiver, receiver.value),
          tbl_wsmetadata(receiver, receiver.value) {}



    ACTION editwebsite(name user, name domain, std::string content, uint32_t settings, uint32_t language);
    ACTION editmetadata(name user, name domain, uint32_t settings, uint32_t language);

    // FOR DEVELOPMENT/TESTING NETWORKS ONLY: delete this action if shipping to the mainnet!
    ACTION devclearall();


    TABLE s_tbl_websites {
      name domain;
      std::string content;
      uint64_t primary_key() const { return domain.value; }
    };

    TABLE s_tbl_wsmetadata {
      name domain;
      name owner;

      checksum256 sha256hash;

      uint32_t platform_version;
      
      uint32_t tier;
      uint32_t mode;
      uint32_t settings;
      uint32_t language;
      
      uint64_t content_updated_at;
      uint64_t created_at;

      uint64_t primary_key() const { return domain.value; }
      uint64_t by_owner() const { return owner.value; }
      uint64_t by_created_at() const { return created_at; }
      uint64_t by_content_updated_at() const { return content_updated_at; }
      uint64_t by_tier() const { return tier; }
    };



    typedef eosio::multi_index<"websites"_n, s_tbl_websites> t_tbl_websites;
    typedef eosio::multi_index<"wsmetadata"_n, s_tbl_wsmetadata, 
      eosio::indexed_by<"byowner"_n, eosio::const_mem_fun<s_tbl_wsmetadata, uint64_t, &s_tbl_wsmetadata::by_owner> >,
      eosio::indexed_by<"bycreatedat"_n, eosio::const_mem_fun<s_tbl_wsmetadata, uint64_t, &s_tbl_wsmetadata::by_created_at> >,
      eosio::indexed_by<"bycontentupd"_n, eosio::const_mem_fun<s_tbl_wsmetadata, uint64_t, &s_tbl_wsmetadata::by_content_updated_at> >,
      eosio::indexed_by<"bytier"_n, eosio::const_mem_fun<s_tbl_wsmetadata, uint64_t, &s_tbl_wsmetadata::by_tier> >
    > t_tbl_wsmetadata;


    using editwebsite_action = action_wrapper<"editwebsite"_n, &dappsurf::editwebsite>;
    using editmetadata_action = action_wrapper<"editmetadata"_n, &dappsurf::editmetadata>;

    using devclearall_action = action_wrapper<"devclearall"_n, &dappsurf::devclearall>;

    t_tbl_websites tbl_websites;
    t_tbl_wsmetadata tbl_wsmetadata;

  private:
    bool claim_domain(name user, name domain);
    auto assert_can_upsert_website(name user, name domain);
    void upsert_website_content(name user, name domain, std::string content, uint32_t tier, uint32_t mode, uint32_t settings, uint32_t language);
    void update_website_metadata(name user, name domain, uint32_t tier, uint32_t mode, uint32_t settings, uint32_t language);


};
