/**
 * Created by TekuConcept on July 31, 2018
 */

#include <gtest/gtest.h>
#include <rfc/uri.h>
#include <impact_error>

using namespace impact;

#define NO_THROW(code) try { {code} } catch (...) { FAIL(); }
#define THROWS(code) try { {code} FAIL(); } catch (...) {}

TEST(test_uri, scheme) {
    EXPECT_TRUE(uri::parse("", NULL));
    // first char must be ALPHA
    EXPECT_FALSE(uri::parse("0abc:", NULL));
    // scheme must be followed by ":" delimiter
    EXPECT_FALSE(uri::parse("http", NULL));
    
    uri u;
    EXPECT_TRUE(uri::parse("http:", &u));
    EXPECT_EQ(u.scheme(), "http");
}

TEST(test_uri, hier_part) {
    // path: empty
    ASSERT_TRUE(uri::parse("foo:", NULL));
    
    uri u;
    
    // path: rootless
    EXPECT_TRUE(uri::parse("foo::", &u));
    EXPECT_EQ(u.path(), ":");
    EXPECT_TRUE(uri::parse("foo:@", &u));
    EXPECT_EQ(u.path(), "@");
    EXPECT_TRUE(uri::parse("foo:bar/baz", &u));
    EXPECT_EQ(u.path(), "bar/baz");
    
    // paths cannot have "//" in it
    EXPECT_FALSE(uri::parse("foo:bar//baz", NULL));
    
    // path: absolute, empty
    EXPECT_TRUE(uri::parse("foo:/", &u));
    EXPECT_EQ(u.path(), "/");
    
    // path: absolute
    EXPECT_TRUE(uri::parse("foo:/bar", &u));
    EXPECT_EQ(u.path(), "/bar");
    EXPECT_TRUE(uri::parse("foo:/bar/", &u));
    EXPECT_EQ(u.path(), "/bar/");
    
    // paths containing pchars
    EXPECT_TRUE(uri::parse("foo:/AZaz09-._~%2F!$&'()*+,;=/", &u));
    EXPECT_EQ(u.path(), "/AZaz09-._~%2F!$&'()*+,;=/");
}

TEST(test_uri, authority) {
    uri u;
    
    EXPECT_TRUE(uri::parse("foo://", &u));
    EXPECT_EQ(u.authority(), "");
    EXPECT_EQ(u.str(), "foo://");
    EXPECT_TRUE(uri::parse("foo://bar.baz", &u));
    EXPECT_EQ(u.authority(), "bar.baz");
    EXPECT_EQ(u.userinfo(), "");
    EXPECT_EQ(u.port(), -1);
    
    EXPECT_TRUE(uri::parse("foo://bar@baz", &u));
    EXPECT_EQ(u.userinfo(), "bar");
    EXPECT_EQ(u.host(), "baz");
    
    std::string v("foo://bar\x00", 10);
    EXPECT_FALSE(uri::parse(v, NULL));
    v.assign("foo://bar@baz\x00", 14);
    EXPECT_FALSE(uri::parse(v, NULL));
    
    // test parser ambiguity
    
    EXPECT_TRUE(uri::parse("foo://usr:@baz", &u));
    EXPECT_EQ(u.userinfo(), "usr:");
    EXPECT_EQ(u.host(), "baz");
    
    EXPECT_TRUE(uri::parse("foo://usr:pwd@baz:42", &u));
    EXPECT_EQ(u.userinfo(), "usr:pwd");
    EXPECT_EQ(u.host(), "baz");
    EXPECT_EQ(u.port(), 42);
    
    EXPECT_TRUE(uri::parse("foo://usr:45@baz:42", &u));
    EXPECT_EQ(u.userinfo(), "usr:45");
    EXPECT_EQ(u.host(), "baz");
    EXPECT_EQ(u.port(), 42);
}

