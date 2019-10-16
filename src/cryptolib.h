/*
  Copyright 2019 SoloKeys Developers

  Licensed under the Apache License, Version 2.0, <LICENSE-APACHE or
  http://apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT or
  http://opensource.org/licenses/MIT>, at your option. This file may not be
  copied, modified, or distributed except according to those terms.
 */

#ifndef SRC_CRYPTOLIB_H_
#define SRC_CRYPTOLIB_H_

#include <util.h>
#include <cstddef>
#include <cstdint>
#include <errors.h>
#include "tlv.h"

#include <mbedtls/config.h>
#include <mbedtls/rsa.h>
#include <mbedtls/aes.h>
#include <mbedtls/havege.h>
#include <mbedtls/ecdsa.h>

namespace Crypto {

// OpenPGP 3.3.1 page 31. RFC 4880 and 6637
enum AlgoritmID {
	None                  = 0x00,
	RSA                   = 0x01,
	ECDSAforCDSandIntAuth = 0x13,
	ECDHforDEC            = 0x12
};

// OpenPGP 3.3.1 page 31.
enum RSAKeyImportFormat {
	StandardEPQ  = 0x00, // EPQ
	StandardEPQN = 0x01,
	Crt          = 0x02, // Chinese Remainder Theorem
	CrtWithN     = 0x03
};

// ECDSA OIDs. OpenPGP 3.3.1 pages 90-92.
// decoding https://docs.microsoft.com/ru-ru/windows/win32/seccertenroll/about-object-identifier
// ansix9p256r1, OID = {1.2.840.10045.3.1.7} = ´2A8648CE3D030107´         MBEDTLS_ECP_DP_SECP256R1
// ansix9p384r1, OID = {1.3.132.0.34} = '2B81040022'                      MBEDTLS_ECP_DP_SECP384R1
// ansix9p521r1, OID = {1.3.132.0.35} = '2B81040023'                      MBEDTLS_ECP_DP_SECP521R1
// brainpoolP256r1, OID={1.3.36.3.3.2.8.1.1.7} = ´2B2403030208010107´     MBEDTLS_ECP_DP_BP256R1
// brainpoolP384r1, OID={1.3.36.3.3.2.8.1.1.11} = ´2B240303020801010B´    MBEDTLS_ECP_DP_BP384R1
// brainpoolP512r1, OID={1.3.36.3.3.2.8.1.1.13} = ´2B240303020801010D´    MBEDTLS_ECP_DP_BP512R1
// max OID length 9 bytes
enum ECDSAaid {
	none,
	ansix9p256r1,
	ansix9p384r1,
	ansix9p521r1,
	brainpoolP256r1,
	brainpoolP384r1,
	brainpoolP512r1
};

struct ECDSAalgParams {
	ECDSAaid aid;
	bstr oid;
	mbedtls_ecp_group_id mbedtlsGroup;
};

static const std::array<ECDSAalgParams, 1> ECDSAalgParamsList = {
		{none,         ""_bstr,                                 MBEDTLS_ECP_DP_NONE},
		//{ansix9p256r1, "\x2A\x86\x48\xCE\x3D\x03\x01\x07"_bstr, MBEDTLS_ECP_DP_SECP256R1}
};
/*
ECDSAaid AIDfromOID(bstr oid) {

	if (oid == "\x2A\x86\x48\xCE\x3D\x03\x01\x07"_bstr)
		return ECDSAaid::ansix9p256r1;
	if (oid == "\x2B\x81\x04\x00\x22"_bstr)
		return ECDSAaid::ansix9p384r1;
	if (oid == "\x2B\x81\x04\x00\x23"_bstr)
		return ECDSAaid::ansix9p521r1;
	if (oid == "\x2B\x24\x03\x03\x02\x08\x01\x01\x07"_bstr)
		return ECDSAaid::brainpoolP256r1;
	if (oid == "\x2B\x24\x03\x03\x02\x08\x01\x01\x0B"_bstr)
		return ECDSAaid::brainpoolP384r1;
	if (oid == "\x2B\x24\x03\x03\x02\x08\x01\x01\x0D"_bstr)
		return ECDSAaid::brainpoolP512r1;

	return ECDSAaid::none;
}
*/
enum KeyType {
	Symmetric,
	FullAsymmetric,
	Public,
	Private
};

// OpenPGP 3.3.1 page 33
enum KeyPartsRSA {
	PublicExponent = 0x91, // key format: standard and crt
	P              = 0x92, // standard and crt
	Q              = 0x93, // standard and crt
	PQ             = 0x94, // crt
	DP1            = 0x95, // crt
	DQ1            = 0x96, // crt
	N              = 0x97  // optional for standard and crt. Modulus.
};

enum KeyPartsECDSA {
	PrivateKey     = 0x92, // mandatory
	PublicKey      = 0x99  // optional
};

// OpenPGP 3.3.1 page 33
struct RSAKey {
	bstr Exp; // Public exponent: e  (key format: standard and crt)
	bstr P;   // Prime1: p           (standard and crt)
	bstr Q;   // Prime2: q           (standard and crt)
	bstr PQ;  // PQ: 1/q mod p       (crt)
	bstr DP1; // DP1: d mod (p - 1)  (crt)
	bstr DQ1; // DQ1: d mod (q - 1)  (crt)
	bstr N;   // Modulus: n          (optional for standard and crt)

	constexpr void clear(){
		Exp.set_length(0);
		P.set_length(0);
		Q.set_length(0);
		PQ.set_length(0);
		DP1.set_length(0);
		DQ1.set_length(0);
		N.set_length(0);
	}

