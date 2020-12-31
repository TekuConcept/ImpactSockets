An implementation is considered conformant if it complies with all of the
requirements associated with the roles it partakes in HTTP.

__MUST__

Section 2
- When a received protocol element is parsed, the recipient MUST be able to
  parse any value of reasonable length that is applicable to the recipient's
  role and that matches the grammar defined by the corresponding ABNF rules.
- At a minimum, a recipient MUST be able to parse and process protocol element
  lengths that are at least as long as the values that it generates for those
  same protocol elements in other messages.
- A recipient MUST interpret a received protocol element according to the
  semantics defined for it by this specification, including extensions to this
  specification, unless the recipient has determined (through experience or
  configuration) that the sender incorrectly implements what is implied by those
  semantics.
- Intermediaries that process HTTP messages (i.e., all intermediaries other than
  those acting as tunnels) MUST send their own HTTP-version in forwarded
  messages.
- (*) A recipient that processes a URI reference with an empty host identifier
  MUST reject it as invalid.
- With the HTTPS scheme, the user agent MUST ensure that its connection to the
  origin server is secured through the use of strong encryption, end-to-end,
  prior to sending the first HTTP request.

Section 3
- A recipient MUST parse an HTTP message as a sequence of octets in an encoding
  that is a superset of US-ASCII [USASCII].
- A recipient that receives whitespace between the start-line and the first
  header field MUST either reject the message as invalid or consume each
  whitespace-preceded line without further processing of it (i.e., ignore the
  entire line, along with any subsequent lines preceded by whitespace, until a
  properly formed header field is received or the header section is terminated).
- A server that receives a request-target longer than any URI it wishes to parse
  MUST respond with a 414 (URI Too Long) status code.
- A proxy MUST forward unrecognized header fields unless the field-name is
  listed in the Connection header field (Section 6.1) or the proxy is
  specifically configured to block, or otherwise transform, such fields.
- A recipient MUST parse for bad whitespace (BWS) and remove it before
  interpreting the protocol element.
- A server MUST reject any received request message that contains whitespace
  between a header field-name and colon with a response code of 400
  (Bad Request).
- A proxy MUST remove any such whitespace from a response message before
  forwarding the message downstream.
- A server that receives an obs-fold in a request message that is not within a
  message/http container MUST either reject the message by sending a 400 (Bad
  Request), preferably with a representation explaining that obsolete line
  folding is unacceptable, or replace each received obs-fold with one or more SP
  octets prior to interpreting the field value or forwarding the message
  downstream.
- A proxy or gateway that receives an obs-fold in a response message that is not
  within a message/http container MUST either discard the message and replace it
  with a 502 (Bad Gateway) response, preferably with a representation explaining
  that unacceptable line folding was received, or replace each received obs-fold
  with one or more SP octets prior to interpreting the field value or forwarding
  the message downstream.
- A user agent that receives an obs-fold in a response message that is not
  within a message/http container MUST replace each received obs-fold with one
  or more SP octets prior to interpreting the field value.
- A server that receives a request header field, or set of fields, larger than
  it wishes to process MUST respond with an appropriate 4xx (Client Error)
  status code.
- Recipients that process the value of a quoted-string MUST handle a quoted-pair
  as if it were replaced by the octet following the backslash.
- A recipient MUST be able to parse the chunked transfer coding (Section 4.1)
  because it plays a crucial role in framing messages when the payload body size
  is not known in advance.
- If any transfer coding other than chunked is applied to a request payload
  body, the sender MUST apply chunked as the final transfer coding to ensure
  that the message is properly framed.
- If any transfer coding other than chunked is applied to a response payload
  body, the sender MUST either apply chunked as the final transfer coding or
  terminate the message by closing the connection.
- Since there is no predefined limit to the length of a payload, a recipient
  MUST anticipate potentially large decimal numerals and prevent parsing errors
  due to integer conversion overflows (Section 9.3).
