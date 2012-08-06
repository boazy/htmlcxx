/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * This file is part of htmlcxx -- A simple non-validating css1 and html parser
 * written in C++.
 *
 * htmlcxx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Uri.h Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 * 
 * Copyright 2005-2010 Davi de Castro Reis and Robson Braga Araújo
 * Copyright 2011 David Hoerl
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Uri.h"

#include "wincstring.h"

#include <cstdlib>
#include <cassert>
#include "tld.h"

//#define DEBUG
#include "debug.h"

using namespace std;
using namespace htmlcxx;

/** Structure to store various schemes and their default ports */
struct schemes_t {
    /** The name of the scheme */
    const char *name;
    /** The default port for the scheme */
    unsigned int default_port;
};

/* Some WWW schemes and their default ports; this is basically /etc/services */
/* This will become global when the protocol abstraction comes */
/* As the schemes are searched by a linear search, */
/* they are sorted by their expected frequency */
static schemes_t schemes[] =
{
    {"http",     Uri::URI_HTTP_DEFAULT_PORT},
    {"ftp",      Uri::URI_FTP_DEFAULT_PORT},
    {"https",    Uri::URI_HTTPS_DEFAULT_PORT},
    {"gopher",   Uri::URI_GOPHER_DEFAULT_PORT},
    {"ldap",     Uri::URI_LDAP_DEFAULT_PORT},
    {"nntp",     Uri::URI_NNTP_DEFAULT_PORT},
    {"snews",    Uri::URI_SNEWS_DEFAULT_PORT},
    {"imap",     Uri::URI_IMAP_DEFAULT_PORT},
    {"pop",      Uri::URI_POP_DEFAULT_PORT},
    {"sip",      Uri::URI_SIP_DEFAULT_PORT},
    {"rtsp",     Uri::URI_RTSP_DEFAULT_PORT},
    {"wais",     Uri::URI_WAIS_DEFAULT_PORT},
    {"z39.50r",  Uri::URI_WAIS_DEFAULT_PORT},
    {"z39.50s",  Uri::URI_WAIS_DEFAULT_PORT},
    {"prospero", Uri::URI_PROSPERO_DEFAULT_PORT},
    {"nfs",      Uri::URI_NFS_DEFAULT_PORT},
    {"tip",      Uri::URI_TIP_DEFAULT_PORT},
    {"acap",     Uri::URI_ACAP_DEFAULT_PORT},
    {"telnet",   Uri::URI_TELNET_DEFAULT_PORT},
    {"ssh",      Uri::URI_SSH_DEFAULT_PORT},
    { NULL, 0xFFFF }     /* unknown port */
};

static unsigned int port_of_Scheme(const char *scheme_str)
{
    schemes_t *scheme;

    if (scheme_str) {
        for (scheme = schemes; scheme->name != NULL; ++scheme) {
            if (strcasecmp(scheme_str, scheme->name) == 0) {
                return scheme->default_port;
            }
        }
    }
    return 0;
}

/* We have a apr_table_t that we can index by character and it tells us if the
 * character is one of the interesting delimiters.  Note that we even get
 * compares for NUL for free -- it's just another delimiter.
 */

#define T_COLON           0x01        /* ':' */
#define T_SLASH           0x02        /* '/' */
#define T_QUESTION        0x04        /* '?' */
#define T_HASH            0x08        /* '#' */
#define T_NUL             0x80        /* '\0' */

/* the uri_delims.h file is autogenerated by gen_uri_delims.c */
/* this file is automatically generated by gen_uri_delims, do not edit */
static const unsigned char uri_delims[256] = {
    T_NUL,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,T_HASH,0,0,0,0,
    0,0,0,0,0,0,0,T_SLASH,0,0,0,0,0,0,0,0,0,0,T_COLON,0,
    0,0,0,T_QUESTION,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 
};

/* it works like this:
    if (uri_delims[ch] & NOTEND_foobar) {
        then we're not at a delimiter for foobar
    }
*/

/* Note that we optimize the scheme scanning here, we cheat and let the
 * compiler know that it doesn't have to do the & masking.
 */
#define NOTEND_SCHEME     (0xff)
#define NOTEND_HOSTINFO   (T_SLASH | T_QUESTION | T_HASH | T_NUL)
#define NOTEND_PATH       (T_QUESTION | T_HASH | T_NUL)


static size_t wwwPrefixOffset(const std::string& hostname);

Uri::Uri()
: mScheme(), mUser(), mPassword(), mHostname(), mPath(), mQuery(), mFragment(), mExistsQuery(false), mExistsFragment(false), mPort(0)
{}

