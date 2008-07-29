/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_H
#define ZYPPER_UTILS_H

#include <ostream>
#include <string>

#include "zypp/Url.h"
#include "zypp/Resolvable.h"
#include "zypp/Pathname.h"
#include "zypp/RepoInfo.h"
#include "zypp/Service.h"

#include "Zypper.h"

typedef std::set<zypp::ResKind> ResKindSet;

std::string readline_getline();

bool is_changeable_media(const zypp::Url & url);

std::string kind_to_string_localized(
    const zypp::Resolvable::Kind & kind, unsigned long count);

bool equalNVRA(const zypp::Resolvable & lhs, const zypp::Resolvable & rhs);

/**
 * Creates a Url out of \a urls_s. If the url_s looks looks_like_url()
 * Url(url_s) is returned. Otherwise if \a url_s represends a valid path to
 * a file or directory, a dir:// Url is returned. Otherwise an empty Url is
 * returned. 
 */
zypp::Url make_url (const std::string & url_s);

/**
 * Returns <code>true</code> if the string \a s contains a substring starting
 * at the beginning and ending before the first colon matches any of registered
 * schemes (Url::isRegisteredScheme()). 
 */
bool looks_like_url (const std::string& s);

/**
 * Returns <code>true</code> if \a s ends with ".rpm" or starts with "/", "./",
 * or "../". 
 */
bool looks_like_rpm_file(const std::string & s);

/**
 * Download the RPM file specified by \a rpm_uri_str and copy it into
 * \a cache_dir.
 * 
 * \return The local Pathname of the file in the cache on success, empty
 *      Pathname if a problem occurs. 
 */
zypp::Pathname cache_rpm(const std::string & rpm_uri_str,
                         const std::string & cache_dir);

std::string xml_encode(const std::string & text);

std::string & indent(std::string & text, int columns);

zypp::Capability safe_parse_cap (Zypper & zypper,
                                 const std::string & capstr,
                                 const zypp::ResKind & kind = zypp::ResKind::nokind);


// comparator for RepoInfo set
class RepoInfoAliasComparator
{
  public: bool operator()(const zypp::RepoInfo & a, const zypp::RepoInfo & b)
  {
    return a.alias() < b.alias();
  }
};

// comparator for Service set
class ServiceAliasComparator
{
  public: bool operator()(const zypp::Service & a, const zypp::Service & b)
  {
    return a.alias() < b.alias();
  }
};


/**
 * checks name for .repo string
 */
inline bool isRepoFile(const std::string& name)
{
  return name.find(".repo") != name.npos ;
}

#endif /*ZYPPER_UTILS_H*/