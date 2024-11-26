# Bunker

Bunker is a chat room with end-to-end encryption.

## Variant 1

When a client joins a room:

- generate RSA keys
- input nickname

The client sends the nickname along with the public key to the server.

The other clients get the nickname and public key of the newly connected client.


When a client sends a message:

- generate AES key
- encrypt message using AES key

For every other client, that the client knows of:
- encrypt AES key using their public RSA key

Send the following data to the server:
- the encrypted AES message
- the RSA encrypted AES key for every other client

bob, og43y8hn43n30hn
noa, 3049nh302nh3b3b

The other clients will receive the following data:
- aes encrypted message
key: og43y8hn43n30hn

The client will then have to decrypt the key to then decrypt the message

## Variant 2

All the clients share a common secret key (AES, or new thing: AEAD scheme)

They share this key with each other using Diffi-Hellman key exchange.

## Interface

The client application will be a simple text based window using ncurses

## Techniques

- ncurses
- AES
- RSA
- sha256
- threads
- socket
- base64
