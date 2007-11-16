/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#ifndef ZYPPERSEARCH_H_
#define ZYPPERSEARCH_H_

#include <string>
#include <vector>
#include <boost/regex.hpp>
#include <boost/function.hpp>
#include <zypp/ZYpp.h>
#include <zypp/base/Hash.h>
#include <zypp/cache/ResolvableQuery.h>
#include <zypp/RepoManager.h>

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-utils.h"
#include "zypper-getopt.h"
#include "zypper-tabulator.h"

extern Settings gSettings;

/**
 * Represents zypper search options.
 */
class ZyppSearchOptions {
public:
  enum InsFilter {
    ALL,
    INSTALLED_ONLY,
    UNINSTALLED_ONLY
  };

  ZyppSearchOptions();

  void resolveConflicts();

  InsFilter installedFilter() const { return _ifilter; }
  bool matchAll() const { return _match_all; }
  bool matchAny() const { return !_match_all; }
  bool matchWords() const { return _match_words; }
  bool matchExact() const { return _match_exact; }
  bool searchDescriptions() const { return _search_descriptions; }
  bool caseSensitive() const { return _case_sensitive; }
  const std::vector<zypp::Resolvable::Kind> & kinds() const { return _kinds; }
  const std::vector<std::string> & repos() const { return _repos; }

  void setInstalledFilter(const InsFilter ifilter) { _ifilter =  ifilter; }
  void setMatchAll(const bool match_all = true) { _match_all = match_all; }
  void setMatchAny(const bool match_any = true) { _match_all = !match_any; }
  void setMatchWords(const bool match_words = true) { _match_words = match_words; }
  void setMatchExact(const bool match_exact = true) { _match_exact = match_exact; }
  void setSearchDescriptions(const bool search_descriptions = true) { _search_descriptions = search_descriptions; }
  void setCaseSensitive(const bool case_sensitive = true) { _case_sensitive = case_sensitive; }
  // reset list of to-be-matched resolvable kinds
  void clearKinds( ) { _kinds.clear(); }
  // add a resolvable kind to match
  void addKind( const zypp::Resolvable::Kind & kind ) { _kinds.push_back( kind ); }
  // reset list of to-be-matched repository aliases
  void clearRepos( ) { _repos.clear(); }
  // add a repository alias to match
  void addRepo( const std::string & repo ) { _repos.push_back( repo ); }

private:
  InsFilter _ifilter;
  bool _match_all;
  bool _match_words;
  bool _match_exact;
  bool _search_descriptions;
  bool _case_sensitive;
  std::vector <zypp::Resolvable::Kind> _kinds;
  std::vector <std::string> _repos;
};

struct GenericStringHash {
  size_t operator()(const std::string &str) const {
    const std::string::size_type size = str.size();
    size_t h = 0; 
    for(std::string::size_type i = 0; i < size; i++) {
      h = 5 * h + str[i];
    }
    return h;
  }
};

typedef zypp::hash_map<std::string, zypp::PoolItem> PoolItemHash;

/**
 * Structure for caching installed PoolItems using a hash map.
 * Name + (if _incl_kind_in_key) kind is used as a key.
 * The hash map is to be manipulated through addItem() and getItem() methods. 
 */
struct InstalledCache  {
private:
  PoolItemHash _items;
  bool _incl_kind_in_key;

public:
  InstalledCache(bool incl_kind_in_key = true) :
    _incl_kind_in_key(incl_kind_in_key)
    {}

  void setIncludeKindInKey(bool value = true) { _incl_kind_in_key = value; }
  bool includeKindInKey() { return _incl_kind_in_key; }

  std::string getKey(const zypp::PoolItem & pi) const {
    return pi.resolvable()->name() +
      (_incl_kind_in_key ? pi.resolvable()->kind().asString() : "");
  }

  std::string getKey( const zypp::data::ResObject_Ptr res ) const {
    return res->name +
      (_incl_kind_in_key ? res->kind.asString() : "");
  }

  void addItem(const zypp::PoolItem & pi) { _items[getKey(pi)] = pi; }

  zypp::PoolItem & getItem(const zypp::PoolItem & pi) {
    return _items[getKey(pi)];
  }
  