	constexpr void Print() {
		printf("Exp [%lu] ", Exp.length()); dump_hex(Exp, 32);
		printf("P [%lu] ",   P.length());   dump_hex(P, 32);
		printf("Q [%lu] ",   Q.length());   dump_hex(Q, 32);
		printf("PQ [%lu] ",  PQ.length());  dump_hex(PQ, 32);
		printf("DP1 [%lu] ", DP1.length()); dump_hex(DP1, 32);
		printf("DQ1 [%lu] ", DQ1.length()); dump_hex(DQ1, 32);
		printf("N [%lu] ",   N.length());   dump_hex(N, 32);
	}
};

struct ECDSAKey {
	bstr Private;
	bstr Public;

	void clear(){
		Private.set_length(0);
		Public.set_length(0);
	}
	constexpr void Print() {
		printf("Public  [%lu] ", Public.length());  dump_hex(Public,  48);
		printf("Private [%lu] ", Private.length()); dump_hex(Private, 48);
	}
};

class CryptoEngine;

class CryptoLib {
private:
	CryptoEngine &cryptoEngine;

	uint8_t _KeyBuffer[2049]; // needs for placing RSA 4096 key
	bstr KeyBuffer{_KeyBuffer, 0, sizeof(_KeyBuffer)};

	Util::Error RSAFillPrivateKey(mbedtls_rsa_context *context, RSAKey key);
	Util::Error AppendKeyPart(bstr &buffer, bstr &keypart, mbedtls_mpi *mpi);
	Util::Error AppendKeyPartEcpPoint(bstr &buffer, bstr &keypart,  mbedtls_ecp_group *grp, mbedtls_ecp_point  *point);
public:
	CryptoLib(CryptoEngine &_cryptoEngine): cryptoEngine(_cryptoEngine) {
		ClearKeyBuffer();
	};

	void ClearKeyBuffer();

	Util::Error GenerateRandom(size_t length, bstr &dataOut);

	Util::Error AESEncrypt(bstr key, bstr dataIn, bstr &dataOut);
	Util::Error AESDecrypt(bstr key, bstr dataIn, bstr &dataOut);

	Util::Error RSAGenKey(RSAKey &keyOut, size_t keySize);
	Util::Error RSACalcPublicKey(bstr strP, bstr strQ, bstr &strN);
	Util::Error RSASign(RSAKey key, bstr data, bstr &signature);
	Util::Error RSADecipher(RSAKey key, bstr data, bstr &dataOut);
	Util::Error RSAVerify(bstr publicKey, bstr data, bstr signature);

	Util::Error ECDSAGenKey(ECDSAKey &keyOut);
	Util::Error ECDSACalcPublicKey(bstr privateKey, bstr &publicKey);
	Util::Error ECDSASign(ECDSAKey key, bstr data, bstr &signature);
	Util::Error ECDSAVerify(ECDSAKey key, bstr data, bstr signature);
};

class KeyStorage {
private:
	CryptoEngine &cryptoEngine;

	uint8_t prvData[2049] = {0}; // needs for placing RSA 4096 key
	bstr prvStr{prvData, 0, sizeof(prvData)};
public:
	KeyStorage(CryptoEngine &_cryptoEngine): cryptoEngine(_cryptoEngine) {prvStr.clear();};

	bool KeyExists(AppID_t appID, KeyID_t keyID);

	Util::Error GetKeyPart(bstr data, Util::tag_t keyPart, bstr &dataOut);
	Util::Error GetPublicKey(AppID_t appID, KeyID_t keyID, uint8_t AlgoritmID, bstr &pubKey);
	Util::Error GetPublicKey7F49(AppID_t appID, KeyID_t keyID, uint8_t AlgoritmID, bstr &tlvKey);

	Util::Error GetRSAKey(AppID_t appID, KeyID_t keyID, RSAKey &key);
	Util::Error GetECDSAKey(AppID_t appID, KeyID_t keyID, ECDSAKey &key);
	Util::Error PutRSAFullKey(AppID_t appID, KeyID_t keyID, RSAKey key);
	Util::Error PutECDSAFullKey(AppID_t appID, KeyID_t keyID, ECDSAKey key);

	Util::Error SetKey(AppID_t appID, KeyID_t keyID, KeyType keyType, bstr key);
	Util::Error SetKeyExtHeader(AppID_t appID, bstr keyData);
};

class CryptoEngine {
private:
	CryptoLib cryptoLib{*this};
	KeyStorage keyStorage{*this};
public:
	Util::Error AESEncrypt(AppID_t appID, KeyID_t keyID, bstr dataIn, bstr &dataOut);
	Util::Error AESDecrypt(AppID_t appID, KeyID_t keyID, bstr dataIn, bstr &dataOut);

	Util::Error RSASign(AppID_t appID, KeyID_t keyID, bstr data, bstr &signature);
	Util::Error RSADecipher(AppID_t appID, KeyID_t keyID, bstr data, bstr &dataOut);
	Util::Error RSAVerify(AppID_t appID, KeyID_t keyID, bstr data, bstr signature);

	Util::Error ECDSASign(AppID_t appID, KeyID_t keyID, bstr data, bstr &signature);
	Util::Error ECDSAVerify(AppID_t appID, KeyID_t keyID, bstr data, bstr signature);

	CryptoLib &getCryptoLib() {
		return cryptoLib;
	}

	KeyStorage &getKeyStorage() {
		return keyStorage;
	}

};

} // namespace Crypto

#endif /* SRC_CRYPTOLIB_H_ */
