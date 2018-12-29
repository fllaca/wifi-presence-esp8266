# IFTTT Connection Trigger

## Check IFTTT certificate fingerprint

```
openssl s_client -connect maker.ifttt.com:443 | openssl x509 -fingerprint -noout
```

## Upload to ESP8266

```
# Upload static htmls
pio run -t uploadfs
# Upload code
pio run && pio run -t upload && pio device monitor
```