TEST(test_uri, IPv6) {
    EXPECT_FALSE(uri::parse("foo://[]", NULL));  // minimum 2 chars required
    EXPECT_FALSE(uri::parse("foo://[:]", NULL)); // minimum 2 chars required
    
    EXPECT_TRUE(uri::parse("foo://[::0.0.0.0]", NULL));
    EXPECT_TRUE(uri::parse("foo://[::255.255.255.255]", NULL));
    
    // // malformed ipv4
    EXPECT_FALSE(uri::parse("foo://[::256.256.256.256]", NULL)); // dec-octet values greater than 255
    EXPECT_FALSE(uri::parse("foo://[::A.0.A.0]", NULL));   // dec-octet containing hex/non-dec values
    EXPECT_FALSE(uri::parse("foo://[::00..0.0]", NULL));   // empty dec-octet
    EXPECT_FALSE(uri::parse("foo://[::.0.0.0.0]", NULL));  // ipv4 begining with .
    EXPECT_FALSE(uri::parse("foo://[::0.0.0.0.]", NULL));  // ipv4 ending with .
    EXPECT_FALSE(uri::parse("foo://[::0.0.0000.0]", NULL));// dec-octet with more than 3 digits
    EXPECT_FALSE(uri::parse("foo://[::0.0.0]", NULL));     // missing dec-octet(s) (see also h16 with .)
    EXPECT_FALSE(uri::parse("foo://[G:0.0.0.0]", NULL));   // non-hex prefix (see also h16 with non-hex)
    
    EXPECT_TRUE(uri::parse("foo://[::]", NULL));
    EXPECT_TRUE(uri::parse("foo://[0:00:000:0000:F:FF:FFF:FFFF]", NULL));
    EXPECT_TRUE(uri::parse("foo://[::0]", NULL));
    EXPECT_TRUE(uri::parse("foo://[0::]", NULL));
    
    // // malformed ipv6
    EXPECT_FALSE(uri::parse("foo://[0:0::0:0::]", NULL));  // duplicate placeholders
    EXPECT_FALSE(uri::parse("foo://[0:0:0.0::0:0]", NULL));// h16 with .
    EXPECT_FALSE(uri::parse("foo://[0:0G0:F::0]", NULL));  // h16 with non-hex
    EXPECT_FALSE(uri::parse("foo://[:0::0]", NULL));       // missing h16
    EXPECT_FALSE(uri::parse("foo://[0::0:]", NULL));       // missing h16
    EXPECT_FALSE(uri::parse("foo://[0:00000:00::]", NULL));// h16 with more than 4 digits
    // // a single h16 group requires a "::" placeholder
    // // also, ":" must be followed by either hex or ":"
    EXPECT_FALSE(uri::parse("foo://:F]", NULL));
    
    EXPECT_FALSE(uri::parse("foo://[0:0.0.0.0]", NULL)); // no placeholder
}

TEST(test_uri, parse_authority) {
    struct uri_authority authority;
    
    EXPECT_TRUE(uri::parse_authority("", &authority));
    EXPECT_EQ(authority.userinfo, "");
    EXPECT_EQ(authority.host, "");
    EXPECT_EQ(authority.port, -1);
    
    EXPECT_TRUE(uri::parse_authority("", NULL));
    
    EXPECT_TRUE(uri::parse_authority("usr@foo.bar:80", &authority));
    EXPECT_EQ(authority.userinfo, "usr");
    EXPECT_EQ(authority.host, "foo.bar");
    EXPECT_EQ(authority.port, 80);
    
    EXPECT_TRUE(uri::parse_authority("www.example.com", &authority));
    EXPECT_EQ(authority.userinfo, "");
    EXPECT_EQ(authority.host, "www.example.com");
    EXPECT_EQ(authority.port, -1);
    
    EXPECT_FALSE(uri::parse_authority("fo[o/b]ar", &authority));
    EXPECT_FALSE(uri::parse_authority(" ", &authority));
    EXPECT_FALSE(uri::parse_authority(" ", NULL));
}

