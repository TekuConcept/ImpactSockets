Design Notes:

Input message parsing

```
HTTP-message   = start-line
                 *( header-field CRLF )
                 CRLF
                 [ message-body ]
```

- line parser
- message parsing state
