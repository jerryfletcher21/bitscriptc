# Bitscriptc

Some scripts written c using
[libwally](https://github.com/ElementsProject/libwally-core) to be used
with bitcoin core. Everything is compiled locally and linked statically,
so there are no external dependecies apart from the c standard library.
Bitscriptc is just a simple wrapper around libwally.

Main scripts are for generating a bip39 and converting it to xprv and
xpub, and for signing a message with the private key of an address.

The idea for the generations scripts is to have the most simple and with
less dependencies way to generate a bip39 mnemonics/bip32 xpub, xprv.

The idea for the signing scripts is that bitcoin core's `signmessage`
and `signmessagewithprivkey` do not support
[bip-0322](https://github.com/bitcoin/bips/blob/master/bip-0322.mediawiki),
so they only work with legacy addresses (see
[here](https://github.com/bitcoin/bitcoin/pull/24058)).
The scripts help with signing from any addresses with bitcoin core in a
minimal way and with less dependencies possible.

## Build and installation

```
./configure
make

# if you also want to install
make install
```

## Usage

Generation
```
bitscriptc generate-bip39 > mnemonics
cat mnemonics | bitscriptc xprv-from-bip39 > xprv
cat xprv | bitscriptc xpub-from-xprv > xpub
```

Signing message
```
bitcoin-cli -rpcwallet="<wallet>" -named listdescriptors private=true |
jq -r '.descriptors[].desc' |
head -n 1 |
rev | cut -d "(" -f 1 | rev |
cut -d ")" -f 1 |
bitscriptc sign-extended-key "<address>" ["<message>"]
```

## License

Bitscriptc is released under the terms of the ISC license.
See [LICENSE](LICENSE) for more details.