TEST(test_uri, parse_resource) {
    struct uri_resource resource;
    
    EXPECT_TRUE(uri::parse_resource("", &resource));
    EXPECT_EQ(resource.authority.userinfo, "");
    EXPECT_EQ(resource.authority.host, "");
    EXPECT_EQ(resource.authority.port, -1);
    EXPECT_EQ(resource.path, "");
    EXPECT_EQ(resource.query, "");
    EXPECT_EQ(resource.fragment, "");
    
    EXPECT_TRUE(uri::parse_resource("", NULL));
    
    EXPECT_TRUE(uri::parse_resource("/path/to/resource?query#fragment",
        &resource));
    EXPECT_EQ(resource.path, "/path/to/resource");
    EXPECT_EQ(resource.query, "query");
    EXPECT_EQ(resource.fragment, "fragment");
    
    EXPECT_TRUE(uri::parse_resource("//user@host:20/path/to/resource",
        &resource));
    EXPECT_EQ(resource.authority.userinfo, "user");
    EXPECT_EQ(resource.authority.host, "host");
    EXPECT_EQ(resource.authority.port, 20);
    EXPECT_EQ(resource.path, "/path/to/resource");
    
    EXPECT_TRUE(uri::parse_resource("path/to/resource", &resource));
    EXPECT_EQ(resource.path, "path/to/resource");
    
    EXPECT_TRUE(uri::parse_resource("/path", NULL));
    
    EXPECT_FALSE(uri::parse_resource("path//is/invalid", &resource));
}

TEST(test_uri, query) {
    uri u;
    
    EXPECT_TRUE(uri::parse("foo:?", &u));
    EXPECT_EQ(u.query(), "");
    EXPECT_TRUE(uri::parse("foo:/?", &u));
    EXPECT_EQ(u.query(), "");
    EXPECT_TRUE(uri::parse("foo:/bar?baz", &u));
    EXPECT_EQ(u.query(), "baz");
    EXPECT_TRUE(uri::parse("foo:/bar?baz#jin", &u));
    EXPECT_EQ(u.query(), "baz");
    
    // all query chars
    EXPECT_TRUE(uri::parse("foo:/?AZaz09-._~%2F!$&'()*+,;=/?", &u));
    EXPECT_EQ(u.query(), "AZaz09-._~%2F!$&'()*+,;=/?");
    
    EXPECT_TRUE(uri::parse("foo://?", NULL));
    EXPECT_TRUE(uri::parse("foo://bar?baz", &u));
    EXPECT_EQ(u.query(), "baz");
}

TEST(test_uri, fragment) {
    uri u;
    
    EXPECT_TRUE(uri::parse("foo:#", &u));
    EXPECT_EQ(u.fragment(), "");
    EXPECT_TRUE(uri::parse("foo:/#", &u));
    EXPECT_EQ(u.fragment(), "");
    EXPECT_TRUE(uri::parse("foo:/bar#baz", &u));
    EXPECT_EQ(u.fragment(), "baz");
    EXPECT_TRUE(uri::parse("foo:/bar?baz#jin", &u));
    EXPECT_EQ(u.fragment(), "jin");
    
    // all chars ('\x00'-'\xFF') after '#' matched for fragment
    
    EXPECT_TRUE(uri::parse("foo://#", NULL));
    EXPECT_TRUE(uri::parse("foo://bar#baz", &u));
    EXPECT_EQ(u.fragment(), "baz");
}