Uri::Uri(const string &uri_str)
: mScheme(), mUser(), mPassword(), mHostname(), mPath(), mQuery(), mFragment(), mExistsQuery(false), mExistsFragment(false), mPort(0)
{
	init(uri_str);
}

void Uri::init(const string &uri_str)
{
	DEBUGP("Parsing uri %s\n", uri_str.c_str());

	if(uri_str.empty()) return;
	const char *uri = uri_str.c_str();
	const char *s;
	const char *s1;
	const char *hostinfo;
	char *endstr;
	
	/* We assume the processor has a branch predictor like most --
	 * it assumes forward branches are untaken and backwards are taken.  That's
	 * the reason for the gotos.  -djg
	 */
	if (uri[0] == '/') {
		deal_with_path:
		DEBUGP("Dealing with path\n");
		/* we expect uri to point to first character of path ... remember
		 * that the path could be empty -- http://foobar?query for example
		 */
		s = uri;
		while ((uri_delims[*(unsigned char *)s] & NOTEND_PATH) == 0) {
			++s;
		}
		if (s != uri) {
			mPath.assign(uri, s - uri);
			DEBUGP("Path is %s\n", mPath.c_str());
		}
		if (*s == 0) {
			return;
		}
		if (*s == '?') {
			++s;
			s1 = strchr(s, '#');
			if (s1) {
				mFragment.assign(s1 + 1);
				mExistsFragment = true;
				DEBUGP("Fragment is %s\n", mFragment.c_str());
				mQuery.assign(s, s1 - s);
				mExistsQuery = true;
				DEBUGP("Query is %s\n", mQuery.c_str());
			}
			else {
				mQuery.assign(s);
				mExistsQuery = true;
				DEBUGP("Query is %s\n", mQuery.c_str());
			}
			return;
		}
		/* otherwise it's a fragment */
		mFragment.assign(s + 1);
		mExistsFragment = true;
		DEBUGP("Fragment is %s\n", mFragment.c_str());
		return;
	}

	DEBUGP("Dealing with scheme\n");
	/* find the scheme: */
	if (!isalpha(*uri)) goto deal_with_path;
	s = uri;
	while ((uri_delims[*(unsigned char *)s] & NOTEND_SCHEME) == 0) {
		++s;
	}
	/* scheme must be non-empty and followed by :// */
	if (s == uri || s[0] != ':' || s[1] != '/' || s[2] != '/') {
		goto deal_with_path;        /* backwards predicted taken! */
	}

	mScheme.assign(uri, s - uri);
	DEBUGP("Scheme is %s\n", mScheme.c_str());
	s += 3;

	DEBUGP("Finding hostinfo\n");
	hostinfo = s;
	DEBUGP("Hostinfo is %s\n", hostinfo);
	while ((uri_delims[*(unsigned char *)s] & NOTEND_HOSTINFO) == 0) {
		++s;
	}
	uri = s;        /* whatever follows hostinfo is start of uri */
//	mHostinfo.assign(hostinfo, uri - hostinfo);

	/* If there's a username:password@host:port, the @ we want is the last @...
	 * too bad there's no memrchr()... For the C purists, note that hostinfo
	 * is definately not the first character of the original uri so therefore
	 * &hostinfo[-1] < &hostinfo[0] ... and this loop is valid C.
	 */
	do {
		--s;
	} while (s >= hostinfo && *s != '@');
	if (s < hostinfo) {
		/* again we want the common case to be fall through */
deal_with_host:
		DEBUGP("Dealing with host\n");
		/* We expect hostinfo to point to the first character of
		 * the hostname.  If there's a port it is the first colon.
		 */
		s = (char *)memchr(hostinfo, ':', uri - hostinfo);
		if (s == NULL) {
			/* we expect the common case to have no port */
			mHostname.assign(hostinfo, uri - hostinfo);
			DEBUGP("Hostname is %s\n", mHostname.c_str());
			goto deal_with_path;
		}
		mHostname.assign(hostinfo, s - hostinfo);
		DEBUGP("Hostname is %s\n", mHostname.c_str());
		++s;
		if (uri != s) {
			mPortStr.assign(s, uri - s);
			mPort = (unsigned int)strtol(mPortStr.c_str(), &endstr, 10);
			if (*endstr == '\0') {
				goto deal_with_path;
			}
			/* Invalid characters after ':' found */
			DEBUGP("Throwing invalid url exception\n");
			throw Exception("Invalid character after ':'");
		}
		this->mPort = port_of_Scheme(mScheme.c_str());
		goto deal_with_path;
	}

	/* first colon delimits username:password */
	s1 = (char *)memchr(hostinfo, ':', s - hostinfo);
	if (s1) {
		mUser.assign(hostinfo, s1 - hostinfo);
		++s1;
		mPassword.assign(s1, s - s1);
	}
	else {
		mUser.assign(hostinfo, s - hostinfo);
	}
	hostinfo = s + 1;
	goto deal_with_host;
}