- If a message is received that has multiple Content-Length header fields with
  field-values consisting of the same decimal value, or a single Content-Length
  header field with a field value containing a list of identical decimal values
  (e.g., "Content-Length: 42, 42"), indicating that duplicate Content-Length
  header fields have been generated or combined by an upstream message
  processor, then the recipient MUST either reject the message as invalid or
  replace the duplicated field-values with a single valid Content-Length field
  containing that decimal value prior to determining the message body length or
  forwarding the message.
- If a Transfer-Encoding header field is present in a request and the chunked
  transfer coding is not the final encoding, the message body length cannot be
  determined reliably; the server MUST respond with the 400 (Bad Request) status
  code and then close the connection.
- A sender MUST remove the received Content-Length field prior to forwarding
  such a message downstream.
- If a message is received without Transfer-Encoding and with either multiple
  Content-Length header fields having differing field-values or a single
  Content-Length header field having an invalid value, then the message framing
  is invalid and the recipient MUST treat it as an unrecoverable error.
- If this is a request message, the server MUST respond with a 400 (Bad Request)
  status code and then close the connection.  If this is a response message
  received by a proxy, the proxy MUST close the connection to the server,
  discard the received response, and send a 502 (Bad Gateway) response to the
  client.
- If this is a response message received by a user agent, the user agent MUST
  close the connection to the server and discard the received response.
- If the sender closes the connection or the recipient times out before the
  indicated number of octets are received, the recipient MUST consider the
  message to be incomplete and close the connection.
- A user agent that sends a request containing a message body MUST send a valid
  Content-Length header field if it does not know the server will handle
  HTTP/1.1 (or later) requests; such knowledge can be in the form of specific
  user configuration or by remembering the version of a prior received response.
- A client that receives an incomplete response message, which can occur when a
  connection is closed prematurely or when decoding a supposedly chunked
  transfer coding fails, MUST record the message as incomplete.
- If terminating the request message body with a line-ending is desired, then
  the user agent MUST count the terminating CRLF octets as part of the message
  body length.

Section 4
- A recipient MUST be able to parse and decode the chunked transfer coding.
- A recipient MUST ignore unrecognized chunk extensions.
- A recipient MUST ignore (or consider as an error) any fields that are
  forbidden to be sent in a trailer, since processing them as if they were
  present in the header section might bypass external security filters.
- Since the TE header field only applies to the immediate connection, a sender
  of TE MUST also send a "TE" connection option within the Connection header
  field (Section 6.1) in order to prevent the TE field from being forwarded by
  intermediaries that do not support its semantics.


__MUST NOT__

Section 2
- A server MUST NOT assume that two requests on the same connection are from the
  same user agent unless the connection is secured and specific to that agent.
- A sender MUST NOT generate protocol elements that convey a meaning that is
  known by that sender to be false.
- A sender MUST NOT generate protocol elements that do not match the grammar
  defined by the corresponding ABNF rules.
- Within a given message, a sender MUST NOT generate protocol elements or syntax
  alternatives that are only allowed to be generated by participants in other
  roles (i.e., a role that the sender does not have for that message).
- A client MUST NOT send a version to which it is not conformant.
- A server MUST NOT send a version to which it is not conformant.
- (*) A sender MUST NOT generate an "http" URI with an empty host identifier.
- (*) A sender MUST NOT generate the userinfo subcomponent (and its "@"
  delimiter) when an "http" URI reference is generated within a message as a
  request target or header field value.

Section 3
- A sender MUST NOT send whitespace between the start-line and the first header
  field.
- A server MUST NOT apply a request to the target resource until the entire
  request header section is received, since later header fields might include
  conditionals, authentication credentials, or deliberately misleading duplicate
  header fields that would impact request processing.