TEST(test_uri, examples) {
    uri u;
    
    EXPECT_TRUE(uri::parse("file:///path/to/log.txt", &u));
    EXPECT_EQ(u.scheme(), "file");
    EXPECT_EQ(u.authority(), "");
    EXPECT_EQ(u.path(), "/path/to/log.txt");
    
    // RFC2732 (IPv6)
    EXPECT_TRUE(uri::parse("http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html", NULL));
    EXPECT_TRUE(uri::parse("http://[1080:0:0:0:8:800:200C:417A]/index.html", NULL));
    EXPECT_TRUE(uri::parse("http://[3ffe:2a00:100:7031::1]", NULL));
    EXPECT_TRUE(uri::parse("http://[1080::8:800:200C:417A]/foo", NULL));
    EXPECT_TRUE(uri::parse("http://[::192.9.5.5]/ipng", NULL));
    EXPECT_TRUE(uri::parse("http://[::FFFF:129.144.52.38]:80/index.html", NULL));
    EXPECT_TRUE(uri::parse("http://[2010:836B:4179::836B:4179]", NULL));
    
    // RFC6068 (mailto)
    EXPECT_TRUE(uri::parse("mailto:user@example.org?subject=caf", NULL));
    
    // Default ports
    EXPECT_TRUE(uri::parse("http://www.example.com/", &u));
    EXPECT_EQ(u.scheme(), "http");
    EXPECT_EQ(u.host(), "www.example.com");
    EXPECT_EQ(u.port(), 80);
    EXPECT_EQ(u.path(), "/");
    
    EXPECT_TRUE(uri::parse("http://www.example.com:80/", &u));
    EXPECT_EQ(u.str(), "http://www.example.com/");
    EXPECT_TRUE(uri::parse("http://www.example.com:3000/", &u));
    EXPECT_EQ(u.str(), "http://www.example.com:3000/");
    
    EXPECT_TRUE(uri::parse("http://[::]:3000/", &u));
    EXPECT_EQ(u.str(), "http://[::]:3000/");
    EXPECT_EQ(u.port(), 3000);
}

TEST(test_uri, normalize) {
    uri u;
    
    // normalized scheme to lower
    EXPECT_TRUE(uri::parse("FoO:", &u));
    EXPECT_EQ(u.scheme(), "foo");

    // normalized host to lower
    EXPECT_TRUE(uri::parse("foo://BaR", &u));
    EXPECT_EQ(u.host(), "bar");
    
    // path not normalized to lower
    EXPECT_TRUE(uri::parse("foo:/Path/TO/File", &u));
    EXPECT_EQ(u.path(), "/Path/TO/File");
    
    // normalized host to lower
    EXPECT_TRUE(uri::parse("foo://BaR", &u));
    EXPECT_EQ(u.host(), "bar");
    
    // path not normalized to lower
    EXPECT_TRUE(uri::parse("foo:/Path/TO/File", &u));
    EXPECT_EQ(u.path(), "/Path/TO/File");
    
    // normalized unreserved %-encoded data to mapped values
    EXPECT_TRUE(uri::parse("foo:/%2E%2E%2F", &u));
    EXPECT_EQ(u.path(), "/..%2F");
    
    // normalized reserved %-encoded values to upper case
    EXPECT_TRUE(uri::parse("foo:/%2f", &u));
    EXPECT_EQ(u.path(), "/%2F");
    
    // malformed %-encoded data
    EXPECT_FALSE(uri::parse("f%6Fo:", NULL));   // %-encoded scheme
    EXPECT_FALSE(uri::parse("foo:/%", NULL));   // no hex digits
    EXPECT_FALSE(uri::parse("foo:/%0", NULL));  // only one hex digit
    EXPECT_FALSE(uri::parse("foo:/%XZ", NULL)); // non-hex digits
    
    // path normalization
    EXPECT_TRUE(uri::parse("foo:./bar", &u));
    EXPECT_EQ(u.path(), "bar");
    EXPECT_TRUE(uri::parse("foo:../bar", &u));
    EXPECT_EQ(u.path(), "bar");
    EXPECT_TRUE(uri::parse("foo:/./bar", &u));
    EXPECT_EQ(u.path(), "/bar");
    EXPECT_TRUE(uri::parse("foo:/../bar", &u));
    EXPECT_EQ(u.path(), "/bar");
    EXPECT_TRUE(uri::parse("foo:./bar/", &u));
    EXPECT_EQ(u.path(), "bar/");
    EXPECT_TRUE(uri::parse("foo:../bar/", &u));
    EXPECT_EQ(u.path(), "bar/");
    EXPECT_TRUE(uri::parse("foo:/bar/..", &u));
    EXPECT_EQ(u.path(), "/bar/..");
    EXPECT_TRUE(uri::parse("foo:/bar/.", &u));
    EXPECT_EQ(u.path(), "/bar/.");
    EXPECT_TRUE(uri::parse("foo:/a/b/c/./../../g", &u));
    EXPECT_EQ(u.path(), "/a/g");
    EXPECT_TRUE(uri::parse("foo:mid/content=5/../6", &u));
    EXPECT_EQ(u.path(), "mid/6");
    
    // normalized strings
    EXPECT_TRUE(uri::parse("foo:/%25AE%7E", &u));
    // '%' is not decoded but '~' is
    // All reserved symbols as well as '%' are left encoded
    // while everything else is decoded
    EXPECT_EQ(u.path(), "/%25AE~");
    // '%' is decoded
    // Everything that hasn't been decoded previously is now decoded
    EXPECT_EQ(u.norm_path(), "/%AE~");
    
    EXPECT_TRUE(uri::parse("foo://bar/", &u));
    // when there are no encoded symbols,
    // the normalized is equal to the original
    EXPECT_EQ(u.authority(), "bar");
    EXPECT_EQ(u.norm_authority(), "bar");
}