  zypp::PoolItem & getItem( const zypp::data::ResObject_Ptr res ) {
    return _items[getKey( res )];
  }
  
  unsigned int size() {
    return _items.size();
  }

  /** defined for use as a functor for filling the hashmap in a for_each */ 
  bool operator()(const zypp::PoolItem & pi) {
    addItem(pi);
    return true;
  }

  /** defined for use as a functor for filling the hashmap in a for_each */ 
  bool operator()(const zypp::data::RecordId & id, const zypp::data::ResObject_Ptr res) {
    // dummy, since zypp::data::ResObject_Ptr is never 'installed'
    return true;
  }

};


typedef zypp::hash_set<std::string> IdSet;

/**
 * Structure for caching identification strings of source PoolItems using
 * a hash set. Name + edition + kind + architecture is used as a key.
 * The has set is to be manipulated through addItem() and contains() methods. 
 */
struct IdCache {
private:
  IdSet _items;

public:
  std::string getKey(const zypp::PoolItem & pi) const {
    return pi.resolvable()->name() + pi.resolvable()->edition().asString() +
      pi.resolvable()->kind().asString() + pi.resolvable()->arch().asString();
  }

  std::string getKey(const zypp::data::ResObject_Ptr res) const {
    return res->name + res->edition.asString() +
      res->kind.asString() + res->arch.asString();
  }

  void addItem(const zypp::PoolItem & pi) { _items.insert(getKey(pi)); }
  void addItem(const zypp::data::ResObject_Ptr res) { _items.insert(getKey(res)); }

  bool contains(const zypp::PoolItem & pi) {
    return _items.count(getKey(pi));
  }

  bool contains(const zypp::data::ResObject_Ptr res) {
    return _items.count(getKey(res));
  }

  /** defined for use as a functor for filling the IdSet in a for_each */ 
  bool operator()(const zypp::PoolItem & pi) {
    addItem(pi);
    return true;
  }

  /** defined for use as a functor for filling the IdSet in a for_each */ 
  bool operator()(const zypp::data::RecordId & id, const zypp::data::ResObject_Ptr res) {
    addItem( res );
    return true;
  }

  int size() { return _items.size(); }
};

/**
 * TODO
 */
class ZyppSearch {

public:
  ZyppSearch (zypp::ZYpp::Ptr & zypp, const ZyppSearchOptions & options,
      const std::vector<std::string> qstrings = std::vector<std::string>());

  void doSearch(const boost::function<bool(const zypp::PoolItem &)> & f, const zypp::cache::ProcessResolvable & r);

  InstalledCache & installedCache() { return _icache; }

  template <class _Filter, class _PoolCallback, class _CacheCallback>
    int invokeOnEachSearched(_Filter filter_r, _PoolCallback pool_cb, _CacheCallback cache_cb);

  zypp::cache::ResolvableQuery *getQueryInstancePtr( void ) { return &_query; }

  const ZyppSearchOptions &options() const { return _options; }

private:
  zypp::ZYpp::Ptr & _zypp;
  const ZyppSearchOptions & _options;
  const std::vector<std::string> _qstrings;
  boost::regex _reg;

  InstalledCache _icache;
  IdCache _idcache;

  zypp::cache::ResolvableQuery _query;

  void setupRegexp();
  void cacheInstalled();
  std::string wildcards2regex(const std::string & str) const;
};

/**
 * Filter resolvables by their installed status.
 * This filter does it by comparing resolvables with matching resolvables
 * from the InstalledCache. A resolvable is considered to be installed if
 * its name, kind, edition, and architecture matches the one in installed
 * cache.
 * <p> 
 * Not an effective filter, surely, but can't find another way to do this.
 */
struct ByInstalledCache
{
  ByInstalledCache(InstalledCache & icache) :
    _icache(&icache)
    {}

  bool operator()(const zypp::PoolItem & pool_item) const {
    zypp::PoolItem inst_item = _icache->getItem(pool_item);
    if (inst_item) {
      // yes, this one is installed, indeed
      if (inst_item.resolvable()->edition() == pool_item.resolvable()->edition() &&
          inst_item.resolvable()->arch() == pool_item.resolvable()->arch()) {
        return true;
      }
      // nope, this package is not there on the target (in the InstalledCache)
      else {
        return false;
      }
    }

    return false;
  }

