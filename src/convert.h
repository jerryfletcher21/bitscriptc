#ifndef CONVERT_H
#define CONVERT_H

int
convert_bip39(const int testnet);

int
xprv_to_xpub(const char *const path);

int
extended_key_check(void);

#endif /* CONVERT_H */