/*
   URI-reference = URI / relative-ref
   
   relative-ref  = relative-part [ "?" query ] [ "#" fragment ]

   relative-part = "//" authority path-abempty
                 / path-absolute
                 / path-noscheme
                 / path-empty
   
   absolute-URI  = scheme ":" hier-part [ "?" query ]
   
   URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]

   hier-part     = "//" authority path-abempty
                 / path-absolute
                 / path-rootless
                 / path-empty

   scheme        = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )

   authority     = [ userinfo "@" ] host [ ":" port ]
   userinfo      = *( unreserved / pct-encoded / sub-delims / ":" )
   host          = IP-literal / IPv4address / reg-name
   port          = *DIGIT

   IP-literal    = "[" ( IPv6address / IPvFuture  ) "]"

   IPvFuture     = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )

   IPv6address   =                            6( h16 ":" ) ls32
                 /                       "::" 5( h16 ":" ) ls32
                 / [               h16 ] "::" 4( h16 ":" ) ls32
                 / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
                 / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
                 / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
                 / [ *4( h16 ":" ) h16 ] "::"              ls32
                 / [ *5( h16 ":" ) h16 ] "::"              h16
                 / [ *6( h16 ":" ) h16 ] "::"

   h16           = 1*4HEXDIG
   ls32          = ( h16 ":" h16 ) / IPv4address
   IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet

   dec-octet     = DIGIT                 ; 0-9
                 / %x31-39 DIGIT         ; 10-99
                 / "1" 2DIGIT            ; 100-199
                 / "2" %x30-34 DIGIT     ; 200-249
                 / "25" %x30-35          ; 250-255

   reg-name      = *( unreserved / pct-encoded / sub-delims )

   path          = path-abempty    ; begins with "/" or is empty
                 / path-absolute   ; begins with "/" but not "//"
                 / path-noscheme   ; begins with a non-colon segment
                 / path-rootless   ; begins with a segment
                 / path-empty      ; zero characters

   path-abempty  = *( "/" segment )
   path-absolute = "/" [ segment-nz *( "/" segment ) ]
   path-noscheme = segment-nz-nc *( "/" segment )
   path-rootless = segment-nz *( "/" segment )
   path-empty    = 0<pchar>

   segment       = *pchar
   segment-nz    = 1*pchar
   segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
                 ; non-zero-length segment without any colon ":"

   pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"

   query         = *( pchar / "/" / "?" )

   fragment      = *( pchar / "/" / "?" )

   pct-encoded   = "%" HEXDIG HEXDIG

   unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
   reserved      = gen-delims / sub-delims
   gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
   sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                 / "*" / "+" / "," / ";" / "="
*/