Uri::~Uri() {
}

string Uri::scheme() const { return mScheme; }

void Uri::scheme(string scheme) {
	mScheme = scheme;
}
string Uri::user() const { return mUser; }
void Uri::user(string user) {
	mUser = user;
}
string Uri::password() const { return mPassword; }
void Uri::password(string password) {
	mPassword = password;
}
string Uri::hostname() const { return mHostname; }
void Uri::hostname(string hostname) {
	mHostname = hostname;
}
string Uri::path() const { return mPath; }
void Uri::path(string path) {
	mPath = path;
}
bool Uri::existsFragment() const { return mExistsFragment; }
void Uri::existsFragment(bool existsFragment) {
	mExistsFragment = existsFragment;
}
bool Uri::existsQuery() const { return mExistsQuery; }
void Uri::existsQuery(bool existsQuery) {
	mExistsQuery = existsQuery;
}
string Uri::query() const { return mQuery; }
void Uri::query(string query) {
	mQuery = query;
}
string Uri::fragment() const { return mFragment; }
void Uri::fragment(string fragment) {
	mFragment = fragment;
}
unsigned int Uri::port() const { return mPort; }
void Uri::port(unsigned int port) { mPort = port; }

static const char *default_filenames[] = { "index", "default", NULL };
static const char *default_extensions[] = { ".html", ".htm", ".php", ".shtml", ".asp", ".cgi", NULL };

static unsigned short default_port_for_scheme(const char *scheme_str)
{
	schemes_t *scheme;

	if (scheme_str == NULL)
		return 0;

	for (scheme = schemes; scheme->name != NULL; ++scheme)
		if (strcasecmp(scheme_str, scheme->name) == 0)
			return (unsigned short)scheme->default_port;

	return 0;
}

Uri Uri::absolute(const Uri &base) const
{
	if (mScheme.empty())
	{
		Uri root(base);

		if (root.mPath.empty()) root.mPath = "/";

		if (mPath.empty())
		{
			if (mExistsQuery)
			{
				root.mQuery = mQuery;
				root.mExistsQuery = mExistsQuery;
				root.mFragment = mFragment;
				root.mExistsFragment = mExistsFragment;
			}
			else if (mExistsFragment)
			{
				root.mFragment = mFragment;
				root.mExistsFragment = mExistsFragment;
			}
		}
		else if (mPath[0] == '/')
		{
			root.mPath = mPath;
			root.mQuery = mQuery;
			root.mExistsQuery = mExistsQuery;
			root.mFragment = mFragment;
			root.mExistsFragment = mExistsFragment;
		}
		else
		{
			string path(root.mPath);
			string::size_type find;
			find = path.rfind("/");
			if (find != string::npos) path.erase(find+1);
			path += mPath;
			root.mPath = path;
			root.mQuery = mQuery;
			root.mExistsQuery = mExistsQuery;
			root.mFragment = mFragment;
			root.mExistsFragment = mExistsFragment;
		}

		return root;
	}

	if (mPath.empty())
	{
		Uri root(*this);
		root.mPath = "/";

		return root;
	}

	return *this;
}