- A sender MUST NOT generate multiple header fields with the same field name in
  a message unless either the entire field value for that header field is
  defined as a comma-separated list [i.e., #(values)] or the header field is a
  well-known exception (ie "Set-Cookie").
- The order in which header fields with the same field name are received is
  therefore significant to the interpretation of the combined field value; a
  proxy MUST NOT change the order of these field values when forwarding a
  message.
- A sender MUST NOT generate BWS in messages.
- A sender MUST NOT generate a message that includes line folding (i.e., that
  has any field-value that contains a match to the obs-fold rule) unless the
  message is intended for packaging within the message/http media type.
- A sender MUST NOT apply chunked more than once to a message body (i.e.,
  chunking an already chunked message is not allowed).
- A server MUST NOT send a Transfer-Encoding header field in any response with a
  status code of 1xx (Informational) or 204 (No Content).  A server MUST NOT
  send a Transfer-Encoding header field in any 2xx (Successful) response to a
  CONNECT request (Section 4.3.6 of [RFC7231]).
- A client MUST NOT send a request containing Transfer-Encoding unless it knows
  the server will handle HTTP/1.1 (or later) requests; such knowledge might be
  in the form of specific user configuration or by remembering the version of a
  prior received response.
- A server MUST NOT send a response containing Transfer-Encoding unless the
  corresponding request indicates HTTP/1.1 (or later).
- A sender MUST NOT send a Content-Length header field in any message that
  contains a Transfer-Encoding header field.
- A server MUST NOT send Content-Length in such a response unless its
  field-value equals the decimal number of octets that would have been sent in
  the payload body of a response if the same request had used the GET method.
- A server MUST NOT send Content-Length in such a response unless its
  field-value equals the decimal number of octets that would have been sent in
  the payload body of a 200 (OK) response to the same request.
- A server MUST NOT send a Content-Length header field in any response with a
  status code of 1xx (Informational) or 204 (No Content).
- A server MUST NOT send a Content-Length header field in any 2xx (Successful)
  response to a CONNECT request (Section 4.3.6 of [RFC7231]).
- A client MUST NOT process, cache, or forward such extra data as a separate
  response, since such behavior would be vulnerable to cache poisoning.
- An HTTP/1.1 user agent MUST NOT preface or follow a request with an extra
  CRLF.

Section 4
- A sender MUST NOT generate a trailer that contains a field necessary for
  message framing (e.g., Transfer-Encoding and Content-Length), routing (e.g.,
  Host), request modifiers (e.g., controls and conditionals in Section 5 of
  [RFC7231]), authentication (e.g., see [RFC7235] and [RFC6265]), response
  control data (e.g., see Section 7.1 of [RFC7231]), or determining how to
  process the payload (e.g., Content-Encoding, Content-Type, Content-Range, and
  Trailer).
- A client MUST NOT send the chunked transfer coding name in TE; chunked is
  always acceptable for HTTP/1.1 recipients.


__REQUIRED__

__SHALL__

__SHALL NOT__

__SHOULD__

Section 2
- A client SHOULD send a request version equal to the highest version to which
  the client is conformant and whose major version is no higher than the highest
  version supported by the server, if this is known.
- A server SHOULD send a response version equal to the highest version to which
  the server is conformant that has a major version less than or equal to the
  one received in the request.
- When an HTTP message is received with a major version number that the
  recipient implements, but a higher minor version number than what the
  recipient implements, the recipient SHOULD process the message as if it were
  in the highest minor version within that major version to which the recipient
  is conformant.
- Before making use of an "http" URI reference received from an untrusted
  source, a recipient SHOULD parse for userinfo and treat its presence as an
  error.

Section 3
- Recipients of an invalid request-line SHOULD respond with either a 400 (Bad
  Request) error or a 301 (Moved Permanently) redirect with the request-target
  properly encoded.
- A server that receives a method longer than any that it implements SHOULD
  respond with a 501 (Not Implemented) status code.
- A client SHOULD ignore the reason-phrase content.
- Other recipients SHOULD ignore unrecognized header fields.
- For protocol elements where optional whitespace is preferred to improve
  readability, a sender SHOULD generate the optional whitespace as a single SP.
- A sender SHOULD generate RWS as a single SP.
- Newly defined header fields SHOULD limit their field values to US-ASCII
  octets.
- A recipient SHOULD treat other octets in field content (obs-text) as opaque
  data.
- A user agent SHOULD send a Content-Length in a request message when no
  Transfer-Encoding is sent and the request method defines a meaning for an
  enclosed payload body.
- Aside from the cases defined above, in the absence of Transfer-Encoding, an
  origin server SHOULD send a Content-Length header field when the payload body
  size is known prior to sending the complete header section.
- Since there is no way to distinguish a successfully completed, close-delimited
  message from a partially received message interrupted by network failure, a
  server SHOULD generate encoding or length-delimited messages whenever
  possible.
- Unless a transfer coding other than chunked has been applied, a client that
  sends a request containing a message body SHOULD use a valid Content-Length
  header field if the message body length is known in advance, rather than the
  chunked transfer coding, since some existing services respond to chunked with
  a 411 (Length Required) status code even though they understand the chunked
  transfer coding.
- In the interest of robustness, a server that is expecting to receive and parse
  a request-line SHOULD ignore at least one empty line (CRLF) received prior to
  the request-line.
- When a server listening only for HTTP request messages, or processing what
  appears from the start-line to be an HTTP request message, receives a sequence
  of octets that does not match the HTTP-message grammar aside from the
  robustness exceptions listed above, the server SHOULD respond with a 400 (Bad
  Request) response.

Section 4
- A recipient SHOULD consider "x-compress" to be equivalent to "compress".
- A recipient SHOULD consider "x-gzip" to be equivalent to "gzip".
- When a message includes a message body encoded with the chunked transfer
  coding and the sender desires to send metadata in the form of trailer fields
  at the end of the message, the sender SHOULD generate a Trailer header field
  before the message body to indicate which fields will be present in the
  trailers.


__SHOULD NOT__

Section 2
- HTTP protocol downgrades SHOULD NOT be performed unless triggered by specific
  client attributes, such as when one or more of the request header fields
  (e.g., User-Agent) uniquely match the values sent by a client known to be in
  error.

Section 3
- A recipient SHOULD NOT attempt to autocorrect and then process the request
  without a redirect, since the invalid request-line might be deliberately
  crafted to bypass security filters along the request chain.
- A sender SHOULD NOT generate optional whitespace except as needed to white out
  invalid or unwanted protocol elements during in-place message filtering.
- A sender SHOULD NOT generate a quoted-pair in a quoted-string except where
  necessary to quote DQUOTE and backslash octets occurring within that string.
- A sender SHOULD NOT generate a quoted-pair in a comment except where necessary
  to quote parentheses ["(" and ")"] and backslash octets occurring within that
  comment.
- A server that receives a request message with a transfer coding it does not
  understand SHOULD respond with 501 (Not Implemented).
- A user agent SHOULD NOT send a Content-Length header field when the request
  message does not contain a payload body and the method semantics do not
  anticipate such a body.

Section 4
- Unless the request includes a TE header field indicating "trailers" is
  acceptable, as described in Section 4.3, a server SHOULD NOT generate trailer
  fields that it believes are necessary for the user agent to receive.


__RECOMMENDED__

Section 3
- It is RECOMMENDED that all HTTP senders and recipients support, at a minimum,
  request-line lengths of 8000 octets.

__MAY__

Section 2
- Unless noted otherwise, a recipient MAY attempt to recover a usable protocol
  element from an invalid construct.
- A client MAY send a lower request version if it is known that the server
  incorrectly implements the HTTP specification, but only after the client has
  attempted at least one normal request and determined from the response status
  code or header fields (e.g., Server) that the server improperly handles higher
  request versions.
- A server MAY send an HTTP/1.0 response to a request if it is known or
  suspected that the client incorrectly implements the HTTP specification and is
  incapable of correctly processing later version responses, such as when a
  client fails to parse the version number correctly or when an intermediary is
  known to blindly forward the HTTP-version even when it doesn't conform to the
  given minor version of the protocol.
- When an "http" URI is used within a context that calls for access to the
  indicated resource, a client MAY attempt access by resolving the host to an IP
  address, establishing a TCP connection to that address on the indicated port,
  and sending an HTTP request message containing the URI's identifying data to
  the server.

Section 3
- A recipient MAY combine multiple header fields with the same field name into
  one "field-name: field-value" pair, without changing the semantics of the
  message, by appending each subsequent field value to the combined field value
  in order, separated by a comma.
- A client MAY discard or truncate received header fields that are larger than
  the client wishes to process if the field semantics are such that the dropped
  value(s) can be safely ignored without changing the message framing or
  response semantics.
- Transfer-Encoding is a property of the message, not of the representation, and
  any recipient along the request/response chain MAY decode the received
  transfer coding(s) or apply additional transfer coding(s) to the message body,
  assuming that corresponding changes are made to the Transfer-Encoding
  field-value.
- Transfer-Encoding MAY be sent in a response to a HEAD request or in a 304 (Not
  Modified) response (Section 4.1 of [RFC7232]) to a GET request, neither of
  which includes a message body, to indicate that the origin server would have
  applied a transfer coding to the message body if the request had been an
  unconditional GET.
- A server MAY send a Content-Length header field in a response to a HEAD
  request (Section 4.3.2 of [RFC7231]);
- A server MAY send a Content-Length header field in a 304 (Not Modified)
  response to a conditional GET request (Section 4.1 of [RFC7232]);
- A server MAY reject a request that contains a message body but not a
  Content-Length by responding with 411 (Length Required).
- If the final response to the last request on a connection has been completely
  received and there remains additional data to read, a user agent MAY discard
  the remaining data or attempt to determine if that data belongs as part of the
  prior response body, which might be the case if the prior message's
  Content-Length value is incorrect.
- A server that receives an incomplete request message, usually due to a
  canceled request or a triggered timeout exception, MAY send an error response
  prior to closing the connection.
- Although the line terminator for the start-line and header fields is the
  sequence CRLF, a recipient MAY recognize a single LF as a line terminator and
  ignore any preceding CR.
- Although the request-line and status-line grammar rules require that each of
  the component elements be separated by a single SP octet, recipients MAY
  instead parse on whitespace-delimited word boundaries and, aside from the CRLF
  terminator, treat any form of whitespace as the SP separator while ignoring
  preceding or trailing whitespace; such whitespace includes one or more of the
  following octets: SP, HTAB, VT (%x0B), FF (%x0C), or bare CR.

Section 4
- When a chunked message containing a non-empty trailer is received, the
  recipient MAY process the fields (aside from those forbidden above) as if they
  were appended to the message's header section.
- When multiple transfer codings are acceptable, the client MAY rank the codings
  by preference using a case-insensitive "q" parameter (similar to the qvalues
  used in content negotiation fields, Section 5.3.1 of [RFC7231]).


__OPTIONAL__

Section 4
- The chunked transfer coding wraps the payload body in order to transfer it as
  a series of chunks, each with its own size indicator, followed by an OPTIONAL
  trailer containing header fields.


__concept__
- - - - - - - - - - - - - -
> http_socket:<br>
>> [basic_socket]<br>
>> send (http_message)<br>
>> send_status (http_code)<br>
>> recv (http_message)
- - - - - - - - - - - - - -

_Types of http nodes:_ proxy, gateway, tunnel, server, client

An empty URI path component is equivalent to an absolute path of "/", so the
normal form is to provide a path of "/" instead.

(*) Messages are parsed using a generic algorithm, independent of the individual
header field names. The contents within a given field value are not parsed until
a later stage of message interpretation (usually after the message's entire
header section has been processed).
(*) The field value does not include any leading or trailing whitespace: OWS
occurring before the first non-whitespace octet of the field value or after the
last non-whitespace octet of the field value ought to be excluded by parsers
when extracting the field value from a header field.
(*) Comments are only allowed in fields containing "comment" as part of their
field value definition.
(*) The presence of a message body in a request is signaled by a Content-Length
or Transfer-Encoding header field. The presence of a message body in a response
depends on both the request method to which it is responding and the response
status code. (Section 3.3)
(*) Section 3.3.3


__ABNF__
```
> HTTP-version  = HTTP-name "/" DIGIT "." DIGIT
> HTTP-name     = %x48.54.54.50 ; "HTTP", case-sensitive
> http-URI      = "http:" "//" authority path-abempty
                  [ "?" query ] [ "#" fragment ]
> https-URI     = "https:" "//" authority path-abempty
                  [ "?" query ] [ "#" fragment ]

> absolute-path = 1*( "/" segment )
> partial-URI   = relative-part [ "?" query ]

> OWS           = *( SP / HTAB )  ; optional whitespace
> RWS           = 1*( SP / HTAB ) ; required whitespace
> BWS           = OWS             ; "bad" whitespace

> token         = 1*tchar
> tchar         = "!" / "#" / "$" / "%" / "&" / "'" / "*"
                  / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
                  / DIGIT / ALPHA
                  ; any VCHAR, except delimiters
> quoted-string = DQUOTE *( qdtext / quoted-pair ) DQUOTE
> qdtext        = HTAB / SP /%x21 / %x23-5B / %x5D-7E / obs-text
> obs-text      = %x80-FF
> comment       = "(" *( ctext / quoted-pair / comment ) ")"
> ctext         = HTAB / SP / %x21-27 / %x2A-5B / %x5D-7E / obs-text
> quoted-pair   = "\" ( HTAB / SP / VCHAR / obs-text )

> HTTP-message  = start-line
                  *( header-field CRLF )
                  CRLF
                  [ message-body ]

> start-line    = request-line / status-line
> request-line  = method SP request-target SP HTTP-version CRLF
> method        = token ; case-sensitive
> status-line   = HTTP-version SP status-code SP reason-phrase CRLF
> status-code   = 3DIGIT
> reason-phrase = *( HTAB / SP / VCHAR / obs-text )

> header-field  = field-name ":" OWS field-value OWS
> field-name    = token ; case-insensitive
> field-value   = *( field-content / obs-fold )
> field-content = field-vchar [ 1*( SP / HTAB ) field-vchar ]
> field-vchar   = VCHAR / obs-text
> obs-fold      = CRLF 1*( SP / HTAB )
                  ; obsolete line folding
                  ; see Section 3.2.4

> message-body      = *OCTET
> Transfer-Encoding = 1#transfer-coding
> Content-Length    = 1*DIGIT

> chunked-body   = *chunk
                   last-chunk
                   trailer-part
                   CRLF
> chunk          = chunk-size [ chunk-ext ] CRLF
                   chunk-data CRLF
> chunk-size     = 1*HEXDIG
> last-chunk     = 1*("0") [ chunk-ext ] CRLF
> chunk-data     = 1*OCTET ; a sequence of chunk-size octets
> chunk-ext      = *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
> chunk-ext-name = token
> chunk-ext-val  = token / quoted-string
> trailer-part   = *( header-field CRLF )

> TE        = #t-codings
> t-codings = "trailers" / ( transfer-coding [ t-ranking ] )
> t-ranking = OWS ";" OWS "q=" rank
> rank      = ( "0" [ "." 0*3DIGIT ] )
              / ( "1" [ "." 0*3("0") ] )
> Trailer   = 1#field-name
```