  InstalledCache * _icache;
};

/**
 * Functor for filling search output table in rug style.
 */
struct FillTable
{
  FillTable( Table & table, InstalledCache & icache, zypp::cache::ResolvableQuery * query, const ZyppSearchOptions &options )
  : _table( &table )
  , _icache( &icache )
  , _query( query )
  , _options( options )
  {
    TableHeader header;

    // TranslatorExplanation S as Status
    header << _("S");

    if (gSettings.is_rug_compatible)
      header << _("Catalog");
    else
      header << _("Repository");

    if (gSettings.is_rug_compatible)
      // TranslatorExplanation This is Bundle in as used in rug.
      header << _("Bundle");
    else
      header << _("Type");

    header << _("Name") << _("Version") << _("Arch");

    *_table << header;
  }

  // PoolItem callback, called for installed resolvables

  bool operator()(const zypp::PoolItem & pool_item) const
  {
    TableRow row;

    std::vector<zypp::Resolvable::Kind>::const_iterator it;
    for (it = _options.kinds().begin(); it != _options.kinds().end(); ++it)
    {
      if (*it == pool_item.resolvable()->kind())
      {
        // add other fields to the result table
        row << "i"
	    << pool_item.resolvable()->repository().info().name()
            // TODO what about rug's Bundle?
            << (gSettings.is_rug_compatible ?
                "" : kind_to_string_localized(pool_item.resolvable()->kind(), 1))
            << pool_item.resolvable()->name()
            << pool_item.resolvable()->edition().asString()
            << pool_item.resolvable()->arch().asString();
        *_table << row;
        break;
      }
    }
    return true;
  }

  // RecordId callback, called for uninstalled resolvables

  bool operator()(const zypp::data::RecordId & id, const zypp::data::ResObject_Ptr res)
  {
    TableRow row;

    // add status to the result table
    zypp::PoolItem inst_item = _icache->getItem( res );
    if (inst_item
        && inst_item.resolvable()->arch() == res->arch)
    {
      if (inst_item.resolvable()->edition() == res->edition)
      {
        row << "i";
      }
      else
      {
        row << "v";
      }
    }
    // or it's not installed at all
    else {
      row << "";
    }

    string alias = _query->queryRepositoryAlias( res->repository );

    // add other fields to the result table
    row << alias
        // TODO what about rug's Bundle?
        << (gSettings.is_rug_compatible ?
            "" : kind_to_string_localized(res->kind, 1))
        << res->name
        << res->edition.asString()
        << res->arch.asString();
  
    *_table << row;

    return true;
  }

  // the table used for output
  Table * _table;

  // the cache of installed names (used for 'i'/'v'/' ' in first column)
  InstalledCache * _icache;

  // the db query interface, used to retrieve additional data like the repository alias
  zypp::cache::ResolvableQuery * _query;

  // the search options, contains i.e. the list of kinds to show
  const ZyppSearchOptions & _options;
};


/**
 * Filter functor for Matching PoolItems' names (or also summaries and
 * descriptions) with a regex created according to search criteria.
 */
struct Match {
  const boost::regex * _regex;
  const bool _search_descs;

  Match(const boost::regex & regex, bool search_descriptions = false) :
    _regex(&regex), _search_descs(search_descriptions)
    {}

  bool operator()(const zypp::PoolItem & pi) const {
    return
      // match resolvable name
      regex_match(pi.resolvable()->name(), *_regex)
        ||
      // if required, match also summary and description of the resolvable
      (_search_descs ?
        regex_match(pi.resolvable()->summary(), *_regex) ||
          regex_match(pi.resolvable()->description(), *_regex)
            :
        false);
  }
};

/**
 * Filters target poolitems which have their counterpart among repository
 * resolvables.
 */
struct DuplicateFilter {
  IdCache * _idcache;

  DuplicateFilter(IdCache & idcache) : _idcache(&idcache) {}

  bool operator()(const zypp::PoolItem & pi) const {
    //! \todo filter out those language resolvables
    return !(pi.status().isInstalled()
        && (_idcache->contains(pi) || copts.count("repo")));
  }
};

#endif /*ZYPPERSEARCH_H_*/