string Uri::unparse(int flags ) const
{
	string ret;
	ret.reserve(mScheme.length() + mUser.length() + mPassword.length() + mHostname.length() + mPath.length() + mQuery.length() + mFragment.length() + mPortStr.length());

	DEBUGP("Unparsing scheme\n");
	if(!(Uri::REMOVE_SCHEME & flags)) {
		if(!mScheme.empty()) {
			ret +=  mScheme;
			ret += "://";
		}
	}
	DEBUGP("Unparsing hostname\n");
	if(!mHostname.empty()) { 
		size_t offset = 0;
		if(flags & Uri::REMOVE_WWW_PREFIX && mHostname.length() > 3) {
			offset = wwwPrefixOffset(mHostname);
		}
		ret += (mHostname.c_str() + offset);
	}
	DEBUGP("Unparsing port\n");
	if (!mPortStr.empty() && !(!mScheme.empty() && mPort == default_port_for_scheme(mScheme.c_str())))
	{
		ret += ':';
		ret += mPortStr;
	}
	DEBUGP("Unparsing path\n");
	if(!mPath.empty()) 
	{
		char *buf = new char[mPath.length() + 1];
		memcpy(buf, mPath.c_str(), mPath.length() + 1);
		if(flags & Uri::REMOVE_DEFAULT_FILENAMES) {
			const char **ptr = default_extensions;
			char *end = buf + mPath.length();
			size_t offset = 0;
			while(*ptr != NULL) {
				size_t len = strlen(*ptr);
				if((strcmp(end - len, *ptr)) == 0) {
					offset = len;
					break;
				}
				++ptr;
			} 
			if(offset == 0) goto remove_bar;
			ptr = default_filenames;
			bool found = false;
			while(*ptr != NULL) {
				size_t len = strlen(*ptr);
				if(strncmp(end - offset - len, *ptr, len) == 0) {
					offset += len; 
					found = true;
					break;
				}
				++ptr;
			}
			if(found) {
				*(end - offset) = 0; //cut filename
			}

		}
		remove_bar:
		if(flags & Uri::REMOVE_TRAILING_BAR) {
			if(strlen(buf) > 1 && buf[strlen(buf) - 1] == '/') { //do not remove if path is only the bar
				buf[strlen(buf) - 1] = 0;
			}
		} 
		ret += buf;
		delete [] buf;
	}
	DEBUGP("Unparsing query\n");
	if(!(flags & Uri::REMOVE_QUERY) && mExistsQuery) {
		ret += '?';
		if(flags & Uri::REMOVE_QUERY_VALUES) {
			const char *ptr = mQuery.c_str();
			bool inside = false;
			while(*ptr) {
				if(*ptr == '=') {
					inside = true;
				}
				if(*ptr == '&') {
					inside = false;
				}
				if(inside) {
					++ptr;
				} else {
					ret += *ptr;
					++ptr;
				}
			}
		} else {
			ret += mQuery;
		}
	}
	DEBUGP("Unparsing fragment\n");
	if(!(flags & Uri::REMOVE_FRAGMENT) && mExistsFragment)
	{
		ret += '#';
		ret += mFragment;
	}

	return ret;
}

static size_t wwwPrefixOffset(const std::string& hostname) 
{
	string::size_type len = hostname.length();
	if(strncasecmp("www", hostname.c_str(), 3) == 0)
	{
		if(len > 3 && hostname[3] == '.') 
		{
			return 4;
		}
		if(len > 4 && isdigit(hostname[3]) && hostname[4] == '.')
		{
			return 5;
		}
	}
	return 0;
}


	

std::string Uri::canonicalHostname(unsigned int maxDepth) const
{

	size_t prefixOffset = wwwPrefixOffset(mHostname);
	size_t suffixOffset = tldOffset(mHostname.c_str());
	unsigned int depth = 0;
	string::const_iterator canonicalStart = mHostname.begin() + prefixOffset;
	string::const_iterator ptr = mHostname.begin();
	ptr += mHostname.length() - suffixOffset;
	while (depth < maxDepth && ptr > canonicalStart) 
	{
		--ptr;
		if (*ptr == '.') ++depth;
	}
	if (*ptr == '.') ++ptr;
	return string(ptr, mHostname.end());
}

std::string Uri::decode(const std::string &uri)
{
    //Note from RFC1630:  "Sequences which start with a percent sign
    //but are not followed by two hexadecimal characters (0-9,A-F) are reserved
    //for future extension"
	const unsigned char *ptr = (const unsigned char *)uri.c_str();
	string ret;
	ret.reserve(uri.length());
	for (; *ptr; ++ptr)
	{
		if (*ptr == '%')
		{
			if (*(ptr + 1))
			{
				char a = *(ptr + 1);
				char b = *(ptr + 2);
				if (!((a >= 0x30 && a < 0x40) || (a >= 0x41 && a < 0x47))) continue;
				if (!((b >= 0x30 && b < 0x40) || (b >= 0x41 && b < 0x47))) continue;
				char buf[3];
				buf[0] = a;
				buf[1] = b;
				buf[2] = 0;
				ret += (char)strtoul(buf, NULL, 16);
				ptr += 2;
				continue;
			}
		}
		ret += *ptr;
	}
	return ret;
}

//This vector is generated by safechars.py. Please do not edit by hand.
static const char safe[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


std::string Uri::encode(const std::string &uri)
{
	string ret;	
	const unsigned char *ptr = (const unsigned char *)uri.c_str();
	ret.reserve(uri.length());

	for (; *ptr ; ++ptr)
	{
		if (!safe[*ptr]) 
		{
			char buf[6];
			memset(buf, 0, 6);
			snprintf(buf, 5, "%%%X", (*ptr));
			ret.append(buf); 	
		}
		else 
		{
			ret += *ptr;
		}
	}
	return ret;
}

