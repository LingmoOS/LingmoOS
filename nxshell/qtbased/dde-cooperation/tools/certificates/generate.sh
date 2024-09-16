#!/bin/bash

# Certificate Authority (CA)
openssl genrsa -passout pass:uos123456 -out ca-secret.key 1024
openssl rsa -passin pass:uos123456 -in ca-secret.key -out ca.key
openssl req -new -x509 -days 3650 -subj '/C=BY/ST=Belarus/L=Minsk/O=Union root CA/OU=Union CA unit/CN=Union.com' -key ca.key -out ca.crt
openssl pkcs12 -export -passout pass:uos123456 -inkey ca.key -in ca.crt -out ca.pfx
openssl pkcs12 -passin pass:uos123456 -passout pass:uos123456 -in ca.pfx -out ca.pem

# SSL Server certificate
openssl genrsa -passout pass:uos123456 -out server-secret.key 1024
openssl rsa -passin pass:uos123456 -in server-secret.key -out server.key
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Union server/OU=Union server unit/CN=server.dde-cooperation.com' -key server.key -out server.csr
openssl x509 -req -days 3650 -in server.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out server.crt
openssl pkcs12 -export -passout pass:uos123456 -inkey server.key -in server.crt -out server.pfx
openssl pkcs12 -passin pass:uos123456 -passout pass:uos123456 -in server.pfx -out server.pem

# SSL Client certificate
openssl genrsa -passout pass:uos123456 -out client-secret.key 1024
openssl rsa -passin pass:uos123456 -in client-secret.key -out client.key
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Union client/OU=Union client unit/CN=client.dde-cooperation.com' -key client.key -out client.csr
openssl x509 -req -days 3650 -in client.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out client.crt
openssl pkcs12 -export -passout pass:uos123456 -inkey client.key -in client.crt -out client.pfx
openssl pkcs12 -passin pass:uos123456 -passout pass:uos123456 -in client.pfx -out client.pem

# Diffie�Hellman (D-H) key exchange
openssl dhparam -out dh1024.pem 1024
