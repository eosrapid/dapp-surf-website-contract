#include <dappsurf.hpp>


bool dappsurf::claim_domain(name user, name domain) {
  if(domain.length() == 12){
    eosio::check(user.value == domain.value,
      "new free domains must be the same as the user's eos name");
    return true;
  }else{
    // TODO: add claiming logic for premium domains
    eosio::check(domain.length() == 12, "only 12 character domains are currently supported!");
  }
  return false;
}
auto dappsurf::assert_can_upsert_website(name user, name domain) {
  auto wsmetadata_iterator = tbl_wsmetadata.find(domain.value);
  if(wsmetadata_iterator == tbl_wsmetadata.end()){
    eosio::check(claim_domain(user, domain) == true,
      "user does not have the right to claim this domain");
  }else{
    name owner = wsmetadata_iterator->owner;
    eosio::check(owner.value == user.value,
      "user does not own this domain");
  }
  return wsmetadata_iterator;
}
void dappsurf::upsert_website_content(name user, name domain, std::string content, uint32_t tier, uint32_t mode, uint32_t settings, uint32_t language) {
  uint64_t current_time = eosio::current_time_point().sec_since_epoch();
  checksum256 sha256hash = sha256(content.c_str(), content.length());

  auto wsmetadata_iterator = assert_can_upsert_website(user, domain);
  auto websites_iterator = tbl_websites.find(domain.value);

  if(wsmetadata_iterator == tbl_wsmetadata.end()){
    eosio::check(websites_iterator == tbl_websites.end(),
      "state corrupted: website record exists for domain but metadata does not");
    
    tbl_wsmetadata.emplace(user, [&](auto &row) {
      row.domain = domain;
      row.owner = user;
      
      row.sha256hash = sha256hash;

      row.platform_version = CURRENT_PLATFORM_VERSION;
      
      row.tier = tier!=0 ? tier : DEFAULT_WS_METADATA_TIER;
      row.mode = mode!=0 ? mode : DEFAULT_WS_METADATA_MODE;
      row.settings = settings!=0 ? settings : DEFAULT_WS_METADATA_SETTINGS;
      row.language = language!=0 ? language : DEFAULT_WS_METADATA_LANGUAGE;

      row.content_updated_at = current_time;
      row.created_at = current_time;
    });

    tbl_websites.emplace(user, [&](auto &row) {
      row.domain = domain;
      row.content = content;
    });
  }else{
    eosio::check(websites_iterator != tbl_websites.end(),
      "state corrupted: metadata record exists for domain but website does not");
    
    tbl_wsmetadata.modify(wsmetadata_iterator, user, [&](auto &row) {
      row.sha256hash = sha256hash;
      row.content_updated_at = current_time;

      if(tier != 0){
        row.tier = tier;
      }
      if(mode != 0){
        row.mode = mode;
      }
      if(settings != 0){
        row.settings = settings;
      }
      if(language != 0){
        row.language = language;
      }
    });

    tbl_websites.modify(websites_iterator, user, [&](auto &row) {
      row.content = content;
    });
  }
}

void dappsurf::update_website_metadata(name user, name domain, uint32_t tier, uint32_t mode, uint32_t settings, uint32_t language) {
  eosio::check(
    tier != 0 ||
    mode != 0 ||
    settings != 0 ||
    language != 0,
    "tried to update metadata, but no updates need to be made!"
  );

  auto wsmetadata_iterator = tbl_wsmetadata.find(domain.value);
  
  eosio::check(wsmetadata_iterator != tbl_wsmetadata.end(),
    "tried to update metadata for domain that does not exist");

  name owner = wsmetadata_iterator->owner;

  eosio::check(owner.value == user.value,
    "user is not the owner of this domain");
  
  tbl_wsmetadata.modify(wsmetadata_iterator, user, [&](auto &row) {
    if(tier != 0){
      row.tier = tier;
    }
    if(mode != 0){
      row.mode = mode;
    }
    if(settings != 0){
      row.settings = settings;
    }
    if(language != 0){
      row.language = language;
    }
  });
}


ACTION dappsurf::editwebsite(name user, name domain, std::string content, uint32_t settings, uint32_t language) {
  require_auth(user);
  upsert_website_content(user, domain, content, 0, 0, settings, language);
}

ACTION dappsurf::editmetadata(name user, name domain, uint32_t settings, uint32_t language) {
  require_auth(user);
  update_website_metadata(user, domain, 0, 0, settings, language);
}

ACTION dappsurf::devclearall(){
  // FOR DEVELOPMENT/TESTING NETWORKS ONLY: delete this action if shipping to the mainnet!
  require_auth(DEBUG_CONTRACT_ADMIN);

  auto pages_iterator = tbl_websites.begin();
  while (pages_iterator != tbl_websites.end()) {
    pages_iterator = tbl_websites.erase(pages_iterator);
  }

  auto wsmetadata_iterator = tbl_wsmetadata.begin();
  while (wsmetadata_iterator != tbl_wsmetadata.end()) {
    wsmetadata_iterator = tbl_wsmetadata.erase(wsmetadata_iterator);
  }
}
