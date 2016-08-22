/**
 * Authors.....: Jens Steube <jens.steube@gmail.com>
 *               Gabriele Gristina <matrix@hashcat.net>
 *               magnum <john.magnum@hushmail.com>
 *
 * License.....: MIT
 */

#ifdef __APPLE__
#include <stdio.h>
#endif

#ifdef __FreeBSD__
#include <stdio.h>
#include <pthread_np.h>
#endif

#include <shared.h>
#include <limits.h>

 /**
  * ciphers for use on cpu
  */

#include "cpu-des.c"
#include "cpu-aes.c"

  /**
   * hashes for use on cpu
   */

#include "cpu-md5.c"
#include "cpu-sha1.c"
#include "cpu-sha256.c"

   /**
    * logging
    */


#include <logging.h>

#include <converter.h>

    /**
     * decoder
     */

static void AES128_decrypt_cbc(const u32 key[4], const u32 iv[4], const u32 in[16], u32 out[16])
{
  AES_KEY skey;

  AES_set_decrypt_key((const u8 *)key, 128, &skey);

  u32 _iv[4] = { 0 };

  _iv[0] = iv[0];
  _iv[1] = iv[1];
  _iv[2] = iv[2];
  _iv[3] = iv[3];

  for (int i = 0; i < 16; i += 4)
  {
    u32 _in[4] = { 0 };
    u32 _out[4] = { 0 };

    _in[0] = in[i + 0];
    _in[1] = in[i + 1];
    _in[2] = in[i + 2];
    _in[3] = in[i + 3];

    AES_decrypt(&skey, (const u8 *)_in, (u8 *)_out);

    _out[0] ^= _iv[0];
    _out[1] ^= _iv[1];
    _out[2] ^= _iv[2];
    _out[3] ^= _iv[3];

    out[i + 0] = _out[0];
    out[i + 1] = _out[1];
    out[i + 2] = _out[2];
    out[i + 3] = _out[3];

    _iv[0] = _in[0];
    _iv[1] = _in[1];
    _iv[2] = _in[2];
    _iv[3] = _in[3];
  }
}

static void juniper_decrypt_hash(char *in, char *out)
{
  // base64 decode

  u8 base64_buf[100] = { 0 };

  base64_decode(base64_to_int, (const u8 *)in, DISPLAY_LEN_MIN_501, base64_buf);

  // iv stuff

  u32 juniper_iv[4] = { 0 };

  memcpy(juniper_iv, base64_buf, 12);

  memcpy(out, juniper_iv, 12);

  // reversed key

  u32 juniper_key[4] = { 0 };

  juniper_key[0] = byte_swap_32(0xa6707a7e);
  juniper_key[1] = byte_swap_32(0x8df91059);
  juniper_key[2] = byte_swap_32(0xdea70ae5);
  juniper_key[3] = byte_swap_32(0x2f9c2442);

  // AES decrypt

  u32 *in_ptr = (u32 *)(base64_buf + 12);
  u32 *out_ptr = (u32 *)(out + 12);

  AES128_decrypt_cbc(juniper_key, juniper_iv, in_ptr, out_ptr);
}

void phpass_decode(u8 digest[16], u8 buf[22])
{
  int l;

  l = itoa64_to_int(buf[0]) << 0;
  l |= itoa64_to_int(buf[1]) << 6;
  l |= itoa64_to_int(buf[2]) << 12;
  l |= itoa64_to_int(buf[3]) << 18;

  digest[0] = (l >> 0) & 0xff;
  digest[1] = (l >> 8) & 0xff;
  digest[2] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[4]) << 0;
  l |= itoa64_to_int(buf[5]) << 6;
  l |= itoa64_to_int(buf[6]) << 12;
  l |= itoa64_to_int(buf[7]) << 18;

  digest[3] = (l >> 0) & 0xff;
  digest[4] = (l >> 8) & 0xff;
  digest[5] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[8]) << 0;
  l |= itoa64_to_int(buf[9]) << 6;
  l |= itoa64_to_int(buf[10]) << 12;
  l |= itoa64_to_int(buf[11]) << 18;

  digest[6] = (l >> 0) & 0xff;
  digest[7] = (l >> 8) & 0xff;
  digest[8] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[12]) << 0;
  l |= itoa64_to_int(buf[13]) << 6;
  l |= itoa64_to_int(buf[14]) << 12;
  l |= itoa64_to_int(buf[15]) << 18;

  digest[9] = (l >> 0) & 0xff;
  digest[10] = (l >> 8) & 0xff;
  digest[11] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[16]) << 0;
  l |= itoa64_to_int(buf[17]) << 6;
  l |= itoa64_to_int(buf[18]) << 12;
  l |= itoa64_to_int(buf[19]) << 18;

  digest[12] = (l >> 0) & 0xff;
  digest[13] = (l >> 8) & 0xff;
  digest[14] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[20]) << 0;
  l |= itoa64_to_int(buf[21]) << 6;

  digest[15] = (l >> 0) & 0xff;
}

void phpass_encode(u8 digest[16], u8 buf[22])
{
  int l;

  l = (digest[0] << 0) | (digest[1] << 8) | (digest[2] << 16);

  buf[0] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[1] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[2] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[3] = int_to_itoa64(l & 0x3f);

  l = (digest[3] << 0) | (digest[4] << 8) | (digest[5] << 16);

  buf[4] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[5] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[6] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[7] = int_to_itoa64(l & 0x3f);

  l = (digest[6] << 0) | (digest[7] << 8) | (digest[8] << 16);

  buf[8] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[9] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[10] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[11] = int_to_itoa64(l & 0x3f);

  l = (digest[9] << 0) | (digest[10] << 8) | (digest[11] << 16);

  buf[12] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[13] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[14] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[15] = int_to_itoa64(l & 0x3f);

  l = (digest[12] << 0) | (digest[13] << 8) | (digest[14] << 16);

  buf[16] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[17] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[18] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[19] = int_to_itoa64(l & 0x3f);

  l = (digest[15] << 0);

  buf[20] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[21] = int_to_itoa64(l & 0x3f);
}

void md5crypt_decode(u8 digest[16], u8 buf[22])
{
  int l;

  l = itoa64_to_int(buf[0]) << 0;
  l |= itoa64_to_int(buf[1]) << 6;
  l |= itoa64_to_int(buf[2]) << 12;
  l |= itoa64_to_int(buf[3]) << 18;

  digest[0] = (l >> 16) & 0xff;
  digest[6] = (l >> 8) & 0xff;
  digest[12] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[4]) << 0;
  l |= itoa64_to_int(buf[5]) << 6;
  l |= itoa64_to_int(buf[6]) << 12;
  l |= itoa64_to_int(buf[7]) << 18;

  digest[1] = (l >> 16) & 0xff;
  digest[7] = (l >> 8) & 0xff;
  digest[13] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[8]) << 0;
  l |= itoa64_to_int(buf[9]) << 6;
  l |= itoa64_to_int(buf[10]) << 12;
  l |= itoa64_to_int(buf[11]) << 18;

  digest[2] = (l >> 16) & 0xff;
  digest[8] = (l >> 8) & 0xff;
  digest[14] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[12]) << 0;
  l |= itoa64_to_int(buf[13]) << 6;
  l |= itoa64_to_int(buf[14]) << 12;
  l |= itoa64_to_int(buf[15]) << 18;

  digest[3] = (l >> 16) & 0xff;
  digest[9] = (l >> 8) & 0xff;
  digest[15] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[16]) << 0;
  l |= itoa64_to_int(buf[17]) << 6;
  l |= itoa64_to_int(buf[18]) << 12;
  l |= itoa64_to_int(buf[19]) << 18;

  digest[4] = (l >> 16) & 0xff;
  digest[10] = (l >> 8) & 0xff;
  digest[5] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[20]) << 0;
  l |= itoa64_to_int(buf[21]) << 6;

  digest[11] = (l >> 0) & 0xff;
}

void md5crypt_encode(u8 digest[16], u8 buf[22])
{
  int l;

  l = (digest[0] << 16) | (digest[6] << 8) | (digest[12] << 0);

  buf[0] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[1] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[2] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[3] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[1] << 16) | (digest[7] << 8) | (digest[13] << 0);

  buf[4] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[5] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[6] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[7] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[2] << 16) | (digest[8] << 8) | (digest[14] << 0);

  buf[8] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[9] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[10] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[11] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[3] << 16) | (digest[9] << 8) | (digest[15] << 0);

  buf[12] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[13] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[14] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[15] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[4] << 16) | (digest[10] << 8) | (digest[5] << 0);

  buf[16] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[17] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[18] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[19] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[11] << 0);

  buf[20] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[21] = int_to_itoa64(l & 0x3f); l >>= 6;
}

void sha512crypt_decode(u8 digest[64], u8 buf[86])
{
  int l;

  l = itoa64_to_int(buf[0]) << 0;
  l |= itoa64_to_int(buf[1]) << 6;
  l |= itoa64_to_int(buf[2]) << 12;
  l |= itoa64_to_int(buf[3]) << 18;

  digest[0] = (l >> 16) & 0xff;
  digest[21] = (l >> 8) & 0xff;
  digest[42] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[4]) << 0;
  l |= itoa64_to_int(buf[5]) << 6;
  l |= itoa64_to_int(buf[6]) << 12;
  l |= itoa64_to_int(buf[7]) << 18;

  digest[22] = (l >> 16) & 0xff;
  digest[43] = (l >> 8) & 0xff;
  digest[1] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[8]) << 0;
  l |= itoa64_to_int(buf[9]) << 6;
  l |= itoa64_to_int(buf[10]) << 12;
  l |= itoa64_to_int(buf[11]) << 18;

  digest[44] = (l >> 16) & 0xff;
  digest[2] = (l >> 8) & 0xff;
  digest[23] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[12]) << 0;
  l |= itoa64_to_int(buf[13]) << 6;
  l |= itoa64_to_int(buf[14]) << 12;
  l |= itoa64_to_int(buf[15]) << 18;

  digest[3] = (l >> 16) & 0xff;
  digest[24] = (l >> 8) & 0xff;
  digest[45] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[16]) << 0;
  l |= itoa64_to_int(buf[17]) << 6;
  l |= itoa64_to_int(buf[18]) << 12;
  l |= itoa64_to_int(buf[19]) << 18;

  digest[25] = (l >> 16) & 0xff;
  digest[46] = (l >> 8) & 0xff;
  digest[4] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[20]) << 0;
  l |= itoa64_to_int(buf[21]) << 6;
  l |= itoa64_to_int(buf[22]) << 12;
  l |= itoa64_to_int(buf[23]) << 18;

  digest[47] = (l >> 16) & 0xff;
  digest[5] = (l >> 8) & 0xff;
  digest[26] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[24]) << 0;
  l |= itoa64_to_int(buf[25]) << 6;
  l |= itoa64_to_int(buf[26]) << 12;
  l |= itoa64_to_int(buf[27]) << 18;

  digest[6] = (l >> 16) & 0xff;
  digest[27] = (l >> 8) & 0xff;
  digest[48] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[28]) << 0;
  l |= itoa64_to_int(buf[29]) << 6;
  l |= itoa64_to_int(buf[30]) << 12;
  l |= itoa64_to_int(buf[31]) << 18;

  digest[28] = (l >> 16) & 0xff;
  digest[49] = (l >> 8) & 0xff;
  digest[7] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[32]) << 0;
  l |= itoa64_to_int(buf[33]) << 6;
  l |= itoa64_to_int(buf[34]) << 12;
  l |= itoa64_to_int(buf[35]) << 18;

  digest[50] = (l >> 16) & 0xff;
  digest[8] = (l >> 8) & 0xff;
  digest[29] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[36]) << 0;
  l |= itoa64_to_int(buf[37]) << 6;
  l |= itoa64_to_int(buf[38]) << 12;
  l |= itoa64_to_int(buf[39]) << 18;

  digest[9] = (l >> 16) & 0xff;
  digest[30] = (l >> 8) & 0xff;
  digest[51] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[40]) << 0;
  l |= itoa64_to_int(buf[41]) << 6;
  l |= itoa64_to_int(buf[42]) << 12;
  l |= itoa64_to_int(buf[43]) << 18;

  digest[31] = (l >> 16) & 0xff;
  digest[52] = (l >> 8) & 0xff;
  digest[10] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[44]) << 0;
  l |= itoa64_to_int(buf[45]) << 6;
  l |= itoa64_to_int(buf[46]) << 12;
  l |= itoa64_to_int(buf[47]) << 18;

  digest[53] = (l >> 16) & 0xff;
  digest[11] = (l >> 8) & 0xff;
  digest[32] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[48]) << 0;
  l |= itoa64_to_int(buf[49]) << 6;
  l |= itoa64_to_int(buf[50]) << 12;
  l |= itoa64_to_int(buf[51]) << 18;

  digest[12] = (l >> 16) & 0xff;
  digest[33] = (l >> 8) & 0xff;
  digest[54] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[52]) << 0;
  l |= itoa64_to_int(buf[53]) << 6;
  l |= itoa64_to_int(buf[54]) << 12;
  l |= itoa64_to_int(buf[55]) << 18;

  digest[34] = (l >> 16) & 0xff;
  digest[55] = (l >> 8) & 0xff;
  digest[13] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[56]) << 0;
  l |= itoa64_to_int(buf[57]) << 6;
  l |= itoa64_to_int(buf[58]) << 12;
  l |= itoa64_to_int(buf[59]) << 18;

  digest[56] = (l >> 16) & 0xff;
  digest[14] = (l >> 8) & 0xff;
  digest[35] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[60]) << 0;
  l |= itoa64_to_int(buf[61]) << 6;
  l |= itoa64_to_int(buf[62]) << 12;
  l |= itoa64_to_int(buf[63]) << 18;

  digest[15] = (l >> 16) & 0xff;
  digest[36] = (l >> 8) & 0xff;
  digest[57] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[64]) << 0;
  l |= itoa64_to_int(buf[65]) << 6;
  l |= itoa64_to_int(buf[66]) << 12;
  l |= itoa64_to_int(buf[67]) << 18;

  digest[37] = (l >> 16) & 0xff;
  digest[58] = (l >> 8) & 0xff;
  digest[16] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[68]) << 0;
  l |= itoa64_to_int(buf[69]) << 6;
  l |= itoa64_to_int(buf[70]) << 12;
  l |= itoa64_to_int(buf[71]) << 18;

  digest[59] = (l >> 16) & 0xff;
  digest[17] = (l >> 8) & 0xff;
  digest[38] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[72]) << 0;
  l |= itoa64_to_int(buf[73]) << 6;
  l |= itoa64_to_int(buf[74]) << 12;
  l |= itoa64_to_int(buf[75]) << 18;

  digest[18] = (l >> 16) & 0xff;
  digest[39] = (l >> 8) & 0xff;
  digest[60] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[76]) << 0;
  l |= itoa64_to_int(buf[77]) << 6;
  l |= itoa64_to_int(buf[78]) << 12;
  l |= itoa64_to_int(buf[79]) << 18;

  digest[40] = (l >> 16) & 0xff;
  digest[61] = (l >> 8) & 0xff;
  digest[19] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[80]) << 0;
  l |= itoa64_to_int(buf[81]) << 6;
  l |= itoa64_to_int(buf[82]) << 12;
  l |= itoa64_to_int(buf[83]) << 18;

  digest[62] = (l >> 16) & 0xff;
  digest[20] = (l >> 8) & 0xff;
  digest[41] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[84]) << 0;
  l |= itoa64_to_int(buf[85]) << 6;

  digest[63] = (l >> 0) & 0xff;
}

void sha512crypt_encode(u8 digest[64], u8 buf[86])
{
  int l;

  l = (digest[0] << 16) | (digest[21] << 8) | (digest[42] << 0);

  buf[0] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[1] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[2] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[3] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[22] << 16) | (digest[43] << 8) | (digest[1] << 0);

  buf[4] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[5] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[6] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[7] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[44] << 16) | (digest[2] << 8) | (digest[23] << 0);

  buf[8] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[9] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[10] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[11] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[3] << 16) | (digest[24] << 8) | (digest[45] << 0);

  buf[12] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[13] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[14] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[15] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[25] << 16) | (digest[46] << 8) | (digest[4] << 0);

  buf[16] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[17] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[18] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[19] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[47] << 16) | (digest[5] << 8) | (digest[26] << 0);

  buf[20] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[21] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[22] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[23] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[6] << 16) | (digest[27] << 8) | (digest[48] << 0);

  buf[24] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[25] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[26] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[27] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[28] << 16) | (digest[49] << 8) | (digest[7] << 0);

  buf[28] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[29] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[30] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[31] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[50] << 16) | (digest[8] << 8) | (digest[29] << 0);

  buf[32] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[33] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[34] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[35] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[9] << 16) | (digest[30] << 8) | (digest[51] << 0);

  buf[36] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[37] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[38] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[39] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[31] << 16) | (digest[52] << 8) | (digest[10] << 0);

  buf[40] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[41] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[42] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[43] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[53] << 16) | (digest[11] << 8) | (digest[32] << 0);

  buf[44] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[45] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[46] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[47] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[12] << 16) | (digest[33] << 8) | (digest[54] << 0);

  buf[48] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[49] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[50] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[51] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[34] << 16) | (digest[55] << 8) | (digest[13] << 0);

  buf[52] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[53] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[54] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[55] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[56] << 16) | (digest[14] << 8) | (digest[35] << 0);

  buf[56] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[57] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[58] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[59] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[15] << 16) | (digest[36] << 8) | (digest[57] << 0);

  buf[60] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[61] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[62] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[63] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[37] << 16) | (digest[58] << 8) | (digest[16] << 0);

  buf[64] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[65] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[66] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[67] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[59] << 16) | (digest[17] << 8) | (digest[38] << 0);

  buf[68] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[69] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[70] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[71] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[18] << 16) | (digest[39] << 8) | (digest[60] << 0);

  buf[72] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[73] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[74] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[75] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[40] << 16) | (digest[61] << 8) | (digest[19] << 0);

  buf[76] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[77] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[78] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[79] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[62] << 16) | (digest[20] << 8) | (digest[41] << 0);

  buf[80] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[81] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[82] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[83] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = 0 | 0 | (digest[63] << 0);

  buf[84] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[85] = int_to_itoa64(l & 0x3f); l >>= 6;
}

void sha1aix_decode(u8 digest[20], u8 buf[27])
{
  int l;

  l = itoa64_to_int(buf[0]) << 0;
  l |= itoa64_to_int(buf[1]) << 6;
  l |= itoa64_to_int(buf[2]) << 12;
  l |= itoa64_to_int(buf[3]) << 18;

  digest[2] = (l >> 0) & 0xff;
  digest[1] = (l >> 8) & 0xff;
  digest[0] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[4]) << 0;
  l |= itoa64_to_int(buf[5]) << 6;
  l |= itoa64_to_int(buf[6]) << 12;
  l |= itoa64_to_int(buf[7]) << 18;

  digest[5] = (l >> 0) & 0xff;
  digest[4] = (l >> 8) & 0xff;
  digest[3] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[8]) << 0;
  l |= itoa64_to_int(buf[9]) << 6;
  l |= itoa64_to_int(buf[10]) << 12;
  l |= itoa64_to_int(buf[11]) << 18;

  digest[8] = (l >> 0) & 0xff;
  digest[7] = (l >> 8) & 0xff;
  digest[6] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[12]) << 0;
  l |= itoa64_to_int(buf[13]) << 6;
  l |= itoa64_to_int(buf[14]) << 12;
  l |= itoa64_to_int(buf[15]) << 18;

  digest[11] = (l >> 0) & 0xff;
  digest[10] = (l >> 8) & 0xff;
  digest[9] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[16]) << 0;
  l |= itoa64_to_int(buf[17]) << 6;
  l |= itoa64_to_int(buf[18]) << 12;
  l |= itoa64_to_int(buf[19]) << 18;

  digest[14] = (l >> 0) & 0xff;
  digest[13] = (l >> 8) & 0xff;
  digest[12] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[20]) << 0;
  l |= itoa64_to_int(buf[21]) << 6;
  l |= itoa64_to_int(buf[22]) << 12;
  l |= itoa64_to_int(buf[23]) << 18;

  digest[17] = (l >> 0) & 0xff;
  digest[16] = (l >> 8) & 0xff;
  digest[15] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[24]) << 0;
  l |= itoa64_to_int(buf[25]) << 6;
  l |= itoa64_to_int(buf[26]) << 12;

  digest[19] = (l >> 8) & 0xff;
  digest[18] = (l >> 16) & 0xff;
}

void sha1aix_encode(u8 digest[20], u8 buf[27])
{
  int l;

  l = (digest[2] << 0) | (digest[1] << 8) | (digest[0] << 16);

  buf[0] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[1] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[2] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[3] = int_to_itoa64(l & 0x3f);

  l = (digest[5] << 0) | (digest[4] << 8) | (digest[3] << 16);

  buf[4] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[5] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[6] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[7] = int_to_itoa64(l & 0x3f);

  l = (digest[8] << 0) | (digest[7] << 8) | (digest[6] << 16);

  buf[8] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[9] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[10] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[11] = int_to_itoa64(l & 0x3f);

  l = (digest[11] << 0) | (digest[10] << 8) | (digest[9] << 16);

  buf[12] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[13] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[14] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[15] = int_to_itoa64(l & 0x3f);

  l = (digest[14] << 0) | (digest[13] << 8) | (digest[12] << 16);

  buf[16] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[17] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[18] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[19] = int_to_itoa64(l & 0x3f);

  l = (digest[17] << 0) | (digest[16] << 8) | (digest[15] << 16);

  buf[20] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[21] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[22] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[23] = int_to_itoa64(l & 0x3f);

  l = 0 | (digest[19] << 8) | (digest[18] << 16);

  buf[24] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[25] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[26] = int_to_itoa64(l & 0x3f);
}

void sha256aix_decode(u8 digest[32], u8 buf[43])
{
  int l;

  l = itoa64_to_int(buf[0]) << 0;
  l |= itoa64_to_int(buf[1]) << 6;
  l |= itoa64_to_int(buf[2]) << 12;
  l |= itoa64_to_int(buf[3]) << 18;

  digest[2] = (l >> 0) & 0xff;
  digest[1] = (l >> 8) & 0xff;
  digest[0] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[4]) << 0;
  l |= itoa64_to_int(buf[5]) << 6;
  l |= itoa64_to_int(buf[6]) << 12;
  l |= itoa64_to_int(buf[7]) << 18;

  digest[5] = (l >> 0) & 0xff;
  digest[4] = (l >> 8) & 0xff;
  digest[3] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[8]) << 0;
  l |= itoa64_to_int(buf[9]) << 6;
  l |= itoa64_to_int(buf[10]) << 12;
  l |= itoa64_to_int(buf[11]) << 18;

  digest[8] = (l >> 0) & 0xff;
  digest[7] = (l >> 8) & 0xff;
  digest[6] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[12]) << 0;
  l |= itoa64_to_int(buf[13]) << 6;
  l |= itoa64_to_int(buf[14]) << 12;
  l |= itoa64_to_int(buf[15]) << 18;

  digest[11] = (l >> 0) & 0xff;
  digest[10] = (l >> 8) & 0xff;
  digest[9] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[16]) << 0;
  l |= itoa64_to_int(buf[17]) << 6;
  l |= itoa64_to_int(buf[18]) << 12;
  l |= itoa64_to_int(buf[19]) << 18;

  digest[14] = (l >> 0) & 0xff;
  digest[13] = (l >> 8) & 0xff;
  digest[12] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[20]) << 0;
  l |= itoa64_to_int(buf[21]) << 6;
  l |= itoa64_to_int(buf[22]) << 12;
  l |= itoa64_to_int(buf[23]) << 18;

  digest[17] = (l >> 0) & 0xff;
  digest[16] = (l >> 8) & 0xff;
  digest[15] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[24]) << 0;
  l |= itoa64_to_int(buf[25]) << 6;
  l |= itoa64_to_int(buf[26]) << 12;
  l |= itoa64_to_int(buf[27]) << 18;

  digest[20] = (l >> 0) & 0xff;
  digest[19] = (l >> 8) & 0xff;
  digest[18] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[28]) << 0;
  l |= itoa64_to_int(buf[29]) << 6;
  l |= itoa64_to_int(buf[30]) << 12;
  l |= itoa64_to_int(buf[31]) << 18;

  digest[23] = (l >> 0) & 0xff;
  digest[22] = (l >> 8) & 0xff;
  digest[21] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[32]) << 0;
  l |= itoa64_to_int(buf[33]) << 6;
  l |= itoa64_to_int(buf[34]) << 12;
  l |= itoa64_to_int(buf[35]) << 18;

  digest[26] = (l >> 0) & 0xff;
  digest[25] = (l >> 8) & 0xff;
  digest[24] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[36]) << 0;
  l |= itoa64_to_int(buf[37]) << 6;
  l |= itoa64_to_int(buf[38]) << 12;
  l |= itoa64_to_int(buf[39]) << 18;

  digest[29] = (l >> 0) & 0xff;
  digest[28] = (l >> 8) & 0xff;
  digest[27] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[40]) << 0;
  l |= itoa64_to_int(buf[41]) << 6;
  l |= itoa64_to_int(buf[42]) << 12;

  //digest[32] = (l >>  0) & 0xff;
  digest[31] = (l >> 8) & 0xff;
  digest[30] = (l >> 16) & 0xff;
}

void sha256aix_encode(u8 digest[32], u8 buf[43])
{
  int l;

  l = (digest[2] << 0) | (digest[1] << 8) | (digest[0] << 16);

  buf[0] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[1] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[2] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[3] = int_to_itoa64(l & 0x3f);

  l = (digest[5] << 0) | (digest[4] << 8) | (digest[3] << 16);

  buf[4] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[5] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[6] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[7] = int_to_itoa64(l & 0x3f);

  l = (digest[8] << 0) | (digest[7] << 8) | (digest[6] << 16);

  buf[8] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[9] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[10] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[11] = int_to_itoa64(l & 0x3f);

  l = (digest[11] << 0) | (digest[10] << 8) | (digest[9] << 16);

  buf[12] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[13] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[14] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[15] = int_to_itoa64(l & 0x3f);

  l = (digest[14] << 0) | (digest[13] << 8) | (digest[12] << 16);

  buf[16] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[17] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[18] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[19] = int_to_itoa64(l & 0x3f);

  l = (digest[17] << 0) | (digest[16] << 8) | (digest[15] << 16);

  buf[20] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[21] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[22] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[23] = int_to_itoa64(l & 0x3f);

  l = (digest[20] << 0) | (digest[19] << 8) | (digest[18] << 16);

  buf[24] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[25] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[26] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[27] = int_to_itoa64(l & 0x3f);

  l = (digest[23] << 0) | (digest[22] << 8) | (digest[21] << 16);

  buf[28] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[29] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[30] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[31] = int_to_itoa64(l & 0x3f);

  l = (digest[26] << 0) | (digest[25] << 8) | (digest[24] << 16);

  buf[32] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[33] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[34] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[35] = int_to_itoa64(l & 0x3f);

  l = (digest[29] << 0) | (digest[28] << 8) | (digest[27] << 16);

  buf[36] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[37] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[38] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[39] = int_to_itoa64(l & 0x3f);

  l = 0 | (digest[31] << 8) | (digest[30] << 16);

  buf[40] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[41] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[42] = int_to_itoa64(l & 0x3f);
}

void sha512aix_decode(u8 digest[64], u8 buf[86])
{
  int l;

  l = itoa64_to_int(buf[0]) << 0;
  l |= itoa64_to_int(buf[1]) << 6;
  l |= itoa64_to_int(buf[2]) << 12;
  l |= itoa64_to_int(buf[3]) << 18;

  digest[2] = (l >> 0) & 0xff;
  digest[1] = (l >> 8) & 0xff;
  digest[0] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[4]) << 0;
  l |= itoa64_to_int(buf[5]) << 6;
  l |= itoa64_to_int(buf[6]) << 12;
  l |= itoa64_to_int(buf[7]) << 18;

  digest[5] = (l >> 0) & 0xff;
  digest[4] = (l >> 8) & 0xff;
  digest[3] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[8]) << 0;
  l |= itoa64_to_int(buf[9]) << 6;
  l |= itoa64_to_int(buf[10]) << 12;
  l |= itoa64_to_int(buf[11]) << 18;

  digest[8] = (l >> 0) & 0xff;
  digest[7] = (l >> 8) & 0xff;
  digest[6] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[12]) << 0;
  l |= itoa64_to_int(buf[13]) << 6;
  l |= itoa64_to_int(buf[14]) << 12;
  l |= itoa64_to_int(buf[15]) << 18;

  digest[11] = (l >> 0) & 0xff;
  digest[10] = (l >> 8) & 0xff;
  digest[9] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[16]) << 0;
  l |= itoa64_to_int(buf[17]) << 6;
  l |= itoa64_to_int(buf[18]) << 12;
  l |= itoa64_to_int(buf[19]) << 18;

  digest[14] = (l >> 0) & 0xff;
  digest[13] = (l >> 8) & 0xff;
  digest[12] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[20]) << 0;
  l |= itoa64_to_int(buf[21]) << 6;
  l |= itoa64_to_int(buf[22]) << 12;
  l |= itoa64_to_int(buf[23]) << 18;

  digest[17] = (l >> 0) & 0xff;
  digest[16] = (l >> 8) & 0xff;
  digest[15] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[24]) << 0;
  l |= itoa64_to_int(buf[25]) << 6;
  l |= itoa64_to_int(buf[26]) << 12;
  l |= itoa64_to_int(buf[27]) << 18;

  digest[20] = (l >> 0) & 0xff;
  digest[19] = (l >> 8) & 0xff;
  digest[18] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[28]) << 0;
  l |= itoa64_to_int(buf[29]) << 6;
  l |= itoa64_to_int(buf[30]) << 12;
  l |= itoa64_to_int(buf[31]) << 18;

  digest[23] = (l >> 0) & 0xff;
  digest[22] = (l >> 8) & 0xff;
  digest[21] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[32]) << 0;
  l |= itoa64_to_int(buf[33]) << 6;
  l |= itoa64_to_int(buf[34]) << 12;
  l |= itoa64_to_int(buf[35]) << 18;

  digest[26] = (l >> 0) & 0xff;
  digest[25] = (l >> 8) & 0xff;
  digest[24] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[36]) << 0;
  l |= itoa64_to_int(buf[37]) << 6;
  l |= itoa64_to_int(buf[38]) << 12;
  l |= itoa64_to_int(buf[39]) << 18;

  digest[29] = (l >> 0) & 0xff;
  digest[28] = (l >> 8) & 0xff;
  digest[27] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[40]) << 0;
  l |= itoa64_to_int(buf[41]) << 6;
  l |= itoa64_to_int(buf[42]) << 12;
  l |= itoa64_to_int(buf[43]) << 18;

  digest[32] = (l >> 0) & 0xff;
  digest[31] = (l >> 8) & 0xff;
  digest[30] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[44]) << 0;
  l |= itoa64_to_int(buf[45]) << 6;
  l |= itoa64_to_int(buf[46]) << 12;
  l |= itoa64_to_int(buf[47]) << 18;

  digest[35] = (l >> 0) & 0xff;
  digest[34] = (l >> 8) & 0xff;
  digest[33] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[48]) << 0;
  l |= itoa64_to_int(buf[49]) << 6;
  l |= itoa64_to_int(buf[50]) << 12;
  l |= itoa64_to_int(buf[51]) << 18;

  digest[38] = (l >> 0) & 0xff;
  digest[37] = (l >> 8) & 0xff;
  digest[36] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[52]) << 0;
  l |= itoa64_to_int(buf[53]) << 6;
  l |= itoa64_to_int(buf[54]) << 12;
  l |= itoa64_to_int(buf[55]) << 18;

  digest[41] = (l >> 0) & 0xff;
  digest[40] = (l >> 8) & 0xff;
  digest[39] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[56]) << 0;
  l |= itoa64_to_int(buf[57]) << 6;
  l |= itoa64_to_int(buf[58]) << 12;
  l |= itoa64_to_int(buf[59]) << 18;

  digest[44] = (l >> 0) & 0xff;
  digest[43] = (l >> 8) & 0xff;
  digest[42] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[60]) << 0;
  l |= itoa64_to_int(buf[61]) << 6;
  l |= itoa64_to_int(buf[62]) << 12;
  l |= itoa64_to_int(buf[63]) << 18;

  digest[47] = (l >> 0) & 0xff;
  digest[46] = (l >> 8) & 0xff;
  digest[45] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[64]) << 0;
  l |= itoa64_to_int(buf[65]) << 6;
  l |= itoa64_to_int(buf[66]) << 12;
  l |= itoa64_to_int(buf[67]) << 18;

  digest[50] = (l >> 0) & 0xff;
  digest[49] = (l >> 8) & 0xff;
  digest[48] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[68]) << 0;
  l |= itoa64_to_int(buf[69]) << 6;
  l |= itoa64_to_int(buf[70]) << 12;
  l |= itoa64_to_int(buf[71]) << 18;

  digest[53] = (l >> 0) & 0xff;
  digest[52] = (l >> 8) & 0xff;
  digest[51] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[72]) << 0;
  l |= itoa64_to_int(buf[73]) << 6;
  l |= itoa64_to_int(buf[74]) << 12;
  l |= itoa64_to_int(buf[75]) << 18;

  digest[56] = (l >> 0) & 0xff;
  digest[55] = (l >> 8) & 0xff;
  digest[54] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[76]) << 0;
  l |= itoa64_to_int(buf[77]) << 6;
  l |= itoa64_to_int(buf[78]) << 12;
  l |= itoa64_to_int(buf[79]) << 18;

  digest[59] = (l >> 0) & 0xff;
  digest[58] = (l >> 8) & 0xff;
  digest[57] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[80]) << 0;
  l |= itoa64_to_int(buf[81]) << 6;
  l |= itoa64_to_int(buf[82]) << 12;
  l |= itoa64_to_int(buf[83]) << 18;

  digest[62] = (l >> 0) & 0xff;
  digest[61] = (l >> 8) & 0xff;
  digest[60] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[84]) << 0;
  l |= itoa64_to_int(buf[85]) << 6;

  digest[63] = (l >> 16) & 0xff;
}

void sha512aix_encode(u8 digest[64], u8 buf[86])
{
  int l;

  l = (digest[2] << 0) | (digest[1] << 8) | (digest[0] << 16);

  buf[0] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[1] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[2] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[3] = int_to_itoa64(l & 0x3f);

  l = (digest[5] << 0) | (digest[4] << 8) | (digest[3] << 16);

  buf[4] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[5] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[6] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[7] = int_to_itoa64(l & 0x3f);

  l = (digest[8] << 0) | (digest[7] << 8) | (digest[6] << 16);

  buf[8] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[9] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[10] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[11] = int_to_itoa64(l & 0x3f);

  l = (digest[11] << 0) | (digest[10] << 8) | (digest[9] << 16);

  buf[12] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[13] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[14] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[15] = int_to_itoa64(l & 0x3f);

  l = (digest[14] << 0) | (digest[13] << 8) | (digest[12] << 16);

  buf[16] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[17] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[18] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[19] = int_to_itoa64(l & 0x3f);

  l = (digest[17] << 0) | (digest[16] << 8) | (digest[15] << 16);

  buf[20] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[21] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[22] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[23] = int_to_itoa64(l & 0x3f);

  l = (digest[20] << 0) | (digest[19] << 8) | (digest[18] << 16);

  buf[24] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[25] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[26] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[27] = int_to_itoa64(l & 0x3f);

  l = (digest[23] << 0) | (digest[22] << 8) | (digest[21] << 16);

  buf[28] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[29] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[30] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[31] = int_to_itoa64(l & 0x3f);

  l = (digest[26] << 0) | (digest[25] << 8) | (digest[24] << 16);

  buf[32] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[33] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[34] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[35] = int_to_itoa64(l & 0x3f);

  l = (digest[29] << 0) | (digest[28] << 8) | (digest[27] << 16);

  buf[36] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[37] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[38] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[39] = int_to_itoa64(l & 0x3f);

  l = (digest[32] << 0) | (digest[31] << 8) | (digest[30] << 16);

  buf[40] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[41] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[42] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[43] = int_to_itoa64(l & 0x3f);

  l = (digest[35] << 0) | (digest[34] << 8) | (digest[33] << 16);

  buf[44] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[45] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[46] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[47] = int_to_itoa64(l & 0x3f);

  l = (digest[38] << 0) | (digest[37] << 8) | (digest[36] << 16);

  buf[48] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[49] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[50] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[51] = int_to_itoa64(l & 0x3f);

  l = (digest[41] << 0) | (digest[40] << 8) | (digest[39] << 16);

  buf[52] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[53] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[54] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[55] = int_to_itoa64(l & 0x3f);

  l = (digest[44] << 0) | (digest[43] << 8) | (digest[42] << 16);

  buf[56] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[57] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[58] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[59] = int_to_itoa64(l & 0x3f);

  l = (digest[47] << 0) | (digest[46] << 8) | (digest[45] << 16);

  buf[60] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[61] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[62] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[63] = int_to_itoa64(l & 0x3f);

  l = (digest[50] << 0) | (digest[49] << 8) | (digest[48] << 16);

  buf[64] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[65] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[66] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[67] = int_to_itoa64(l & 0x3f);

  l = (digest[53] << 0) | (digest[52] << 8) | (digest[51] << 16);

  buf[68] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[69] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[70] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[71] = int_to_itoa64(l & 0x3f);

  l = (digest[56] << 0) | (digest[55] << 8) | (digest[54] << 16);

  buf[72] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[73] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[74] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[75] = int_to_itoa64(l & 0x3f);

  l = (digest[59] << 0) | (digest[58] << 8) | (digest[57] << 16);

  buf[76] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[77] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[78] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[79] = int_to_itoa64(l & 0x3f);

  l = (digest[62] << 0) | (digest[61] << 8) | (digest[60] << 16);

  buf[80] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[81] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[82] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[83] = int_to_itoa64(l & 0x3f);

  l = 0 | 0 | (digest[63] << 16);

  buf[84] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[85] = int_to_itoa64(l & 0x3f); l >>= 6;
}

void sha256crypt_decode(u8 digest[32], u8 buf[43])
{
  int l;

  l = itoa64_to_int(buf[0]) << 0;
  l |= itoa64_to_int(buf[1]) << 6;
  l |= itoa64_to_int(buf[2]) << 12;
  l |= itoa64_to_int(buf[3]) << 18;

  digest[0] = (l >> 16) & 0xff;
  digest[10] = (l >> 8) & 0xff;
  digest[20] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[4]) << 0;
  l |= itoa64_to_int(buf[5]) << 6;
  l |= itoa64_to_int(buf[6]) << 12;
  l |= itoa64_to_int(buf[7]) << 18;

  digest[21] = (l >> 16) & 0xff;
  digest[1] = (l >> 8) & 0xff;
  digest[11] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[8]) << 0;
  l |= itoa64_to_int(buf[9]) << 6;
  l |= itoa64_to_int(buf[10]) << 12;
  l |= itoa64_to_int(buf[11]) << 18;

  digest[12] = (l >> 16) & 0xff;
  digest[22] = (l >> 8) & 0xff;
  digest[2] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[12]) << 0;
  l |= itoa64_to_int(buf[13]) << 6;
  l |= itoa64_to_int(buf[14]) << 12;
  l |= itoa64_to_int(buf[15]) << 18;

  digest[3] = (l >> 16) & 0xff;
  digest[13] = (l >> 8) & 0xff;
  digest[23] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[16]) << 0;
  l |= itoa64_to_int(buf[17]) << 6;
  l |= itoa64_to_int(buf[18]) << 12;
  l |= itoa64_to_int(buf[19]) << 18;

  digest[24] = (l >> 16) & 0xff;
  digest[4] = (l >> 8) & 0xff;
  digest[14] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[20]) << 0;
  l |= itoa64_to_int(buf[21]) << 6;
  l |= itoa64_to_int(buf[22]) << 12;
  l |= itoa64_to_int(buf[23]) << 18;

  digest[15] = (l >> 16) & 0xff;
  digest[25] = (l >> 8) & 0xff;
  digest[5] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[24]) << 0;
  l |= itoa64_to_int(buf[25]) << 6;
  l |= itoa64_to_int(buf[26]) << 12;
  l |= itoa64_to_int(buf[27]) << 18;

  digest[6] = (l >> 16) & 0xff;
  digest[16] = (l >> 8) & 0xff;
  digest[26] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[28]) << 0;
  l |= itoa64_to_int(buf[29]) << 6;
  l |= itoa64_to_int(buf[30]) << 12;
  l |= itoa64_to_int(buf[31]) << 18;

  digest[27] = (l >> 16) & 0xff;
  digest[7] = (l >> 8) & 0xff;
  digest[17] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[32]) << 0;
  l |= itoa64_to_int(buf[33]) << 6;
  l |= itoa64_to_int(buf[34]) << 12;
  l |= itoa64_to_int(buf[35]) << 18;

  digest[18] = (l >> 16) & 0xff;
  digest[28] = (l >> 8) & 0xff;
  digest[8] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[36]) << 0;
  l |= itoa64_to_int(buf[37]) << 6;
  l |= itoa64_to_int(buf[38]) << 12;
  l |= itoa64_to_int(buf[39]) << 18;

  digest[9] = (l >> 16) & 0xff;
  digest[19] = (l >> 8) & 0xff;
  digest[29] = (l >> 0) & 0xff;

  l = itoa64_to_int(buf[40]) << 0;
  l |= itoa64_to_int(buf[41]) << 6;
  l |= itoa64_to_int(buf[42]) << 12;

  digest[31] = (l >> 8) & 0xff;
  digest[30] = (l >> 0) & 0xff;
}

void sha256crypt_encode(u8 digest[32], u8 buf[43])
{
  int l;

  l = (digest[0] << 16) | (digest[10] << 8) | (digest[20] << 0);

  buf[0] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[1] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[2] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[3] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[21] << 16) | (digest[1] << 8) | (digest[11] << 0);

  buf[4] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[5] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[6] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[7] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[12] << 16) | (digest[22] << 8) | (digest[2] << 0);

  buf[8] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[9] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[10] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[11] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[3] << 16) | (digest[13] << 8) | (digest[23] << 0);

  buf[12] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[13] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[14] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[15] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[24] << 16) | (digest[4] << 8) | (digest[14] << 0);

  buf[16] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[17] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[18] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[19] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[15] << 16) | (digest[25] << 8) | (digest[5] << 0);

  buf[20] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[21] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[22] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[23] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[6] << 16) | (digest[16] << 8) | (digest[26] << 0);

  buf[24] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[25] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[26] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[27] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[27] << 16) | (digest[7] << 8) | (digest[17] << 0);

  buf[28] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[29] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[30] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[31] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[18] << 16) | (digest[28] << 8) | (digest[8] << 0);

  buf[32] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[33] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[34] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[35] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = (digest[9] << 16) | (digest[19] << 8) | (digest[29] << 0);

  buf[36] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[37] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[38] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[39] = int_to_itoa64(l & 0x3f); l >>= 6;

  l = 0 | (digest[31] << 8) | (digest[30] << 0);

  buf[40] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[41] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[42] = int_to_itoa64(l & 0x3f);
}

void drupal7_decode(u8 digest[64], u8 buf[44])
{
  int l;

  l = itoa64_to_int(buf[0]) << 0;
  l |= itoa64_to_int(buf[1]) << 6;
  l |= itoa64_to_int(buf[2]) << 12;
  l |= itoa64_to_int(buf[3]) << 18;

  digest[0] = (l >> 0) & 0xff;
  digest[1] = (l >> 8) & 0xff;
  digest[2] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[4]) << 0;
  l |= itoa64_to_int(buf[5]) << 6;
  l |= itoa64_to_int(buf[6]) << 12;
  l |= itoa64_to_int(buf[7]) << 18;

  digest[3] = (l >> 0) & 0xff;
  digest[4] = (l >> 8) & 0xff;
  digest[5] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[8]) << 0;
  l |= itoa64_to_int(buf[9]) << 6;
  l |= itoa64_to_int(buf[10]) << 12;
  l |= itoa64_to_int(buf[11]) << 18;

  digest[6] = (l >> 0) & 0xff;
  digest[7] = (l >> 8) & 0xff;
  digest[8] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[12]) << 0;
  l |= itoa64_to_int(buf[13]) << 6;
  l |= itoa64_to_int(buf[14]) << 12;
  l |= itoa64_to_int(buf[15]) << 18;

  digest[9] = (l >> 0) & 0xff;
  digest[10] = (l >> 8) & 0xff;
  digest[11] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[16]) << 0;
  l |= itoa64_to_int(buf[17]) << 6;
  l |= itoa64_to_int(buf[18]) << 12;
  l |= itoa64_to_int(buf[19]) << 18;

  digest[12] = (l >> 0) & 0xff;
  digest[13] = (l >> 8) & 0xff;
  digest[14] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[20]) << 0;
  l |= itoa64_to_int(buf[21]) << 6;
  l |= itoa64_to_int(buf[22]) << 12;
  l |= itoa64_to_int(buf[23]) << 18;

  digest[15] = (l >> 0) & 0xff;
  digest[16] = (l >> 8) & 0xff;
  digest[17] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[24]) << 0;
  l |= itoa64_to_int(buf[25]) << 6;
  l |= itoa64_to_int(buf[26]) << 12;
  l |= itoa64_to_int(buf[27]) << 18;

  digest[18] = (l >> 0) & 0xff;
  digest[19] = (l >> 8) & 0xff;
  digest[20] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[28]) << 0;
  l |= itoa64_to_int(buf[29]) << 6;
  l |= itoa64_to_int(buf[30]) << 12;
  l |= itoa64_to_int(buf[31]) << 18;

  digest[21] = (l >> 0) & 0xff;
  digest[22] = (l >> 8) & 0xff;
  digest[23] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[32]) << 0;
  l |= itoa64_to_int(buf[33]) << 6;
  l |= itoa64_to_int(buf[34]) << 12;
  l |= itoa64_to_int(buf[35]) << 18;

  digest[24] = (l >> 0) & 0xff;
  digest[25] = (l >> 8) & 0xff;
  digest[26] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[36]) << 0;
  l |= itoa64_to_int(buf[37]) << 6;
  l |= itoa64_to_int(buf[38]) << 12;
  l |= itoa64_to_int(buf[39]) << 18;

  digest[27] = (l >> 0) & 0xff;
  digest[28] = (l >> 8) & 0xff;
  digest[29] = (l >> 16) & 0xff;

  l = itoa64_to_int(buf[40]) << 0;
  l |= itoa64_to_int(buf[41]) << 6;
  l |= itoa64_to_int(buf[42]) << 12;
  l |= itoa64_to_int(buf[43]) << 18;

  digest[30] = (l >> 0) & 0xff;
  digest[31] = (l >> 8) & 0xff;
  digest[32] = (l >> 16) & 0xff;

  digest[33] = 0;
  digest[34] = 0;
  digest[35] = 0;
  digest[36] = 0;
  digest[37] = 0;
  digest[38] = 0;
  digest[39] = 0;
  digest[40] = 0;
  digest[41] = 0;
  digest[42] = 0;
  digest[43] = 0;
  digest[44] = 0;
  digest[45] = 0;
  digest[46] = 0;
  digest[47] = 0;
  digest[48] = 0;
  digest[49] = 0;
  digest[50] = 0;
  digest[51] = 0;
  digest[52] = 0;
  digest[53] = 0;
  digest[54] = 0;
  digest[55] = 0;
  digest[56] = 0;
  digest[57] = 0;
  digest[58] = 0;
  digest[59] = 0;
  digest[60] = 0;
  digest[61] = 0;
  digest[62] = 0;
  digest[63] = 0;
}

void drupal7_encode(u8 digest[64], u8 buf[43])
{
  int l;

  l = (digest[0] << 0) | (digest[1] << 8) | (digest[2] << 16);

  buf[0] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[1] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[2] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[3] = int_to_itoa64(l & 0x3f);

  l = (digest[3] << 0) | (digest[4] << 8) | (digest[5] << 16);

  buf[4] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[5] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[6] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[7] = int_to_itoa64(l & 0x3f);

  l = (digest[6] << 0) | (digest[7] << 8) | (digest[8] << 16);

  buf[8] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[9] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[10] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[11] = int_to_itoa64(l & 0x3f);

  l = (digest[9] << 0) | (digest[10] << 8) | (digest[11] << 16);

  buf[12] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[13] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[14] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[15] = int_to_itoa64(l & 0x3f);

  l = (digest[12] << 0) | (digest[13] << 8) | (digest[14] << 16);

  buf[16] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[17] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[18] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[19] = int_to_itoa64(l & 0x3f);

  l = (digest[15] << 0) | (digest[16] << 8) | (digest[17] << 16);

  buf[20] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[21] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[22] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[23] = int_to_itoa64(l & 0x3f);

  l = (digest[18] << 0) | (digest[19] << 8) | (digest[20] << 16);

  buf[24] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[25] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[26] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[27] = int_to_itoa64(l & 0x3f);

  l = (digest[21] << 0) | (digest[22] << 8) | (digest[23] << 16);

  buf[28] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[29] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[30] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[31] = int_to_itoa64(l & 0x3f);

  l = (digest[24] << 0) | (digest[25] << 8) | (digest[26] << 16);

  buf[32] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[33] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[34] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[35] = int_to_itoa64(l & 0x3f);

  l = (digest[27] << 0) | (digest[28] << 8) | (digest[29] << 16);

  buf[36] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[37] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[38] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[39] = int_to_itoa64(l & 0x3f);

  l = (digest[30] << 0) | (digest[31] << 8) | (digest[32] << 16);

  buf[40] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[41] = int_to_itoa64(l & 0x3f); l >>= 6;
  buf[42] = int_to_itoa64(l & 0x3f); l >>= 6;
  //buf[43] = int_to_itoa64 (l & 0x3f);
}

#include <tty.h>
#include <logfile.h>

/**
 * system
 */

#if F_SETLKW
void lock_file(FILE *fp)
{
  struct flock lock;

  memset(&lock, 0, sizeof(struct flock));

  lock.l_type = F_WRLCK;
  while (fcntl(fileno(fp), F_SETLKW, &lock))
  {
    if (errno != EINTR)
    {
      log_error("ERROR: Failed acquiring write lock: %s", strerror(errno));

      exit(-1);
    }
  }
}

void unlock_file(FILE *fp)
{
  struct flock lock;

  memset(&lock, 0, sizeof(struct flock));

  lock.l_type = F_UNLCK;
  fcntl(fileno(fp), F_SETLK, &lock);
}
#endif // F_SETLKW

#ifdef WIN
void fsync(int fd)
{
  HANDLE h = (HANDLE)_get_osfhandle(fd);

  FlushFileBuffers(h);
}
#endif

/**
 * thermal
 */

#ifdef HAVE_HWMON

int get_adapters_num_adl(void *adl, int *iNumberAdapters)
{
  if (hm_ADL_Adapter_NumberOfAdapters_Get((ADL_PTR *)adl, iNumberAdapters) != ADL_OK) return -1;

  if (iNumberAdapters == 0)
  {
    log_info("WARN: No ADL adapters found.");

    return -1;
  }

  return 0;
}

/*
int hm_show_performance_level (HM_LIB hm_dll, int iAdapterIndex)
{
  ADLODPerformanceLevels *lpOdPerformanceLevels = NULL;
  ADLODParameters lpOdParameters;

  lpOdParameters.iSize = sizeof (ADLODParameters);
  size_t plevels_size = 0;

  if (hm_ADL_Overdrive_ODParameters_Get (hm_dll, iAdapterIndex, &lpOdParameters) != ADL_OK) return -1;

  log_info ("[DEBUG] %s, adapter %d performance level (%d) : %s %s",
          __func__, iAdapterIndex,
          lpOdParameters.iNumberOfPerformanceLevels,
          (lpOdParameters.iActivityReportingSupported) ? "activity reporting" : "",
          (lpOdParameters.iDiscretePerformanceLevels) ? "discrete performance levels" : "performance ranges");

  plevels_size = sizeof (ADLODPerformanceLevels) + sizeof (ADLODPerformanceLevel) * (lpOdParameters.iNumberOfPerformanceLevels - 1);

  lpOdPerformanceLevels = (ADLODPerformanceLevels *) mymalloc (plevels_size);

  lpOdPerformanceLevels->iSize = sizeof (ADLODPerformanceLevels) + sizeof (ADLODPerformanceLevel) * (lpOdParameters.iNumberOfPerformanceLevels - 1);

  if (hm_ADL_Overdrive_ODPerformanceLevels_Get (hm_dll, iAdapterIndex, 0, lpOdPerformanceLevels) != ADL_OK) return -1;

  for (int j = 0; j < lpOdParameters.iNumberOfPerformanceLevels; j++)
    log_info ("[DEBUG] %s, adapter %d, level %d : engine %d, memory %d, voltage: %d",
    __func__, iAdapterIndex, j,
    lpOdPerformanceLevels->aLevels[j].iEngineClock / 100, lpOdPerformanceLevels->aLevels[j].iMemoryClock / 100, lpOdPerformanceLevels->aLevels[j].iVddc);

  myfree (lpOdPerformanceLevels);

  return 0;
}
*/

LPAdapterInfo hm_get_adapter_info_adl(void *adl, int iNumberAdapters)
{
  size_t AdapterInfoSize = iNumberAdapters * sizeof(AdapterInfo);

  LPAdapterInfo lpAdapterInfo = (LPAdapterInfo)mymalloc(AdapterInfoSize);

  if (hm_ADL_Adapter_AdapterInfo_Get((ADL_PTR *)adl, lpAdapterInfo, AdapterInfoSize) != ADL_OK) return NULL;

  return lpAdapterInfo;
}

int hm_get_adapter_index_nvapi(HM_ADAPTER_NVAPI nvapiGPUHandle[DEVICES_MAX])
{
  NvU32 pGpuCount;

  if (hm_NvAPI_EnumPhysicalGPUs(data.hm_nvapi, nvapiGPUHandle, &pGpuCount) != NVAPI_OK) return 0;

  if (pGpuCount == 0)
  {
    log_info("WARN: No NvAPI adapters found");

    return 0;
  }

  return (pGpuCount);
}

int hm_get_adapter_index_nvml(HM_ADAPTER_NVML nvmlGPUHandle[DEVICES_MAX])
{
  int pGpuCount = 0;

  for (uint i = 0; i < DEVICES_MAX; i++)
  {
    if (hm_NVML_nvmlDeviceGetHandleByIndex(data.hm_nvml, 1, i, &nvmlGPUHandle[i]) != NVML_SUCCESS) break;

    // can be used to determine if the device by index matches the cuda device by index
    // char name[100]; memset (name, 0, sizeof (name));
    // hm_NVML_nvmlDeviceGetName (data.hm_nvml, nvGPUHandle[i], name, sizeof (name) - 1);

    pGpuCount++;
  }

  if (pGpuCount == 0)
  {
    log_info("WARN: No NVML adapters found");

    return 0;
  }

  return (pGpuCount);
}

/*
//
// does not help at all, since ADL does not assign different bus id, device id when we have multi GPU setups
//

int hm_get_opencl_device_index (hm_attrs_t *hm_device, uint num_adl_adapters, int bus_num, int dev_num)
{
  u32 idx = -1;

  for (uint i = 0; i < num_adl_adapters; i++)
  {
    int opencl_bus_num = hm_device[i].busid;
    int opencl_dev_num = hm_device[i].devid;

    if ((opencl_bus_num == bus_num) && (opencl_dev_num == dev_num))
    {
      idx = i;

      break;
    }
  }

  if (idx >= DEVICES_MAX) return -1;

  return idx;
}

void hm_get_opencl_busid_devid (hm_attrs_t *hm_device, uint opencl_num_devices, cl_device_id *devices)
{
  for (uint i = 0; i < opencl_num_devices; i++)
  {
    cl_device_topology_amd device_topology;

    hc_clGetDeviceInfo (devices[i], CL_DEVICE_TOPOLOGY_AMD, sizeof (device_topology), &device_topology, NULL);

    hm_device[i].busid = device_topology.pcie.bus;
    hm_device[i].devid = device_topology.pcie.device;
  }
}
*/

void hm_sort_adl_adapters_by_busid_devid(u32 *valid_adl_device_list, int num_adl_adapters, LPAdapterInfo lpAdapterInfo)
{
  // basically bubble sort

  for (int i = 0; i < num_adl_adapters; i++)
  {
    for (int j = 0; j < num_adl_adapters - 1; j++)
    {
      // get info of adapter [x]

      u32 adapter_index_x = valid_adl_device_list[j];
      AdapterInfo info_x = lpAdapterInfo[adapter_index_x];

      u32 bus_num_x = info_x.iBusNumber;
      u32 dev_num_x = info_x.iDeviceNumber;

      // get info of adapter [y]

      u32 adapter_index_y = valid_adl_device_list[j + 1];
      AdapterInfo info_y = lpAdapterInfo[adapter_index_y];

      u32 bus_num_y = info_y.iBusNumber;
      u32 dev_num_y = info_y.iDeviceNumber;

      uint need_swap = 0;

      if (bus_num_y < bus_num_x)
      {
        need_swap = 1;
      }
      else if (bus_num_y == bus_num_x)
      {
        if (dev_num_y < dev_num_x)
        {
          need_swap = 1;
        }
      }

      if (need_swap == 1)
      {
        u32 temp = valid_adl_device_list[j + 1];

        valid_adl_device_list[j + 1] = valid_adl_device_list[j];
        valid_adl_device_list[j + 0] = temp;
      }
    }
  }
}

u32 *hm_get_list_valid_adl_adapters(int iNumberAdapters, int *num_adl_adapters, LPAdapterInfo lpAdapterInfo)
{
  *num_adl_adapters = 0;

  u32 *adl_adapters = NULL;

  int *bus_numbers = NULL;
  int *device_numbers = NULL;

  for (int i = 0; i < iNumberAdapters; i++)
  {
    AdapterInfo info = lpAdapterInfo[i];

    if (strlen(info.strUDID) < 1) continue;

#ifdef WIN
    if (info.iVendorID != 1002) continue;
#else
    if (info.iVendorID != 0x1002) continue;
#endif

    if (info.iBusNumber < 0) continue;
    if (info.iDeviceNumber < 0) continue;

    int found = 0;

    for (int pos = 0; pos < *num_adl_adapters; pos++)
    {
      if ((bus_numbers[pos] == info.iBusNumber) && (device_numbers[pos] == info.iDeviceNumber))
      {
        found = 1;
        break;
      }
    }

    if (found) continue;

    // add it to the list

    adl_adapters = (u32 *)myrealloc(adl_adapters, (*num_adl_adapters) * sizeof(int), sizeof(int));

    adl_adapters[*num_adl_adapters] = i;

    // rest is just bookkeeping

    bus_numbers = (int*)myrealloc(bus_numbers, (*num_adl_adapters) * sizeof(int), sizeof(int));
    device_numbers = (int*)myrealloc(device_numbers, (*num_adl_adapters) * sizeof(int), sizeof(int));

    bus_numbers[*num_adl_adapters] = info.iBusNumber;
    device_numbers[*num_adl_adapters] = info.iDeviceNumber;

    (*num_adl_adapters)++;
  }

  myfree(bus_numbers);
  myfree(device_numbers);

  // sort the list by increasing bus id, device id number

  hm_sort_adl_adapters_by_busid_devid(adl_adapters, *num_adl_adapters, lpAdapterInfo);

  return adl_adapters;
}

int hm_check_fanspeed_control(void *adl, hm_attrs_t *hm_device, u32 *valid_adl_device_list, int num_adl_adapters, LPAdapterInfo lpAdapterInfo)
{
  // loop through all valid devices

  for (int i = 0; i < num_adl_adapters; i++)
  {
    u32 adapter_index = valid_adl_device_list[i];

    // get AdapterInfo

    AdapterInfo info = lpAdapterInfo[adapter_index];

    // unfortunately this doesn't work since bus id and dev id are not unique
    // int opencl_device_index = hm_get_opencl_device_index (hm_device, num_adl_adapters, info.iBusNumber, info.iDeviceNumber);
    // if (opencl_device_index == -1) continue;

    int opencl_device_index = i;

    // if (hm_show_performance_level (adl, info.iAdapterIndex) != 0) return -1;

    // get fanspeed info

    if (hm_device[opencl_device_index].od_version == 5)
    {
      ADLFanSpeedInfo FanSpeedInfo;

      memset(&FanSpeedInfo, 0, sizeof(ADLFanSpeedInfo));

      FanSpeedInfo.iSize = sizeof(ADLFanSpeedInfo);

      if (hm_ADL_Overdrive5_FanSpeedInfo_Get(adl, info.iAdapterIndex, 0, &FanSpeedInfo) != ADL_OK) return -1;

      // check read and write capability in fanspeedinfo

      if ((FanSpeedInfo.iFlags & ADL_DL_FANCTRL_SUPPORTS_PERCENT_READ) &&
        (FanSpeedInfo.iFlags & ADL_DL_FANCTRL_SUPPORTS_PERCENT_WRITE))
      {
        hm_device[opencl_device_index].fan_get_supported = 1;
      }
      else
      {
        hm_device[opencl_device_index].fan_get_supported = 0;
      }
    }
    else // od_version == 6
    {
      ADLOD6FanSpeedInfo faninfo;

      memset(&faninfo, 0, sizeof(faninfo));

      if (hm_ADL_Overdrive6_FanSpeed_Get(adl, info.iAdapterIndex, &faninfo) != ADL_OK) return -1;

      // check read capability in fanspeedinfo

      if (faninfo.iSpeedType & ADL_OD6_FANSPEED_TYPE_PERCENT)
      {
        hm_device[opencl_device_index].fan_get_supported = 1;
      }
      else
      {
        hm_device[opencl_device_index].fan_get_supported = 0;
      }
    }
  }

  return 0;
}

int hm_get_overdrive_version(void *adl, hm_attrs_t *hm_device, u32 *valid_adl_device_list, int num_adl_adapters, LPAdapterInfo lpAdapterInfo)
{
  for (int i = 0; i < num_adl_adapters; i++)
  {
    u32 adapter_index = valid_adl_device_list[i];

    // get AdapterInfo

    AdapterInfo info = lpAdapterInfo[adapter_index];

    // get overdrive version

    int od_supported = 0;
    int od_enabled = 0;
    int od_version = 0;

    if (hm_ADL_Overdrive_Caps(adl, info.iAdapterIndex, &od_supported, &od_enabled, &od_version) != ADL_OK) return -1;

    // store the overdrive version in hm_device

    // unfortunately this doesn't work since bus id and dev id are not unique
    // int opencl_device_index = hm_get_opencl_device_index (hm_device, num_adl_adapters, info.iBusNumber, info.iDeviceNumber);
    // if (opencl_device_index == -1) continue;

    int opencl_device_index = i;

    hm_device[opencl_device_index].od_version = od_version;
  }

  return 0;
}

int hm_get_adapter_index_adl(hm_attrs_t *hm_device, u32 *valid_adl_device_list, int num_adl_adapters, LPAdapterInfo lpAdapterInfo)
{
  for (int i = 0; i < num_adl_adapters; i++)
  {
    u32 adapter_index = valid_adl_device_list[i];

    // get AdapterInfo

    AdapterInfo info = lpAdapterInfo[adapter_index];

    // store the iAdapterIndex in hm_device

    // unfortunately this doesn't work since bus id and dev id are not unique
    // int opencl_device_index = hm_get_opencl_device_index (hm_device, num_adl_adapters, info.iBusNumber, info.iDeviceNumber);
    // if (opencl_device_index == -1) continue;

    int opencl_device_index = i;

    hm_device[opencl_device_index].adl = info.iAdapterIndex;
  }

  return num_adl_adapters;
}

int hm_get_threshold_slowdown_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
  {
    if (data.hm_adl)
    {
      if (data.hm_device[device_id].od_version == 5)
      {

      }
      else if (data.hm_device[device_id].od_version == 6)
      {
        int CurrentValue = 0;
        int DefaultValue = 0;

        if (hm_ADL_Overdrive6_TargetTemperatureData_Get(data.hm_adl, data.hm_device[device_id].adl, &CurrentValue, &DefaultValue) != ADL_OK) return -1;

        // the return value has never been tested since hm_ADL_Overdrive6_TargetTemperatureData_Get() never worked on any system. expect problems.

        return DefaultValue;
      }
    }
  }

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
  {
    int target = 0;

    if (hm_NVML_nvmlDeviceGetTemperatureThreshold(data.hm_nvml, 1, data.hm_device[device_id].nvml, NVML_TEMPERATURE_THRESHOLD_SLOWDOWN, (unsigned int *)&target) != NVML_SUCCESS) return -1;

    return target;
  }

  return -1;
}

int hm_get_threshold_shutdown_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
  {
    if (data.hm_adl)
    {
      if (data.hm_device[device_id].od_version == 5)
      {

      }
      else if (data.hm_device[device_id].od_version == 6)
      {

      }
    }
  }

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
  {
    int target = 0;

    if (hm_NVML_nvmlDeviceGetTemperatureThreshold(data.hm_nvml, 1, data.hm_device[device_id].nvml, NVML_TEMPERATURE_THRESHOLD_SHUTDOWN, (unsigned int *)&target) != NVML_SUCCESS) return -1;

    return target;
  }

  return -1;
}

int hm_get_temperature_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
  {
    if (data.hm_adl)
    {
      if (data.hm_device[device_id].od_version == 5)
      {
        ADLTemperature Temperature;

        Temperature.iSize = sizeof(ADLTemperature);

        if (hm_ADL_Overdrive5_Temperature_Get(data.hm_adl, data.hm_device[device_id].adl, 0, &Temperature) != ADL_OK) return -1;

        return Temperature.iTemperature / 1000;
      }
      else if (data.hm_device[device_id].od_version == 6)
      {
        int Temperature = 0;

        if (hm_ADL_Overdrive6_Temperature_Get(data.hm_adl, data.hm_device[device_id].adl, &Temperature) != ADL_OK) return -1;

        return Temperature / 1000;
      }
    }
  }

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
  {
    int temperature = 0;

    if (hm_NVML_nvmlDeviceGetTemperature(data.hm_nvml, 1, data.hm_device[device_id].nvml, NVML_TEMPERATURE_GPU, (uint *)&temperature) != NVML_SUCCESS) return -1;

    return temperature;
  }

  return -1;
}

int hm_get_fanpolicy_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.hm_device[device_id].fan_get_supported == 1)
  {
    if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
    {
      if (data.hm_adl)
      {
        if (data.hm_device[device_id].od_version == 5)
        {
          ADLFanSpeedValue lpFanSpeedValue;

          memset(&lpFanSpeedValue, 0, sizeof(lpFanSpeedValue));

          lpFanSpeedValue.iSize = sizeof(lpFanSpeedValue);
          lpFanSpeedValue.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_PERCENT;

          if (hm_ADL_Overdrive5_FanSpeed_Get(data.hm_adl, data.hm_device[device_id].adl, 0, &lpFanSpeedValue) != ADL_OK) return -1;

          return (lpFanSpeedValue.iFanSpeed & ADL_DL_FANCTRL_FLAG_USER_DEFINED_SPEED) ? 0 : 1;
        }
        else // od_version == 6
        {
          return 1;
        }
      }
    }

    if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
    {
      return 1;
    }
  }

  return -1;
}

int hm_get_fanspeed_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.hm_device[device_id].fan_get_supported == 1)
  {
    if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
    {
      if (data.hm_adl)
      {
        if (data.hm_device[device_id].od_version == 5)
        {
          ADLFanSpeedValue lpFanSpeedValue;

          memset(&lpFanSpeedValue, 0, sizeof(lpFanSpeedValue));

          lpFanSpeedValue.iSize = sizeof(lpFanSpeedValue);
          lpFanSpeedValue.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_PERCENT;
          lpFanSpeedValue.iFlags = ADL_DL_FANCTRL_FLAG_USER_DEFINED_SPEED;

          if (hm_ADL_Overdrive5_FanSpeed_Get(data.hm_adl, data.hm_device[device_id].adl, 0, &lpFanSpeedValue) != ADL_OK) return -1;

          return lpFanSpeedValue.iFanSpeed;
        }
        else // od_version == 6
        {
          ADLOD6FanSpeedInfo faninfo;

          memset(&faninfo, 0, sizeof(faninfo));

          if (hm_ADL_Overdrive6_FanSpeed_Get(data.hm_adl, data.hm_device[device_id].adl, &faninfo) != ADL_OK) return -1;

          return faninfo.iFanSpeedPercent;
        }
      }
    }

    if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
    {
      int speed = 0;

      if (hm_NVML_nvmlDeviceGetFanSpeed(data.hm_nvml, 0, data.hm_device[device_id].nvml, (uint *)&speed) != NVML_SUCCESS) return -1;

      return speed;
    }
  }

  return -1;
}

int hm_get_buslanes_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
  {
    if (data.hm_adl)
    {
      ADLPMActivity PMActivity;

      PMActivity.iSize = sizeof(ADLPMActivity);

      if (hm_ADL_Overdrive_CurrentActivity_Get(data.hm_adl, data.hm_device[device_id].adl, &PMActivity) != ADL_OK) return -1;

      return PMActivity.iCurrentBusLanes;
    }
  }

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
  {
    unsigned int currLinkWidth;

    if (hm_NVML_nvmlDeviceGetCurrPcieLinkWidth(data.hm_nvml, 1, data.hm_device[device_id].nvml, &currLinkWidth) != NVML_SUCCESS) return -1;

    return currLinkWidth;
  }

  return -1;
}

int hm_get_utilization_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
  {
    if (data.hm_adl)
    {
      ADLPMActivity PMActivity;

      PMActivity.iSize = sizeof(ADLPMActivity);

      if (hm_ADL_Overdrive_CurrentActivity_Get(data.hm_adl, data.hm_device[device_id].adl, &PMActivity) != ADL_OK) return -1;

      return PMActivity.iActivityPercent;
    }
  }

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
  {
    nvmlUtilization_t utilization;

    if (hm_NVML_nvmlDeviceGetUtilizationRates(data.hm_nvml, 1, data.hm_device[device_id].nvml, &utilization) != NVML_SUCCESS) return -1;

    return utilization.gpu;
  }

  return -1;
}

int hm_get_memoryspeed_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
  {
    if (data.hm_adl)
    {
      ADLPMActivity PMActivity;

      PMActivity.iSize = sizeof(ADLPMActivity);

      if (hm_ADL_Overdrive_CurrentActivity_Get(data.hm_adl, data.hm_device[device_id].adl, &PMActivity) != ADL_OK) return -1;

      return PMActivity.iMemoryClock / 100;
    }
  }

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
  {
    unsigned int clock;

    if (hm_NVML_nvmlDeviceGetClockInfo(data.hm_nvml, 1, data.hm_device[device_id].nvml, NVML_CLOCK_MEM, &clock) != NVML_SUCCESS) return -1;

    return clock;
  }

  return -1;
}

int hm_get_corespeed_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
  {
    if (data.hm_adl)
    {
      ADLPMActivity PMActivity;

      PMActivity.iSize = sizeof(ADLPMActivity);

      if (hm_ADL_Overdrive_CurrentActivity_Get(data.hm_adl, data.hm_device[device_id].adl, &PMActivity) != ADL_OK) return -1;

      return PMActivity.iEngineClock / 100;
    }
  }

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
  {
    unsigned int clock;

    if (hm_NVML_nvmlDeviceGetClockInfo(data.hm_nvml, 1, data.hm_device[device_id].nvml, NVML_CLOCK_SM, &clock) != NVML_SUCCESS) return -1;

    return clock;
  }

  return -1;
}

int hm_get_throttle_with_device_id(const uint device_id)
{
  if ((data.devices_param[device_id].device_type & CL_DEVICE_TYPE_GPU) == 0) return -1;

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_AMD)
  {

  }

  if (data.devices_param[device_id].device_vendor_id == VENDOR_ID_NV)
  {
    unsigned long long clocksThrottleReasons = 0;
    unsigned long long supportedThrottleReasons = 0;

    if (hm_NVML_nvmlDeviceGetCurrentClocksThrottleReasons(data.hm_nvml, 1, data.hm_device[device_id].nvml, &clocksThrottleReasons) != NVML_SUCCESS) return -1;
    if (hm_NVML_nvmlDeviceGetSupportedClocksThrottleReasons(data.hm_nvml, 1, data.hm_device[device_id].nvml, &supportedThrottleReasons) != NVML_SUCCESS) return -1;

    clocksThrottleReasons &= supportedThrottleReasons;
    clocksThrottleReasons &= ~nvmlClocksThrottleReasonGpuIdle;
    clocksThrottleReasons &= ~nvmlClocksThrottleReasonApplicationsClocksSetting;
    clocksThrottleReasons &= ~nvmlClocksThrottleReasonUnknown;

    if (data.kernel_power_final)
    {
      clocksThrottleReasons &= ~nvmlClocksThrottleReasonHwSlowdown;
    }

    return (clocksThrottleReasons != nvmlClocksThrottleReasonNone);
  }

  return -1;
}

int hm_set_fanspeed_with_device_id_adl(const uint device_id, const int fanspeed, const int fanpolicy)
{
  if (data.hm_device[device_id].fan_set_supported == 1)
  {
    if (data.hm_adl)
    {
      if (fanpolicy == 1)
      {
        if (data.hm_device[device_id].od_version == 5)
        {
          ADLFanSpeedValue lpFanSpeedValue;

          memset(&lpFanSpeedValue, 0, sizeof(lpFanSpeedValue));

          lpFanSpeedValue.iSize = sizeof(lpFanSpeedValue);
          lpFanSpeedValue.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_PERCENT;
          lpFanSpeedValue.iFlags = ADL_DL_FANCTRL_FLAG_USER_DEFINED_SPEED;
          lpFanSpeedValue.iFanSpeed = fanspeed;

          if (hm_ADL_Overdrive5_FanSpeed_Set(data.hm_adl, data.hm_device[device_id].adl, 0, &lpFanSpeedValue) != ADL_OK) return -1;

          return 0;
        }
        else // od_version == 6
        {
          ADLOD6FanSpeedValue fan_speed_value;

          memset(&fan_speed_value, 0, sizeof(fan_speed_value));

          fan_speed_value.iSpeedType = ADL_OD6_FANSPEED_TYPE_PERCENT;
          fan_speed_value.iFanSpeed = fanspeed;

          if (hm_ADL_Overdrive6_FanSpeed_Set(data.hm_adl, data.hm_device[device_id].adl, &fan_speed_value) != ADL_OK) return -1;

          return 0;
        }
      }
      else
      {
        if (data.hm_device[device_id].od_version == 5)
        {
          if (hm_ADL_Overdrive5_FanSpeedToDefault_Set(data.hm_adl, data.hm_device[device_id].adl, 0) != ADL_OK) return -1;

          return 0;
        }
        else // od_version == 6
        {
          if (hm_ADL_Overdrive6_FanSpeed_Reset(data.hm_adl, data.hm_device[device_id].adl) != ADL_OK) return -1;

          return 0;
        }
      }
    }
  }

  return -1;
}

int hm_set_fanspeed_with_device_id_nvapi(const uint device_id, const int fanspeed, const int fanpolicy)
{
  if (data.hm_device[device_id].fan_set_supported == 1)
  {
    if (data.hm_nvapi)
    {
      if (fanpolicy == 1)
      {
        NV_GPU_COOLER_LEVELS CoolerLevels;

        memset(&CoolerLevels, 0, sizeof(NV_GPU_COOLER_LEVELS));

        CoolerLevels.Version = GPU_COOLER_LEVELS_VER | sizeof(NV_GPU_COOLER_LEVELS);

        CoolerLevels.Levels[0].Level = fanspeed;
        CoolerLevels.Levels[0].Policy = 1;

        if (hm_NvAPI_GPU_SetCoolerLevels(data.hm_nvapi, data.hm_device[device_id].nvapi, 0, &CoolerLevels) != NVAPI_OK) return -1;

        return 0;
      }
      else
      {
        if (hm_NvAPI_GPU_RestoreCoolerSettings(data.hm_nvapi, data.hm_device[device_id].nvapi, 0) != NVAPI_OK) return -1;

        return 0;
      }
    }
  }

  return -1;
}

int hm_set_fanspeed_with_device_id_xnvctrl(const uint device_id, const int fanspeed)
{
  if (data.hm_device[device_id].fan_set_supported == 1)
  {
    if (data.hm_xnvctrl)
    {
      if (set_fan_speed_target(data.hm_xnvctrl, data.hm_device[device_id].xnvctrl, fanspeed) != 0) return -1;

      return 0;
    }
  }

  return -1;
}

#endif // HAVE_HWMON

/**
 * mixed shared functions
 */

void dump_hex(const u8 *s, const int sz)
{
  for (int i = 0; i < sz; i++)
  {
    log_info_nn("%02x ", s[i]);
  }

  log_info("");
}

void usage_mini_print(const char *progname)
{
  for (uint i = 0; USAGE_MINI[i] != NULL; i++) log_info(USAGE_MINI[i], progname);
}

void usage_big_print(const char *progname)
{
  for (uint i = 0; USAGE_BIG[i] != NULL; i++) log_info(USAGE_BIG[i], progname);
}

char *get_exec_path()
{
  int exec_path_len = 1024;

  char *exec_path = (char *)mymalloc(exec_path_len);

#ifdef __linux__

  char tmp[32] = { 0 };

  snprintf(tmp, sizeof(tmp) - 1, "/proc/%d/exe", getpid());

  const int len = readlink(tmp, exec_path, exec_path_len - 1);

#elif WIN

  const int len = GetModuleFileName(NULL, exec_path, exec_path_len - 1);

#elif __APPLE__

  uint size = exec_path_len;

  if (_NSGetExecutablePath(exec_path, &size) != 0)
  {
    log_error("! executable path buffer too small\n");

    exit(-1);
  }

  const int len = strlen(exec_path);

#elif __FreeBSD__

#include <sys/sysctl.h>

  int mib[4];
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PATHNAME;
  mib[3] = -1;

  char tmp[32] = { 0 };

  size_t size = exec_path_len;
  sysctl(mib, 4, exec_path, &size, NULL, 0);

  const int len = readlink(tmp, exec_path, exec_path_len - 1);

#else
#error Your Operating System is not supported or detected
#endif

  exec_path[len] = 0;

  return exec_path;
}

char *get_install_dir(const char *progname)
{
  char *install_dir = mystrdup(progname);
  char *last_slash = NULL;

  if ((last_slash = strrchr(install_dir, '/')) != NULL)
  {
    *last_slash = 0;
  }
  else if ((last_slash = strrchr(install_dir, '\\')) != NULL)
  {
    *last_slash = 0;
  }
  else
  {
    install_dir[0] = '.';
    install_dir[1] = 0;
  }

  return (install_dir);
}

char *get_profile_dir(const char *homedir)
{
#define DOT_HASHCAT ".hashcat"

  size_t len = strlen(homedir) + 1 + strlen(DOT_HASHCAT) + 1;

  char *profile_dir = (char *)mymalloc(len + 1);

  snprintf(profile_dir, len, "%s/%s", homedir, DOT_HASHCAT);

  return profile_dir;
}

char *get_session_dir(const char *profile_dir)
{
#define SESSIONS_FOLDER "sessions"

  size_t len = strlen(profile_dir) + 1 + strlen(SESSIONS_FOLDER) + 1;

  char *session_dir = (char *)mymalloc(len + 1);

  snprintf(session_dir, len, "%s/%s", profile_dir, SESSIONS_FOLDER);

  return session_dir;
}

uint count_lines(FILE *fd)
{
  uint cnt = 0;

  char *buf = (char *)mymalloc(HCBUFSIZ + 1);

  char prev = '\n';

  while (!feof(fd))
  {
    size_t nread = fread(buf, sizeof(char), HCBUFSIZ, fd);

    if (nread < 1) continue;

    size_t i;

    for (i = 0; i < nread; i++)
    {
      if (prev == '\n') cnt++;

      prev = buf[i];
    }
  }

  myfree(buf);

  return cnt;
}

void truecrypt_crc32(const char *filename, u8 keytab[64])
{
  uint crc = ~0;

  FILE *fd = fopen(filename, "rb");

  if (fd == NULL)
  {
    log_error("%s: %s", filename, strerror(errno));

    exit(-1);
  }

#define MAX_KEY_SIZE (1024 * 1024)

  u8 *buf = (u8 *)mymalloc(MAX_KEY_SIZE + 1);

  int nread = fread(buf, sizeof(u8), MAX_KEY_SIZE, fd);

  fclose(fd);

  int kpos = 0;

  for (int fpos = 0; fpos < nread; fpos++)
  {
    crc = crc32tab[(crc ^ buf[fpos]) & 0xff] ^ (crc >> 8);

    keytab[kpos++] += (crc >> 24) & 0xff;
    keytab[kpos++] += (crc >> 16) & 0xff;
    keytab[kpos++] += (crc >> 8) & 0xff;
    keytab[kpos++] += (crc >> 0) & 0xff;

    if (kpos >= 64) kpos = 0;
  }

  myfree(buf);
}

#ifdef __APPLE__
int pthread_setaffinity_np(pthread_t thread, size_t cpu_size, cpu_set_t *cpu_set)
{
  int core;

  for (core = 0; core < (8 * (int)cpu_size); core++)
    if (CPU_ISSET(core, cpu_set)) break;

  thread_affinity_policy_data_t policy = { core };

  const int rc = thread_policy_set(pthread_mach_thread_np(thread), THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);

  if (data.quiet == 0)
  {
    if (rc != KERN_SUCCESS)
    {
      log_error("ERROR: %s : %d", "thread_policy_set()", rc);
    }
  }

  return rc;
}
#endif

void set_cpu_affinity(char *cpu_affinity)
{
#ifdef _WIN
  DWORD_PTR aff_mask = 0;
#elif __FreeBSD__
  cpuset_t cpuset;
  CPU_ZERO(&cpuset);
#elif _POSIX
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
#endif

  if (cpu_affinity)
  {
    char *devices = strdup(cpu_affinity);

    char *next = strtok(devices, ",");

    do
    {
      uint cpu_id = atoi(next);

      if (cpu_id == 0)
      {
#ifdef _WIN
        aff_mask = 0;
#elif _POSIX
        CPU_ZERO(&cpuset);
#endif

        break;
      }

      if (cpu_id > 32)
      {
        log_error("ERROR: Invalid cpu_id %u specified", cpu_id);

        exit(-1);
      }

#ifdef _WIN
      aff_mask |= 1u << (cpu_id - 1);
#elif _POSIX
      CPU_SET((cpu_id - 1), &cpuset);
#endif

    } while ((next = strtok(NULL, ",")) != NULL);

    free(devices);
  }

#ifdef _WIN
  SetProcessAffinityMask(GetCurrentProcess(), aff_mask);
  SetThreadAffinityMask(GetCurrentThread(), aff_mask);
#elif __FreeBSD__
  pthread_t thread = pthread_self();
  pthread_setaffinity_np(thread, sizeof(cpuset_t), &cpuset);
#elif _POSIX
  pthread_t thread = pthread_self();
  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
#endif
}

void *rulefind(const void *key, void *base, int nmemb, size_t size, int(*compar) (const void *, const void *))
{
  char *element, *end;

  end = (char *)base + nmemb * size;

  for (element = (char *)base; element < end; element += size)
    if (!compar(element, key))
      return element;

  return NULL;
}

void format_debug(char *debug_file, uint debug_mode, unsigned char *orig_plain_ptr, uint orig_plain_len, unsigned char *mod_plain_ptr, uint mod_plain_len, char *rule_buf, int rule_len)
{
  uint outfile_autohex = data.outfile_autohex;

  unsigned char *rule_ptr = (unsigned char *)rule_buf;

  FILE *debug_fp = NULL;

  if (debug_file != NULL)
  {
    debug_fp = fopen(debug_file, "ab");

    lock_file(debug_fp);
  }
  else
  {
    debug_fp = stderr;
  }

  if (debug_fp == NULL)
  {
    log_info("WARNING: Could not open debug-file for writing");
  }
  else
  {
    if ((debug_mode == 2) || (debug_mode == 3) || (debug_mode == 4))
    {
      format_plain(debug_fp, orig_plain_ptr, orig_plain_len, outfile_autohex);

      if ((debug_mode == 3) || (debug_mode == 4)) fputc(':', debug_fp);
    }

    fwrite(rule_ptr, rule_len, 1, debug_fp);

    if (debug_mode == 4)
    {
      fputc(':', debug_fp);

      format_plain(debug_fp, mod_plain_ptr, mod_plain_len, outfile_autohex);
    }

    fputc('\n', debug_fp);

    if (debug_file != NULL) fclose(debug_fp);
  }
}

void format_plain(FILE *fp, unsigned char *plain_ptr, uint plain_len, uint outfile_autohex)
{
  int needs_hexify = 0;

  if (outfile_autohex == 1)
  {
    for (uint i = 0; i < plain_len; i++)
    {
      if (plain_ptr[i] < 0x20)
      {
        needs_hexify = 1;

        break;
      }

      if (plain_ptr[i] > 0x7f)
      {
        needs_hexify = 1;

        break;
      }
    }
  }

  if (needs_hexify == 1)
  {
    fprintf(fp, "$HEX[");

    for (uint i = 0; i < plain_len; i++)
    {
      fprintf(fp, "%02x", plain_ptr[i]);
    }

    fprintf(fp, "]");
  }
  else
  {
    fwrite(plain_ptr, plain_len, 1, fp);
  }
}

void format_output(FILE *out_fp, char *out_buf, unsigned char *plain_ptr, const uint plain_len, const u64 crackpos, unsigned char *username, const uint user_len)
{
  uint outfile_format = data.outfile_format;

  char separator = data.separator;

  if (outfile_format & OUTFILE_FMT_HASH)
  {
    fprintf(out_fp, "%s", out_buf);

    if (outfile_format & (OUTFILE_FMT_PLAIN | OUTFILE_FMT_HEXPLAIN | OUTFILE_FMT_CRACKPOS))
    {
      fputc(separator, out_fp);
    }
  }
  else if (data.username)
  {
    if (username != NULL)
    {
      for (uint i = 0; i < user_len; i++)
      {
        fprintf(out_fp, "%c", username[i]);
      }

      if (outfile_format & (OUTFILE_FMT_PLAIN | OUTFILE_FMT_HEXPLAIN | OUTFILE_FMT_CRACKPOS))
      {
        fputc(separator, out_fp);
      }
    }
  }

  if (outfile_format & OUTFILE_FMT_PLAIN)
  {
    format_plain(out_fp, plain_ptr, plain_len, data.outfile_autohex);

    if (outfile_format & (OUTFILE_FMT_HEXPLAIN | OUTFILE_FMT_CRACKPOS))
    {
      fputc(separator, out_fp);
    }
  }

  if (outfile_format & OUTFILE_FMT_HEXPLAIN)
  {
    for (uint i = 0; i < plain_len; i++)
    {
      fprintf(out_fp, "%02x", plain_ptr[i]);
    }

    if (outfile_format & (OUTFILE_FMT_CRACKPOS))
    {
      fputc(separator, out_fp);
    }
  }

  if (outfile_format & OUTFILE_FMT_CRACKPOS)
  {
#ifdef _WIN
    __mingw_fprintf(out_fp, "%llu", crackpos);
#endif

#ifdef _POSIX
#ifdef __x86_64__
    fprintf(out_fp, "%lu", (unsigned long)crackpos);
#else
    fprintf(out_fp, "%llu", crackpos);
#endif
#endif
  }

  fputs(EOL, out_fp);
}

void handle_show_request(pot_t *pot, uint pot_cnt, char *input_buf, int input_len, hash_t *hashes_buf, int(*sort_by_pot) (const void *, const void *), FILE *out_fp)
{
  pot_t pot_key;

  pot_key.hash.salt = hashes_buf->salt;
  pot_key.hash.digest = hashes_buf->digest;

  pot_t *pot_ptr = (pot_t *)bsearch(&pot_key, pot, pot_cnt, sizeof(pot_t), sort_by_pot);

  if (pot_ptr)
  {
    log_info_nn("");

    input_buf[input_len] = 0;

    // user
    unsigned char *username = NULL;
    uint user_len = 0;

    if (data.username)
    {
      user_t *user = hashes_buf->hash_info->user;

      if (user)
      {
        username = (unsigned char *)(user->user_name);

        user_len = user->user_len;
      }
    }

    // do output the line
    format_output(out_fp, input_buf, (unsigned char *)pot_ptr->plain_buf, pot_ptr->plain_len, 0, username, user_len);
  }
}

#define LM_WEAK_HASH    "\x4e\xcf\x0d\x0c\x0a\xe2\xfb\xc1"
#define LM_MASKED_PLAIN "[notfound]"

void handle_show_request_lm(pot_t *pot, uint pot_cnt, char *input_buf, int input_len, hash_t *hash_left, hash_t *hash_right, int(*sort_by_pot) (const void *, const void *), FILE *out_fp)
{
  // left

  pot_t pot_left_key;

  pot_left_key.hash.salt = hash_left->salt;
  pot_left_key.hash.digest = hash_left->digest;

  pot_t *pot_left_ptr = (pot_t *)bsearch(&pot_left_key, pot, pot_cnt, sizeof(pot_t), sort_by_pot);

  // right

  uint weak_hash_found = 0;

  pot_t pot_right_key;

  pot_right_key.hash.salt = hash_right->salt;
  pot_right_key.hash.digest = hash_right->digest;

  pot_t *pot_right_ptr = (pot_t *)bsearch(&pot_right_key, pot, pot_cnt, sizeof(pot_t), sort_by_pot);

  if (pot_right_ptr == NULL)
  {
    // special case, if "weak hash"

    if (memcmp(hash_right->digest, LM_WEAK_HASH, 8) == 0)
    {
      weak_hash_found = 1;

      pot_right_ptr = (pot_t *)mycalloc(1, sizeof(pot_t));

      // in theory this is not needed, but we are paranoia:

      memset(pot_right_ptr->plain_buf, 0, sizeof(pot_right_ptr->plain_buf));
      pot_right_ptr->plain_len = 0;
    }
  }

  if ((pot_left_ptr == NULL) && (pot_right_ptr == NULL))
  {
    if (weak_hash_found == 1) myfree(pot_right_ptr); // this shouldn't happen at all: if weak_hash_found == 1, than pot_right_ptr is not NULL for sure

    return;
  }

  // at least one half was found:

  log_info_nn("");

  input_buf[input_len] = 0;

  // user

  unsigned char *username = NULL;
  uint user_len = 0;

  if (data.username)
  {
    user_t *user = hash_left->hash_info->user;

    if (user)
    {
      username = (unsigned char *)(user->user_name);

      user_len = user->user_len;
    }
  }

  // mask the part which was not found

  uint left_part_masked = 0;
  uint right_part_masked = 0;

  uint mask_plain_len = strlen(LM_MASKED_PLAIN);

  if (pot_left_ptr == NULL)
  {
    left_part_masked = 1;

    pot_left_ptr = (pot_t *)mycalloc(1, sizeof(pot_t));

    memset(pot_left_ptr->plain_buf, 0, sizeof(pot_left_ptr->plain_buf));

    memcpy(pot_left_ptr->plain_buf, LM_MASKED_PLAIN, mask_plain_len);
    pot_left_ptr->plain_len = mask_plain_len;
  }

  if (pot_right_ptr == NULL)
  {
    right_part_masked = 1;

    pot_right_ptr = (pot_t *)mycalloc(1, sizeof(pot_t));

    memset(pot_right_ptr->plain_buf, 0, sizeof(pot_right_ptr->plain_buf));

    memcpy(pot_right_ptr->plain_buf, LM_MASKED_PLAIN, mask_plain_len);
    pot_right_ptr->plain_len = mask_plain_len;
  }

  // create the pot_ptr out of pot_left_ptr and pot_right_ptr

  pot_t pot_ptr;

  pot_ptr.plain_len = pot_left_ptr->plain_len + pot_right_ptr->plain_len;

  memcpy(pot_ptr.plain_buf, pot_left_ptr->plain_buf, pot_left_ptr->plain_len);

  memcpy(pot_ptr.plain_buf + pot_left_ptr->plain_len, pot_right_ptr->plain_buf, pot_right_ptr->plain_len);

  // do output the line

  format_output(out_fp, input_buf, (unsigned char *)pot_ptr.plain_buf, pot_ptr.plain_len, 0, username, user_len);

  if (weak_hash_found == 1) myfree(pot_right_ptr);

  if (left_part_masked == 1) myfree(pot_left_ptr);
  if (right_part_masked == 1) myfree(pot_right_ptr);
}

void handle_left_request(pot_t *pot, uint pot_cnt, char *input_buf, int input_len, hash_t *hashes_buf, int(*sort_by_pot) (const void *, const void *), FILE *out_fp)
{
  pot_t pot_key;

  memcpy(&pot_key.hash, hashes_buf, sizeof(hash_t));

  pot_t *pot_ptr = (pot_t *)bsearch(&pot_key, pot, pot_cnt, sizeof(pot_t), sort_by_pot);

  if (pot_ptr == NULL)
  {
    log_info_nn("");

    input_buf[input_len] = 0;

    format_output(out_fp, input_buf, NULL, 0, 0, NULL, 0);
  }
}

void handle_left_request_lm(pot_t *pot, uint pot_cnt, char *input_buf, int input_len, hash_t *hash_left, hash_t *hash_right, int(*sort_by_pot) (const void *, const void *), FILE *out_fp)
{
  // left

  pot_t pot_left_key;

  memcpy(&pot_left_key.hash, hash_left, sizeof(hash_t));

  pot_t *pot_left_ptr = (pot_t *)bsearch(&pot_left_key, pot, pot_cnt, sizeof(pot_t), sort_by_pot);

  // right

  pot_t pot_right_key;

  memcpy(&pot_right_key.hash, hash_right, sizeof(hash_t));

  pot_t *pot_right_ptr = (pot_t *)bsearch(&pot_right_key, pot, pot_cnt, sizeof(pot_t), sort_by_pot);

  uint weak_hash_found = 0;

  if (pot_right_ptr == NULL)
  {
    // special case, if "weak hash"

    if (memcmp(hash_right->digest, LM_WEAK_HASH, 8) == 0)
    {
      weak_hash_found = 1;

      // we just need that pot_right_ptr is not a NULL pointer

      pot_right_ptr = (pot_t *)mycalloc(1, sizeof(pot_t));
    }
  }

  if ((pot_left_ptr != NULL) && (pot_right_ptr != NULL))
  {
    if (weak_hash_found == 1) myfree(pot_right_ptr);

    return;
  }

  // ... at least one part was not cracked

  log_info_nn("");

  input_buf[input_len] = 0;

  // only show the hash part which is still not cracked

  uint user_len = input_len - 32;

  char *hash_output = (char *)mymalloc(33);

  memcpy(hash_output, input_buf, input_len);

  if (pot_left_ptr != NULL)
  {
    // only show right part (because left part was already found)

    memcpy(hash_output + user_len, input_buf + user_len + 16, 16);

    hash_output[user_len + 16] = 0;
  }

  if (pot_right_ptr != NULL)
  {
    // only show left part (because right part was already found)

    memcpy(hash_output + user_len, input_buf + user_len, 16);

    hash_output[user_len + 16] = 0;
  }

  format_output(out_fp, hash_output, NULL, 0, 0, NULL, 0);

  myfree(hash_output);

  if (weak_hash_found == 1) myfree(pot_right_ptr);
}

uint setup_opencl_platforms_filter(char *opencl_platforms)
{
  uint opencl_platforms_filter = 0;

  if (opencl_platforms)
  {
    char *platforms = strdup(opencl_platforms);

    char *next = strtok(platforms, ",");

    do
    {
      int platform = atoi(next);

      if (platform < 1 || platform > 32)
      {
        log_error("ERROR: Invalid OpenCL platform %u specified", platform);

        exit(-1);
      }

      opencl_platforms_filter |= 1u << (platform - 1);

    } while ((next = strtok(NULL, ",")) != NULL);

    free(platforms);
  }
  else
  {
    opencl_platforms_filter = -1;
  }

  return opencl_platforms_filter;
}

u32 setup_devices_filter(char *opencl_devices)
{
  u32 devices_filter = 0;

  if (opencl_devices)
  {
    char *devices = strdup(opencl_devices);

    char *next = strtok(devices, ",");

    do
    {
      int device_id = atoi(next);

      if (device_id < 1 || device_id > 32)
      {
        log_error("ERROR: Invalid device_id %u specified", device_id);

        exit(-1);
      }

      devices_filter |= 1u << (device_id - 1);

    } while ((next = strtok(NULL, ",")) != NULL);

    free(devices);
  }
  else
  {
    devices_filter = -1;
  }

  return devices_filter;
}

cl_device_type setup_device_types_filter(char *opencl_device_types)
{
  cl_device_type device_types_filter = 0;

  if (opencl_device_types)
  {
    char *device_types = strdup(opencl_device_types);

    char *next = strtok(device_types, ",");

    do
    {
      int device_type = atoi(next);

      if (device_type < 1 || device_type > 3)
      {
        log_error("ERROR: Invalid device_type %u specified", device_type);

        exit(-1);
      }

      device_types_filter |= 1u << device_type;

    } while ((next = strtok(NULL, ",")) != NULL);

    free(device_types);
  }
  else
  {
    // Do not use CPU by default, this often reduces GPU performance because
    // the CPU is too busy to handle GPU synchronization

    device_types_filter = CL_DEVICE_TYPE_ALL & ~CL_DEVICE_TYPE_CPU;
  }

  return device_types_filter;
}

u32 get_random_num(const u32 min, const u32 max)
{
  if (min == max) return (min);

  return ((rand() % (max - min)) + min);
}

u32 mydivc32(const u32 dividend, const u32 divisor)
{
  u32 quotient = dividend / divisor;

  if (dividend % divisor) quotient++;

  return quotient;
}

u64 mydivc64(const u64 dividend, const u64 divisor)
{
  u64 quotient = dividend / divisor;

  if (dividend % divisor) quotient++;

  return quotient;
}

void format_timer_display(struct tm *tm, char *buf, size_t len)
{
  const char *time_entities_s[] = { "year",  "day",  "hour",  "min",  "sec" };
  const char *time_entities_m[] = { "years", "days", "hours", "mins", "secs" };

  if (tm->tm_year - 70)
  {
    char *time_entity1 = ((tm->tm_year - 70) == 1) ? (char *)time_entities_s[0] : (char *)time_entities_m[0];
    char *time_entity2 = (tm->tm_yday == 1) ? (char *)time_entities_s[1] : (char *)time_entities_m[1];

    snprintf(buf, len - 1, "%d %s, %d %s", tm->tm_year - 70, time_entity1, tm->tm_yday, time_entity2);
  }
  else if (tm->tm_yday)
  {
    char *time_entity1 = (tm->tm_yday == 1) ? (char *)time_entities_s[1] : (char *)time_entities_m[1];
    char *time_entity2 = (tm->tm_hour == 1) ? (char *)time_entities_s[2] : (char *)time_entities_m[2];

    snprintf(buf, len - 1, "%d %s, %d %s", tm->tm_yday, time_entity1, tm->tm_hour, time_entity2);
  }
  else if (tm->tm_hour)
  {
    char *time_entity1 = (tm->tm_hour == 1) ? (char *)time_entities_s[2] : (char *)time_entities_m[2];
    char *time_entity2 = (tm->tm_min == 1) ? (char *)time_entities_s[3] : (char *)time_entities_m[3];

    snprintf(buf, len - 1, "%d %s, %d %s", tm->tm_hour, time_entity1, tm->tm_min, time_entity2);
  }
  else if (tm->tm_min)
  {
    char *time_entity1 = (tm->tm_min == 1) ? (char *)time_entities_s[3] : (char *)time_entities_m[3];
    char *time_entity2 = (tm->tm_sec == 1) ? (char *)time_entities_s[4] : (char *)time_entities_m[4];

    snprintf(buf, len - 1, "%d %s, %d %s", tm->tm_min, time_entity1, tm->tm_sec, time_entity2);
  }
  else
  {
    char *time_entity1 = (tm->tm_sec == 1) ? (char *)time_entities_s[4] : (char *)time_entities_m[4];

    snprintf(buf, len - 1, "%d %s", tm->tm_sec, time_entity1);
  }
}

void format_speed_display(float val, char *buf, size_t len)
{
  if (val <= 0)
  {
    buf[0] = '0';
    buf[1] = ' ';
    buf[2] = 0;

    return;
  }

  char units[7] = { ' ', 'k', 'M', 'G', 'T', 'P', 'E' };

  uint level = 0;

  while (val > 99999)
  {
    val /= 1000;

    level++;
  }

  /* generate output */

  if (level == 0)
  {
    snprintf(buf, len - 1, "%.0f ", val);
  }
  else
  {
    snprintf(buf, len - 1, "%.1f %c", val, units[level]);
  }
}

void lowercase(u8 *buf, int len)
{
  for (int i = 0; i < len; i++) buf[i] = tolower(buf[i]);
}

void uppercase(u8 *buf, int len)
{
  for (int i = 0; i < len; i++) buf[i] = toupper(buf[i]);
}

int fgetl(FILE *fp, char *line_buf)
{
  int line_len = 0;

  while (!feof(fp))
  {
    const int c = fgetc(fp);

    if (c == EOF) break;

    line_buf[line_len] = (char)c;

    line_len++;

    if (line_len == HCBUFSIZ) line_len--;

    if (c == '\n') break;
  }

  if (line_len == 0) return 0;

  if (line_buf[line_len - 1] == '\n')
  {
    line_len--;

    line_buf[line_len] = 0;
  }

  if (line_len == 0) return 0;

  if (line_buf[line_len - 1] == '\r')
  {
    line_len--;

    line_buf[line_len] = 0;
  }

  return (line_len);
}

int in_superchop(char *buf)
{
  int len = strlen(buf);

  while (len)
  {
    if (buf[len - 1] == '\n')
    {
      len--;

      continue;
    }

    if (buf[len - 1] == '\r')
    {
      len--;

      continue;
    }

    break;
  }

  buf[len] = 0;

  return len;
}

char **scan_directory(const char *path)
{
  char *tmp_path = mystrdup(path);

  size_t tmp_path_len = strlen(tmp_path);

  while (tmp_path[tmp_path_len - 1] == '/' || tmp_path[tmp_path_len - 1] == '\\')
  {
    tmp_path[tmp_path_len - 1] = 0;

    tmp_path_len = strlen(tmp_path);
  }

  char **files = NULL;

  int num_files = 0;

  DIR *d = NULL;

  if ((d = opendir(tmp_path)) != NULL)
  {
#ifdef __APPLE__

    struct dirent e;

    for (;;)
    {
      memset(&e, 0, sizeof(e));

      struct dirent *de = NULL;

      if (readdir_r(d, &e, &de) != 0)
      {
        log_error("ERROR: readdir_r() failed");

        break;
      }

      if (de == NULL) break;

#else

    struct dirent *de;

    while ((de = readdir(d)) != NULL)
    {

#endif

      if ((strcmp(de->d_name, ".") == 0) || (strcmp(de->d_name, "..") == 0)) continue;

      int path_size = strlen(tmp_path) + 1 + strlen(de->d_name);

      char *path_file = (char *)mymalloc(path_size + 1);

      snprintf(path_file, path_size + 1, "%s/%s", tmp_path, de->d_name);

      path_file[path_size] = 0;

      DIR *d_test;

      if ((d_test = opendir(path_file)) != NULL)
      {
        closedir(d_test);

        myfree(path_file);
      }
      else
      {
        files = (char **)myrealloc(files, num_files * sizeof(char *), sizeof(char *));

        num_files++;

        files[num_files - 1] = path_file;
      }
    }

    closedir(d);
  }
  else if (errno == ENOTDIR)
  {
    files = (char **)myrealloc(files, num_files * sizeof(char *), sizeof(char *));

    num_files++;

    files[num_files - 1] = mystrdup(path);
  }

  files = (char **)myrealloc(files, num_files * sizeof(char *), sizeof(char *));

  num_files++;

  files[num_files - 1] = NULL;

  myfree(tmp_path);

  return (files);
}

int count_dictionaries(char **dictionary_files)
{
  if (dictionary_files == NULL) return 0;

  int cnt = 0;

  for (int d = 0; dictionary_files[d] != NULL; d++)
  {
    cnt++;
  }

  return (cnt);
}

char *stroptitype(const uint opti_type)
{
  switch (opti_type)
  {
  case OPTI_TYPE_ZERO_BYTE:         return ((char *)OPTI_STR_ZERO_BYTE);         break;
  case OPTI_TYPE_PRECOMPUTE_INIT:   return ((char *)OPTI_STR_PRECOMPUTE_INIT);   break;
  case OPTI_TYPE_PRECOMPUTE_MERKLE: return ((char *)OPTI_STR_PRECOMPUTE_MERKLE); break;
  case OPTI_TYPE_PRECOMPUTE_PERMUT: return ((char *)OPTI_STR_PRECOMPUTE_PERMUT); break;
  case OPTI_TYPE_MEET_IN_MIDDLE:    return ((char *)OPTI_STR_MEET_IN_MIDDLE);    break;
  case OPTI_TYPE_EARLY_SKIP:        return ((char *)OPTI_STR_EARLY_SKIP);        break;
  case OPTI_TYPE_NOT_SALTED:        return ((char *)OPTI_STR_NOT_SALTED);        break;
  case OPTI_TYPE_NOT_ITERATED:      return ((char *)OPTI_STR_NOT_ITERATED);      break;
  case OPTI_TYPE_PREPENDED_SALT:    return ((char *)OPTI_STR_PREPENDED_SALT);    break;
  case OPTI_TYPE_APPENDED_SALT:     return ((char *)OPTI_STR_APPENDED_SALT);     break;
  case OPTI_TYPE_SINGLE_HASH:       return ((char *)OPTI_STR_SINGLE_HASH);       break;
  case OPTI_TYPE_SINGLE_SALT:       return ((char *)OPTI_STR_SINGLE_SALT);       break;
  case OPTI_TYPE_BRUTE_FORCE:       return ((char *)OPTI_STR_BRUTE_FORCE);       break;
  case OPTI_TYPE_RAW_HASH:          return ((char *)OPTI_STR_RAW_HASH);          break;
  case OPTI_TYPE_SLOW_HASH_SIMD:    return ((char *)OPTI_STR_SLOW_HASH_SIMD);    break;
  case OPTI_TYPE_USES_BITS_8:       return ((char *)OPTI_STR_USES_BITS_8);       break;
  case OPTI_TYPE_USES_BITS_16:      return ((char *)OPTI_STR_USES_BITS_16);      break;
  case OPTI_TYPE_USES_BITS_32:      return ((char *)OPTI_STR_USES_BITS_32);      break;
  case OPTI_TYPE_USES_BITS_64:      return ((char *)OPTI_STR_USES_BITS_64);      break;
  }

  return (NULL);
}

char *strparser(const uint parser_status)
{
  switch (parser_status)
  {
  case PARSER_OK:                   return ((char *)PA_000); break;
  case PARSER_COMMENT:              return ((char *)PA_001); break;
  case PARSER_GLOBAL_ZERO:          return ((char *)PA_002); break;
  case PARSER_GLOBAL_LENGTH:        return ((char *)PA_003); break;
  case PARSER_HASH_LENGTH:          return ((char *)PA_004); break;
  case PARSER_HASH_VALUE:           return ((char *)PA_005); break;
  case PARSER_SALT_LENGTH:          return ((char *)PA_006); break;
  case PARSER_SALT_VALUE:           return ((char *)PA_007); break;
  case PARSER_SALT_ITERATION:       return ((char *)PA_008); break;
  case PARSER_SEPARATOR_UNMATCHED:  return ((char *)PA_009); break;
  case PARSER_SIGNATURE_UNMATCHED:  return ((char *)PA_010); break;
  case PARSER_HCCAP_FILE_SIZE:      return ((char *)PA_011); break;
  case PARSER_HCCAP_EAPOL_SIZE:     return ((char *)PA_012); break;
  case PARSER_PSAFE2_FILE_SIZE:     return ((char *)PA_013); break;
  case PARSER_PSAFE3_FILE_SIZE:     return ((char *)PA_014); break;
  case PARSER_TC_FILE_SIZE:         return ((char *)PA_015); break;
  case PARSER_SIP_AUTH_DIRECTIVE:   return ((char *)PA_016); break;
  }

  return ((char *)PA_255);
}

char *strhashtype(const uint hash_mode)
{
  switch (hash_mode)
  {
  case     0: return ((char *)HT_00000); break;
  case    10: return ((char *)HT_00010); break;
  case    11: return ((char *)HT_00011); break;
  case    12: return ((char *)HT_00012); break;
  case    20: return ((char *)HT_00020); break;
  case    21: return ((char *)HT_00021); break;
  case    22: return ((char *)HT_00022); break;
  case    23: return ((char *)HT_00023); break;
  case    30: return ((char *)HT_00030); break;
  case    40: return ((char *)HT_00040); break;
  case    50: return ((char *)HT_00050); break;
  case    60: return ((char *)HT_00060); break;
  case   100: return ((char *)HT_00100); break;
  case   101: return ((char *)HT_00101); break;
  case   110: return ((char *)HT_00110); break;
  case   111: return ((char *)HT_00111); break;
  case   112: return ((char *)HT_00112); break;
  case   120: return ((char *)HT_00120); break;
  case   121: return ((char *)HT_00121); break;
  case   122: return ((char *)HT_00122); break;
  case   124: return ((char *)HT_00124); break;
  case   125: return ((char *)HT_00125); break;
  case   130: return ((char *)HT_00130); break;
  case   131: return ((char *)HT_00131); break;
  case   132: return ((char *)HT_00132); break;
  case   133: return ((char *)HT_00133); break;
  case   140: return ((char *)HT_00140); break;
  case   141: return ((char *)HT_00141); break;
  case   150: return ((char *)HT_00150); break;
  case   160: return ((char *)HT_00160); break;
  case   200: return ((char *)HT_00200); break;
  case   300: return ((char *)HT_00300); break;
  case   400: return ((char *)HT_00400); break;
  case   500: return ((char *)HT_00500); break;
  case   501: return ((char *)HT_00501); break;
  case   900: return ((char *)HT_00900); break;
  case   910: return ((char *)HT_00910); break;
  case  1000: return ((char *)HT_01000); break;
  case  1100: return ((char *)HT_01100); break;
  case  1400: return ((char *)HT_01400); break;
  case  1410: return ((char *)HT_01410); break;
  case  1420: return ((char *)HT_01420); break;
  case  1421: return ((char *)HT_01421); break;
  case  1430: return ((char *)HT_01430); break;
  case  1440: return ((char *)HT_01440); break;
  case  1441: return ((char *)HT_01441); break;
  case  1450: return ((char *)HT_01450); break;
  case  1460: return ((char *)HT_01460); break;
  case  1500: return ((char *)HT_01500); break;
  case  1600: return ((char *)HT_01600); break;
  case  1700: return ((char *)HT_01700); break;
  case  1710: return ((char *)HT_01710); break;
  case  1711: return ((char *)HT_01711); break;
  case  1720: return ((char *)HT_01720); break;
  case  1722: return ((char *)HT_01722); break;
  case  1730: return ((char *)HT_01730); break;
  case  1731: return ((char *)HT_01731); break;
  case  1740: return ((char *)HT_01740); break;
  case  1750: return ((char *)HT_01750); break;
  case  1760: return ((char *)HT_01760); break;
  case  1800: return ((char *)HT_01800); break;
  case  2100: return ((char *)HT_02100); break;
  case  2400: return ((char *)HT_02400); break;
  case  2410: return ((char *)HT_02410); break;
  case  2500: return ((char *)HT_02500); break;
  case  2600: return ((char *)HT_02600); break;
  case  2611: return ((char *)HT_02611); break;
  case  2612: return ((char *)HT_02612); break;
  case  2711: return ((char *)HT_02711); break;
  case  2811: return ((char *)HT_02811); break;
  case  3000: return ((char *)HT_03000); break;
  case  3100: return ((char *)HT_03100); break;
  case  3200: return ((char *)HT_03200); break;
  case  3710: return ((char *)HT_03710); break;
  case  3711: return ((char *)HT_03711); break;
  case  3800: return ((char *)HT_03800); break;
  case  4300: return ((char *)HT_04300); break;
  case  4400: return ((char *)HT_04400); break;
  case  4500: return ((char *)HT_04500); break;
  case  4700: return ((char *)HT_04700); break;
  case  4800: return ((char *)HT_04800); break;
  case  4900: return ((char *)HT_04900); break;
  case  5000: return ((char *)HT_05000); break;
  case  5100: return ((char *)HT_05100); break;
  case  5200: return ((char *)HT_05200); break;
  case  5300: return ((char *)HT_05300); break;
  case  5400: return ((char *)HT_05400); break;
  case  5500: return ((char *)HT_05500); break;
  case  5600: return ((char *)HT_05600); break;
  case  5700: return ((char *)HT_05700); break;
  case  5800: return ((char *)HT_05800); break;
  case  6000: return ((char *)HT_06000); break;
  case  6100: return ((char *)HT_06100); break;
  case  6211: return ((char *)HT_06211); break;
  case  6212: return ((char *)HT_06212); break;
  case  6213: return ((char *)HT_06213); break;
  case  6221: return ((char *)HT_06221); break;
  case  6222: return ((char *)HT_06222); break;
  case  6223: return ((char *)HT_06223); break;
  case  6231: return ((char *)HT_06231); break;
  case  6232: return ((char *)HT_06232); break;
  case  6233: return ((char *)HT_06233); break;
  case  6241: return ((char *)HT_06241); break;
  case  6242: return ((char *)HT_06242); break;
  case  6243: return ((char *)HT_06243); break;
  case  6300: return ((char *)HT_06300); break;
  case  6400: return ((char *)HT_06400); break;
  case  6500: return ((char *)HT_06500); break;
  case  6600: return ((char *)HT_06600); break;
  case  6700: return ((char *)HT_06700); break;
  case  6800: return ((char *)HT_06800); break;
  case  6900: return ((char *)HT_06900); break;
  case  7100: return ((char *)HT_07100); break;
  case  7200: return ((char *)HT_07200); break;
  case  7300: return ((char *)HT_07300); break;
  case  7400: return ((char *)HT_07400); break;
  case  7500: return ((char *)HT_07500); break;
  case  7600: return ((char *)HT_07600); break;
  case  7700: return ((char *)HT_07700); break;
  case  7800: return ((char *)HT_07800); break;
  case  7900: return ((char *)HT_07900); break;
  case  8000: return ((char *)HT_08000); break;
  case  8100: return ((char *)HT_08100); break;
  case  8200: return ((char *)HT_08200); break;
  case  8300: return ((char *)HT_08300); break;
  case  8400: return ((char *)HT_08400); break;
  case  8500: return ((char *)HT_08500); break;
  case  8600: return ((char *)HT_08600); break;
  case  8700: return ((char *)HT_08700); break;
  case  8800: return ((char *)HT_08800); break;
  case  8900: return ((char *)HT_08900); break;
  case  9000: return ((char *)HT_09000); break;
  case  9100: return ((char *)HT_09100); break;
  case  9200: return ((char *)HT_09200); break;
  case  9300: return ((char *)HT_09300); break;
  case  9400: return ((char *)HT_09400); break;
  case  9500: return ((char *)HT_09500); break;
  case  9600: return ((char *)HT_09600); break;
  case  9700: return ((char *)HT_09700); break;
  case  9710: return ((char *)HT_09710); break;
  case  9720: return ((char *)HT_09720); break;
  case  9800: return ((char *)HT_09800); break;
  case  9810: return ((char *)HT_09810); break;
  case  9820: return ((char *)HT_09820); break;
  case  9900: return ((char *)HT_09900); break;
  case 10000: return ((char *)HT_10000); break;
  case 10100: return ((char *)HT_10100); break;
  case 10200: return ((char *)HT_10200); break;
  case 10300: return ((char *)HT_10300); break;
  case 10400: return ((char *)HT_10400); break;
  case 10410: return ((char *)HT_10410); break;
  case 10420: return ((char *)HT_10420); break;
  case 10500: return ((char *)HT_10500); break;
  case 10600: return ((char *)HT_10600); break;
  case 10700: return ((char *)HT_10700); break;
  case 10800: return ((char *)HT_10800); break;
  case 10900: return ((char *)HT_10900); break;
  case 11000: return ((char *)HT_11000); break;
  case 11100: return ((char *)HT_11100); break;
  case 11200: return ((char *)HT_11200); break;
  case 11300: return ((char *)HT_11300); break;
  case 11400: return ((char *)HT_11400); break;
  case 11500: return ((char *)HT_11500); break;
  case 11600: return ((char *)HT_11600); break;
  case 11700: return ((char *)HT_11700); break;
  case 11800: return ((char *)HT_11800); break;
  case 11900: return ((char *)HT_11900); break;
  case 12000: return ((char *)HT_12000); break;
  case 12100: return ((char *)HT_12100); break;
  case 12200: return ((char *)HT_12200); break;
  case 12300: return ((char *)HT_12300); break;
  case 12400: return ((char *)HT_12400); break;
  case 12500: return ((char *)HT_12500); break;
  case 12600: return ((char *)HT_12600); break;
  case 12700: return ((char *)HT_12700); break;
  case 12800: return ((char *)HT_12800); break;
  case 12900: return ((char *)HT_12900); break;
  case 13000: return ((char *)HT_13000); break;
  case 13100: return ((char *)HT_13100); break;
  case 13200: return ((char *)HT_13200); break;
  case 13300: return ((char *)HT_13300); break;
  case 13400: return ((char *)HT_13400); break;
  case 13500: return ((char *)HT_13500); break;
  case 13600: return ((char *)HT_13600); break;
  case 13711: return ((char *)HT_13711); break;
  case 13712: return ((char *)HT_13712); break;
  case 13713: return ((char *)HT_13713); break;
  case 13721: return ((char *)HT_13721); break;
  case 13722: return ((char *)HT_13722); break;
  case 13723: return ((char *)HT_13723); break;
  case 13731: return ((char *)HT_13731); break;
  case 13732: return ((char *)HT_13732); break;
  case 13733: return ((char *)HT_13733); break;
  case 13741: return ((char *)HT_13741); break;
  case 13742: return ((char *)HT_13742); break;
  case 13743: return ((char *)HT_13743); break;
  case 13751: return ((char *)HT_13751); break;
  case 13752: return ((char *)HT_13752); break;
  case 13753: return ((char *)HT_13753); break;
  case 13761: return ((char *)HT_13761); break;
  case 13762: return ((char *)HT_13762); break;
  case 13763: return ((char *)HT_13763); break;
  case 13800: return ((char *)HT_13800); break;
  case 13900: return ((char *)HT_13900); break;
  }

  return ((char *) "Unknown");
}

char *strstatus(const uint devices_status)
{
  switch (devices_status)
  {
  case  STATUS_INIT:               return ((char *)ST_0000); break;
  case  STATUS_STARTING:           return ((char *)ST_0001); break;
  case  STATUS_RUNNING:            return ((char *)ST_0002); break;
  case  STATUS_PAUSED:             return ((char *)ST_0003); break;
  case  STATUS_EXHAUSTED:          return ((char *)ST_0004); break;
  case  STATUS_CRACKED:            return ((char *)ST_0005); break;
  case  STATUS_ABORTED:            return ((char *)ST_0006); break;
  case  STATUS_QUIT:               return ((char *)ST_0007); break;
  case  STATUS_BYPASS:             return ((char *)ST_0008); break;
  case  STATUS_STOP_AT_CHECKPOINT: return ((char *)ST_0009); break;
  case  STATUS_AUTOTUNE:           return ((char *)ST_0010); break;
  }

  return ((char *) "Unknown");
}

void to_hccap_t(hccap_t *hccap, uint salt_pos, uint digest_pos)
{
  memset(hccap, 0, sizeof(hccap_t));

  salt_t *salt = &data.salts_buf[salt_pos];

  memcpy(hccap->essid, salt->salt_buf, salt->salt_len);

  wpa_t *wpas = (wpa_t *)data.esalts_buf;
  wpa_t *wpa = &wpas[salt_pos];

  hccap->keyver = wpa->keyver;

  hccap->eapol_size = wpa->eapol_size;

  if (wpa->keyver != 1)
  {
    uint eapol_tmp[64] = { 0 };

    for (uint i = 0; i < 64; i++)
    {
      eapol_tmp[i] = byte_swap_32(wpa->eapol[i]);
    }

    memcpy(hccap->eapol, eapol_tmp, wpa->eapol_size);
  }
  else
  {
    memcpy(hccap->eapol, wpa->eapol, wpa->eapol_size);
  }

  memcpy(hccap->mac1, wpa->orig_mac1, 6);
  memcpy(hccap->mac2, wpa->orig_mac2, 6);
  memcpy(hccap->nonce1, wpa->orig_nonce1, 32);
  memcpy(hccap->nonce2, wpa->orig_nonce2, 32);

  char *digests_buf_ptr = (char *)data.digests_buf;

  uint dgst_size = data.dgst_size;

  uint *digest_ptr = (uint *)(digests_buf_ptr + (data.salts_buf[salt_pos].digests_offset * dgst_size) + (digest_pos * dgst_size));

  if (wpa->keyver != 1)
  {
    uint digest_tmp[4] = { 0 };

    digest_tmp[0] = byte_swap_32(digest_ptr[0]);
    digest_tmp[1] = byte_swap_32(digest_ptr[1]);
    digest_tmp[2] = byte_swap_32(digest_ptr[2]);
    digest_tmp[3] = byte_swap_32(digest_ptr[3]);

    memcpy(hccap->keymic, digest_tmp, 16);
  }
  else
  {
    memcpy(hccap->keymic, digest_ptr, 16);
  }
}

void SuspendThreads()
{
  if (data.devices_status != STATUS_RUNNING) return;

  hc_timer_set(&data.timer_paused);

  data.devices_status = STATUS_PAUSED;

  log_info("Paused");
}

void ResumeThreads()
{
  if (data.devices_status != STATUS_PAUSED) return;

  double ms_paused;

  hc_timer_get(data.timer_paused, ms_paused);

  data.ms_paused += ms_paused;

  data.devices_status = STATUS_RUNNING;

  log_info("Resumed");
}

void bypass()
{
  data.devices_status = STATUS_BYPASS;

  log_info("Next dictionary / mask in queue selected, bypassing current one");
}

void stop_at_checkpoint()
{
  if (data.devices_status != STATUS_STOP_AT_CHECKPOINT)
  {
    if (data.devices_status != STATUS_RUNNING) return;
  }

  // this feature only makes sense if --restore-disable was not specified

  if (data.restore_disable == 1)
  {
    log_info("WARNING: This feature is disabled when --restore-disable is specified");

    return;
  }

  // check if monitoring of Restore Point updates should be enabled or disabled

  if (data.devices_status != STATUS_STOP_AT_CHECKPOINT)
  {
    data.devices_status = STATUS_STOP_AT_CHECKPOINT;

    // save the current restore point value

    data.checkpoint_cur_words = get_lowest_words_done();

    log_info("Checkpoint enabled: Will quit at next Restore Point update");
  }
  else
  {
    data.devices_status = STATUS_RUNNING;

    // reset the global value for checkpoint checks

    data.checkpoint_cur_words = 0;

    log_info("Checkpoint disabled: Restore Point updates will no longer be monitored");
  }
}

void myabort()
{
  data.devices_status = STATUS_ABORTED;
}

void myquit()
{
  data.devices_status = STATUS_QUIT;
}

void naive_replace(char *s, const u8 key_char, const u8 replace_char)
{
  const size_t len = strlen(s);

  for (size_t in = 0; in < len; in++)
  {
    const u8 c = s[in];

    if (c == key_char)
    {
      s[in] = replace_char;
    }
  }
}

void naive_escape(char *s, size_t s_max, const u8 key_char, const u8 escape_char)
{
  char s_escaped[1024] = { 0 };

  size_t s_escaped_max = sizeof(s_escaped);

  const size_t len = strlen(s);

  for (size_t in = 0, out = 0; in < len; in++, out++)
  {
    const u8 c = s[in];

    if (c == key_char)
    {
      s_escaped[out] = escape_char;

      out++;
    }

    if (out == s_escaped_max - 2) break;

    s_escaped[out] = c;
  }

  strncpy(s, s_escaped, s_max - 1);
}

void load_kernel(const char *kernel_file, int num_devices, size_t *kernel_lengths, const u8 **kernel_sources)
{
  FILE *fp = fopen(kernel_file, "rb");

  if (fp != NULL)
  {
    struct stat st;

    memset(&st, 0, sizeof(st));

    stat(kernel_file, &st);

    u8 *buf = (u8 *)mymalloc(st.st_size + 1);

    size_t num_read = fread(buf, sizeof(u8), st.st_size, fp);

    if (num_read != (size_t)st.st_size)
    {
      log_error("ERROR: %s: %s", kernel_file, strerror(errno));

      exit(-1);
    }

    fclose(fp);

    buf[st.st_size] = 0;

    for (int i = 0; i < num_devices; i++)
    {
      kernel_lengths[i] = (size_t)st.st_size;

      kernel_sources[i] = buf;
    }
  }
  else
  {
    log_error("ERROR: %s: %s", kernel_file, strerror(errno));

    exit(-1);
  }

  return;
}

void writeProgramBin(char *dst, u8 *binary, size_t binary_size)
{
  if (binary_size > 0)
  {
    FILE *fp = fopen(dst, "wb");

    lock_file(fp);
    fwrite(binary, sizeof(u8), binary_size, fp);

    fflush(fp);
    fclose(fp);
  }
}

/**
 * restore
 */

restore_data_t *init_restore(int argc, char **argv)
{
  restore_data_t *rd = (restore_data_t *)mymalloc(sizeof(restore_data_t));

  if (data.restore_disable == 0)
  {
    FILE *fp = fopen(data.eff_restore_file, "rb");

    if (fp)
    {
      size_t nread = fread(rd, sizeof(restore_data_t), 1, fp);

      if (nread != 1)
      {
        log_error("ERROR: Cannot read %s", data.eff_restore_file);

        exit(-1);
      }

      fclose(fp);

      if (rd->pid)
      {
        char *pidbin = (char *)mymalloc(HCBUFSIZ);

        int pidbin_len = -1;

#ifdef _POSIX
        snprintf(pidbin, HCBUFSIZ - 1, "/proc/%d/cmdline", rd->pid);

        FILE *fd = fopen(pidbin, "rb");

        if (fd)
        {
          pidbin_len = fread(pidbin, 1, HCBUFSIZ, fd);

          pidbin[pidbin_len] = 0;

          fclose(fd);

          char *argv0_r = strrchr(argv[0], '/');

          char *pidbin_r = strrchr(pidbin, '/');

          if (argv0_r == NULL) argv0_r = argv[0];

          if (pidbin_r == NULL) pidbin_r = pidbin;

          if (strcmp(argv0_r, pidbin_r) == 0)
          {
            log_error("ERROR: Already an instance %s running on pid %d", pidbin, rd->pid);

            exit(-1);
          }
        }

#elif _WIN
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, rd->pid);

        char *pidbin2 = (char *)mymalloc(HCBUFSIZ);

        int pidbin2_len = -1;

        pidbin_len = GetModuleFileName(NULL, pidbin, HCBUFSIZ);
        pidbin2_len = GetModuleFileNameEx(hProcess, NULL, pidbin2, HCBUFSIZ);

        pidbin[pidbin_len] = 0;
        pidbin2[pidbin2_len] = 0;

        if (pidbin2_len)
        {
          if (strcmp(pidbin, pidbin2) == 0)
          {
            log_error("ERROR: Already an instance %s running on pid %d", pidbin2, rd->pid);

            exit(-1);
          }
        }

        myfree(pidbin2);

#endif

        myfree(pidbin);
      }

      if (rd->version_bin < RESTORE_MIN)
      {
        log_error("ERROR: Cannot use outdated %s. Please remove it.", data.eff_restore_file);

        exit(-1);
      }
    }
  }

  memset(rd, 0, sizeof(restore_data_t));

  rd->version_bin = VERSION_BIN;

#ifdef _POSIX
  rd->pid = getpid();
#elif _WIN
  rd->pid = GetCurrentProcessId();
#endif

  if (getcwd(rd->cwd, 255) == NULL)
  {
    myfree(rd);

    return (NULL);
  }

  rd->argc = argc;
  rd->argv = argv;

  return (rd);
}

void read_restore(const char *eff_restore_file, restore_data_t *rd)
{
  FILE *fp = fopen(eff_restore_file, "rb");

  if (fp == NULL)
  {
    log_error("ERROR: Restore file '%s': %s", eff_restore_file, strerror(errno));

    exit(-1);
  }

  if (fread(rd, sizeof(restore_data_t), 1, fp) != 1)
  {
    log_error("ERROR: Can't read %s", eff_restore_file);

    exit(-1);
  }

  rd->argv = (char **)mycalloc(rd->argc, sizeof(char *));

  char *buf = (char *)mymalloc(HCBUFSIZ);

  for (uint i = 0; i < rd->argc; i++)
  {
    if (fgets(buf, HCBUFSIZ - 1, fp) == NULL)
    {
      log_error("ERROR: Can't read %s", eff_restore_file);

      exit(-1);
    }

    size_t len = strlen(buf);

    if (len) buf[len - 1] = 0;

    rd->argv[i] = mystrdup(buf);
  }

  myfree(buf);

  fclose(fp);

  log_info("INFO: Changing current working directory to the path found within the .restore file: '%s'", rd->cwd);

  if (chdir(rd->cwd))
  {
    log_error("ERROR: The directory '%s' does not exist. It is needed to restore (--restore) the session.\n"
      "       You could either create this directory (or link it) or update the .restore file using e.g. the analyze_hc_restore.pl tool:\n"
      "       https://github.com/philsmd/analyze_hc_restore\n"
      "       The directory must be relative to (or contain) all files/folders mentioned within the command line.", rd->cwd);

    exit(-1);
  }
}

u64 get_lowest_words_done()
{
  u64 words_cur = -1;

  for (uint device_id = 0; device_id < data.devices_cnt; device_id++)
  {
    hc_device_param_t *device_param = &data.devices_param[device_id];

    if (device_param->skipped) continue;

    const u64 words_done = device_param->words_done;

    if (words_done < words_cur) words_cur = words_done;
  }

  // It's possible that a device's workload isn't finished right after a restore-case.
  // In that case, this function would return 0 and overwrite the real restore point
  // There's also data.words_cur which is set to rd->words_cur but it changes while
  // the attack is running therefore we should stick to rd->words_cur.
  // Note that -s influences rd->words_cur we should keep a close look on that.

  if (words_cur < data.rd->words_cur) words_cur = data.rd->words_cur;

  return words_cur;
}

void write_restore(const char *new_restore_file, restore_data_t *rd)
{
  u64 words_cur = get_lowest_words_done();

  rd->words_cur = words_cur;

  FILE *fp = fopen(new_restore_file, "wb");

  if (fp == NULL)
  {
    log_error("ERROR: %s: %s", new_restore_file, strerror(errno));

    exit(-1);
  }

  if (setvbuf(fp, NULL, _IONBF, 0))
  {
    log_error("ERROR: setvbuf file '%s': %s", new_restore_file, strerror(errno));

    exit(-1);
  }

  fwrite(rd, sizeof(restore_data_t), 1, fp);

  for (uint i = 0; i < rd->argc; i++)
  {
    fprintf(fp, "%s", rd->argv[i]);
    fputc('\n', fp);
  }

  fflush(fp);

  fsync(fileno(fp));

  fclose(fp);
}

void cycle_restore()
{
  const char *eff_restore_file = data.eff_restore_file;
  const char *new_restore_file = data.new_restore_file;

  restore_data_t *rd = data.rd;

  write_restore(new_restore_file, rd);

  struct stat st;

  memset(&st, 0, sizeof(st));

  if (stat(eff_restore_file, &st) == 0)
  {
    if (unlink(eff_restore_file))
    {
      log_info("WARN: Unlink file '%s': %s", eff_restore_file, strerror(errno));
    }
  }

  if (rename(new_restore_file, eff_restore_file))
  {
    log_info("WARN: Rename file '%s' to '%s': %s", new_restore_file, eff_restore_file, strerror(errno));
  }
}

void check_checkpoint()
{
  // if (data.restore_disable == 1) break;  (this is already implied by previous checks)

  u64 words_cur = get_lowest_words_done();

  if (words_cur != data.checkpoint_cur_words)
  {
    myabort();
  }
}

/**
 * tuning db
 */

void tuning_db_destroy(tuning_db_t *tuning_db)
{
  int i;

  for (i = 0; i < tuning_db->alias_cnt; i++)
  {
    tuning_db_alias_t *alias = &tuning_db->alias_buf[i];

    myfree(alias->device_name);
    myfree(alias->alias_name);
  }

  for (i = 0; i < tuning_db->entry_cnt; i++)
  {
    tuning_db_entry_t *entry = &tuning_db->entry_buf[i];

    myfree(entry->device_name);
  }

  myfree(tuning_db->alias_buf);
  myfree(tuning_db->entry_buf);

  myfree(tuning_db);
}

tuning_db_t *tuning_db_alloc(FILE *fp)
{
  tuning_db_t *tuning_db = (tuning_db_t *)mymalloc(sizeof(tuning_db_t));

  int num_lines = count_lines(fp);

  // a bit over-allocated

  tuning_db->alias_buf = (tuning_db_alias_t *)mycalloc(num_lines + 1, sizeof(tuning_db_alias_t));
  tuning_db->alias_cnt = 0;

  tuning_db->entry_buf = (tuning_db_entry_t *)mycalloc(num_lines + 1, sizeof(tuning_db_entry_t));
  tuning_db->entry_cnt = 0;

  return tuning_db;
}

tuning_db_t *tuning_db_init(const char *tuning_db_file)
{
  FILE *fp = fopen(tuning_db_file, "rb");

  if (fp == NULL)
  {
    log_error("%s: %s", tuning_db_file, strerror(errno));

    exit(-1);
  }

  tuning_db_t *tuning_db = tuning_db_alloc(fp);

  rewind(fp);

  int line_num = 0;

  char *buf = (char *)mymalloc(HCBUFSIZ);

  while (!feof(fp))
  {
    char *line_buf = fgets(buf, HCBUFSIZ - 1, fp);

    if (line_buf == NULL) break;

    line_num++;

    const int line_len = in_superchop(line_buf);

    if (line_len == 0) continue;

    if (line_buf[0] == '#') continue;

    // start processing

    char *token_ptr[7] = { NULL };

    int token_cnt = 0;

    char *next = strtok(line_buf, "\t ");

    token_ptr[token_cnt] = next;

    token_cnt++;

    while ((next = strtok(NULL, "\t ")) != NULL)
    {
      token_ptr[token_cnt] = next;

      token_cnt++;
    }

    if (token_cnt == 2)
    {
      char *device_name = token_ptr[0];
      char *alias_name = token_ptr[1];

      tuning_db_alias_t *alias = &tuning_db->alias_buf[tuning_db->alias_cnt];

      alias->device_name = mystrdup(device_name);
      alias->alias_name = mystrdup(alias_name);

      tuning_db->alias_cnt++;
    }
    else if (token_cnt == 6)
    {
      if ((token_ptr[1][0] != '0') &&
        (token_ptr[1][0] != '1') &&
        (token_ptr[1][0] != '3') &&
        (token_ptr[1][0] != '*'))
      {
        log_info("WARNING: Tuning-db: Invalid attack_mode '%c' in Line '%u'", token_ptr[1][0], line_num);

        continue;
      }

      if ((token_ptr[3][0] != '1') &&
        (token_ptr[3][0] != '2') &&
        (token_ptr[3][0] != '4') &&
        (token_ptr[3][0] != '8') &&
        (token_ptr[3][0] != 'N'))
      {
        log_info("WARNING: Tuning-db: Invalid vector_width '%c' in Line '%u'", token_ptr[3][0], line_num);

        continue;
      }

      char *device_name = token_ptr[0];

      int attack_mode = -1;
      int hash_type = -1;
      int vector_width = -1;
      int kernel_accel = -1;
      int kernel_loops = -1;

      if (token_ptr[1][0] != '*') attack_mode = atoi(token_ptr[1]);
      if (token_ptr[2][0] != '*') hash_type = atoi(token_ptr[2]);
      if (token_ptr[3][0] != 'N') vector_width = atoi(token_ptr[3]);

      if (token_ptr[4][0] != 'A')
      {
        kernel_accel = atoi(token_ptr[4]);

        if ((kernel_accel < 1) || (kernel_accel > 1024))
        {
          log_info("WARNING: Tuning-db: Invalid kernel_accel '%d' in Line '%u'", kernel_accel, line_num);

          continue;
        }
      }
      else
      {
        kernel_accel = 0;
      }

      if (token_ptr[5][0] != 'A')
      {
        kernel_loops = atoi(token_ptr[5]);

        if ((kernel_loops < 1) || (kernel_loops > 1024))
        {
          log_info("WARNING: Tuning-db: Invalid kernel_loops '%d' in Line '%u'", kernel_loops, line_num);

          continue;
        }
      }
      else
      {
        kernel_loops = 0;
      }

      tuning_db_entry_t *entry = &tuning_db->entry_buf[tuning_db->entry_cnt];

      entry->device_name = mystrdup(device_name);
      entry->attack_mode = attack_mode;
      entry->hash_type = hash_type;
      entry->vector_width = vector_width;
      entry->kernel_accel = kernel_accel;
      entry->kernel_loops = kernel_loops;

      tuning_db->entry_cnt++;
    }
    else
    {
      log_info("WARNING: Tuning-db: Invalid number of token in Line '%u'", line_num);

      continue;
    }
  }

  myfree(buf);

  fclose(fp);

  // todo: print loaded 'cnt' message

  // sort the database

  qsort(tuning_db->alias_buf, tuning_db->alias_cnt, sizeof(tuning_db_alias_t), sort_by_tuning_db_alias);
  qsort(tuning_db->entry_buf, tuning_db->entry_cnt, sizeof(tuning_db_entry_t), sort_by_tuning_db_entry);

  return tuning_db;
}

tuning_db_entry_t *tuning_db_search(tuning_db_t *tuning_db, hc_device_param_t *device_param, int attack_mode, int hash_type)
{
  static tuning_db_entry_t s;

  // first we need to convert all spaces in the device_name to underscore

  char *device_name_nospace = strdup(device_param->device_name);

  int device_name_length = strlen(device_name_nospace);

  int i;

  for (i = 0; i < device_name_length; i++)
  {
    if (device_name_nospace[i] == ' ') device_name_nospace[i] = '_';
  }

  // find out if there's an alias configured

  tuning_db_alias_t a;

  a.device_name = device_name_nospace;

  tuning_db_alias_t *alias = bsearch(&a, tuning_db->alias_buf, tuning_db->alias_cnt, sizeof(tuning_db_alias_t), sort_by_tuning_db_alias);

  char *alias_name = (alias == NULL) ? NULL : alias->alias_name;

  // attack-mode 6 and 7 are attack-mode 1 basically

  if (attack_mode == 6) attack_mode = 1;
  if (attack_mode == 7) attack_mode = 1;

  // bsearch is not ideal but fast enough

  s.device_name = device_name_nospace;
  s.attack_mode = attack_mode;
  s.hash_type = hash_type;

  tuning_db_entry_t *entry = NULL;

  // this will produce all 2^3 combinations required

  for (i = 0; i < 8; i++)
  {
    s.device_name = (i & 1) ? "*" : device_name_nospace;
    s.attack_mode = (i & 2) ? -1 : attack_mode;
    s.hash_type = (i & 4) ? -1 : hash_type;

    entry = bsearch(&s, tuning_db->entry_buf, tuning_db->entry_cnt, sizeof(tuning_db_entry_t), sort_by_tuning_db_entry);

    if (entry != NULL) break;

    // in non-wildcard mode do some additional checks:

    if ((i & 1) == 0)
    {
      // in case we have an alias-name

      if (alias_name != NULL)
      {
        s.device_name = alias_name;

        entry = bsearch(&s, tuning_db->entry_buf, tuning_db->entry_cnt, sizeof(tuning_db_entry_t), sort_by_tuning_db_entry);

        if (entry != NULL) break;
      }

      // or by device type

      if (device_param->device_type & CL_DEVICE_TYPE_CPU)
      {
        s.device_name = "DEVICE_TYPE_CPU";
      }
      else if (device_param->device_type & CL_DEVICE_TYPE_GPU)
      {
        s.device_name = "DEVICE_TYPE_GPU";
      }
      else if (device_param->device_type & CL_DEVICE_TYPE_ACCELERATOR)
      {
        s.device_name = "DEVICE_TYPE_ACCELERATOR";
      }

      entry = bsearch(&s, tuning_db->entry_buf, tuning_db->entry_cnt, sizeof(tuning_db_entry_t), sort_by_tuning_db_entry);

      if (entry != NULL) break;
    }
  }

  // free converted device_name

  myfree(device_name_nospace);

  return entry;
}



/**
 * parallel running threads
 */

#ifdef WIN

BOOL WINAPI sigHandler_default(DWORD sig)
{
  switch (sig)
  {
  case CTRL_CLOSE_EVENT:

    /*
     * special case see: https://stackoverflow.com/questions/3640633/c-setconsolectrlhandler-routine-issue/5610042#5610042
     * if the user interacts w/ the user-interface (GUI/cmd), we need to do the finalization job within this signal handler
     * function otherwise it is too late (e.g. after returning from this function)
     */

    myabort();

    SetConsoleCtrlHandler(NULL, TRUE);

    hc_sleep(10);

    return TRUE;

  case CTRL_C_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_SHUTDOWN_EVENT:

    myabort();

    SetConsoleCtrlHandler(NULL, TRUE);

    return TRUE;
  }

  return FALSE;
}

BOOL WINAPI sigHandler_benchmark(DWORD sig)
{
  switch (sig)
  {
  case CTRL_CLOSE_EVENT:

    myquit();

    SetConsoleCtrlHandler(NULL, TRUE);

    hc_sleep(10);

    return TRUE;

  case CTRL_C_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_SHUTDOWN_EVENT:

    myquit();

    SetConsoleCtrlHandler(NULL, TRUE);

    return TRUE;
  }

  return FALSE;
}

void hc_signal(BOOL WINAPI(callback) (DWORD))
{
  if (callback == NULL)
  {
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)callback, FALSE);
  }
  else
  {
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)callback, TRUE);
  }
}

#else

void sigHandler_default(int sig)
{
  myabort();

  signal(sig, NULL);
}

void sigHandler_benchmark(int sig)
{
  myquit();

  signal(sig, NULL);
}

void hc_signal(void (callback)(int))
{
  if (callback == NULL) callback = SIG_DFL;

  signal(SIGINT, callback);
  signal(SIGTERM, callback);
  signal(SIGABRT, callback);
}

#endif

void status_display();

void *thread_keypress(void *p)
{
  uint quiet = data.quiet;

  tty_break();

  while (data.shutdown_outer == 0)
  {
    int ch = tty_getchar();

    if (ch == -1) break;

    if (ch == 0) continue;

    //https://github.com/hashcat/hashcat/issues/302
    //#ifdef _POSIX
    //if (ch != '\n')
    //#endif

    hc_thread_mutex_lock(mux_display);

    log_info("");

    switch (ch)
    {
    case 's':
    case '\r':
    case '\n':

      log_info("");

      status_display();

      log_info("");

      if (quiet == 0) fprintf(stdout, "%s", PROMPT);
      if (quiet == 0) fflush(stdout);

      break;

    case 'b':

      log_info("");

      bypass();

      log_info("");

      if (quiet == 0) fprintf(stdout, "%s", PROMPT);
      if (quiet == 0) fflush(stdout);

      break;

    case 'p':

      log_info("");

      SuspendThreads();

      log_info("");

      if (quiet == 0) fprintf(stdout, "%s", PROMPT);
      if (quiet == 0) fflush(stdout);

      break;

    case 'r':

      log_info("");

      ResumeThreads();

      log_info("");

      if (quiet == 0) fprintf(stdout, "%s", PROMPT);
      if (quiet == 0) fflush(stdout);

      break;

    case 'c':

      log_info("");

      stop_at_checkpoint();

      log_info("");

      if (quiet == 0) fprintf(stdout, "%s", PROMPT);
      if (quiet == 0) fflush(stdout);

      break;

    case 'q':

      log_info("");

      myabort();

      break;
    }

    //https://github.com/hashcat/hashcat/issues/302
    //#ifdef _POSIX
    //if (ch != '\n')
    //#endif

    hc_thread_mutex_unlock(mux_display);
  }

  tty_fix();

  return (p);
}

/**
 * rules common
 */

bool class_num(const u8 c)
{
  return ((c >= '0') && (c <= '9'));
}

bool class_lower(const u8 c)
{
  return ((c >= 'a') && (c <= 'z'));
}

bool class_upper(const u8 c)
{
  return ((c >= 'A') && (c <= 'Z'));
}

bool class_alpha(const u8 c)
{
  return (class_lower(c) || class_upper(c));
}

int conv_ctoi(const u8 c)
{
  if (class_num(c))
  {
    return c - '0';
  }
  else if (class_upper(c))
  {
    return c - 'A' + 10;
  }

  return -1;
}

int conv_itoc(const u8 c)
{
  if (c < 10)
  {
    return c + '0';
  }
  else if (c < 37)
  {
    return c + 'A' - 10;
  }

  return -1;
}

/**
 * device rules
 */

#define INCR_POS           if (++rule_pos == rule_len) return (-1)
#define SET_NAME(rule,val) (rule)->cmds[rule_cnt]  = ((val) & 0xff) <<  0
#define SET_P0(rule,val)   INCR_POS; (rule)->cmds[rule_cnt] |= ((val) & 0xff) <<  8
#define SET_P1(rule,val)   INCR_POS; (rule)->cmds[rule_cnt] |= ((val) & 0xff) << 16
#define MAX_KERNEL_RULES   255
#define GET_NAME(rule)     rule_cmd = (((rule)->cmds[rule_cnt] >>  0) & 0xff)
#define GET_P0(rule)       INCR_POS; rule_buf[rule_pos] = (((rule)->cmds[rule_cnt] >>  8) & 0xff)
#define GET_P1(rule)       INCR_POS; rule_buf[rule_pos] = (((rule)->cmds[rule_cnt] >> 16) & 0xff)

#define SET_P0_CONV(rule,val)  INCR_POS; (rule)->cmds[rule_cnt] |= ((conv_ctoi (val)) & 0xff) <<  8
#define SET_P1_CONV(rule,val)  INCR_POS; (rule)->cmds[rule_cnt] |= ((conv_ctoi (val)) & 0xff) << 16
#define GET_P0_CONV(rule)      INCR_POS; rule_buf[rule_pos] = conv_itoc (((rule)->cmds[rule_cnt] >>  8) & 0xff)
#define GET_P1_CONV(rule)      INCR_POS; rule_buf[rule_pos] = conv_itoc (((rule)->cmds[rule_cnt] >> 16) & 0xff)

int cpu_rule_to_kernel_rule(char *rule_buf, uint rule_len, kernel_rule_t *rule)
{
  uint rule_pos;
  uint rule_cnt;

  for (rule_pos = 0, rule_cnt = 0; rule_pos < rule_len && rule_cnt < MAX_KERNEL_RULES; rule_pos++, rule_cnt++)
  {
    switch (rule_buf[rule_pos])
    {
    case ' ':
      rule_cnt--;
      break;

    case RULE_OP_MANGLE_NOOP:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_LREST:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_UREST:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_LREST_UFIRST:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_UREST_LFIRST:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_TREST:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_TOGGLE_AT:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_REVERSE:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_DUPEWORD:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_DUPEWORD_TIMES:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_REFLECT:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_ROTATE_LEFT:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_ROTATE_RIGHT:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_APPEND:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_PREPEND:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_DELETE_FIRST:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_DELETE_LAST:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_DELETE_AT:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_EXTRACT:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      SET_P1_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_OMIT:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      SET_P1_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_INSERT:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      SET_P1(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_OVERSTRIKE:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      SET_P1(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_TRUNCATE_AT:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_REPLACE:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0(rule, rule_buf[rule_pos]);
      SET_P1(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_PURGECHAR:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_TOGGLECASE_REC:
      return -1;
      break;

    case RULE_OP_MANGLE_DUPECHAR_FIRST:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_DUPECHAR_LAST:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_DUPECHAR_ALL:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_SWITCH_FIRST:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_SWITCH_LAST:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_SWITCH_AT:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      SET_P1_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_CHR_SHIFTL:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_CHR_SHIFTR:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_CHR_INCR:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_CHR_DECR:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_REPLACE_NP1:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_REPLACE_NM1:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_DUPEBLOCK_FIRST:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_DUPEBLOCK_LAST:
      SET_NAME(rule, rule_buf[rule_pos]);
      SET_P0_CONV(rule, rule_buf[rule_pos]);
      break;

    case RULE_OP_MANGLE_TITLE:
      SET_NAME(rule, rule_buf[rule_pos]);
      break;

    default:
      return -1;
      break;
    }
  }

  if (rule_pos < rule_len) return -1;

  return 0;
}

int kernel_rule_to_cpu_rule(char *rule_buf, kernel_rule_t *rule)
{
  uint rule_cnt;
  uint rule_pos;
  uint rule_len = HCBUFSIZ - 1; // maximum possible len

  char rule_cmd;

  for (rule_cnt = 0, rule_pos = 0; rule_pos < rule_len && rule_cnt < MAX_KERNEL_RULES; rule_pos++, rule_cnt++)
  {
    GET_NAME(rule);

    if (rule_cnt > 0) rule_buf[rule_pos++] = ' ';

    switch (rule_cmd)
    {
    case RULE_OP_MANGLE_NOOP:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_LREST:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_UREST:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_LREST_UFIRST:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_UREST_LFIRST:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_TREST:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_TOGGLE_AT:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_REVERSE:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_DUPEWORD:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_DUPEWORD_TIMES:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_REFLECT:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_ROTATE_LEFT:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_ROTATE_RIGHT:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_APPEND:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0(rule);
      break;

    case RULE_OP_MANGLE_PREPEND:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0(rule);
      break;

    case RULE_OP_MANGLE_DELETE_FIRST:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_DELETE_LAST:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_DELETE_AT:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_EXTRACT:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      GET_P1_CONV(rule);
      break;

    case RULE_OP_MANGLE_OMIT:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      GET_P1_CONV(rule);
      break;

    case RULE_OP_MANGLE_INSERT:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      GET_P1(rule);
      break;

    case RULE_OP_MANGLE_OVERSTRIKE:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      GET_P1(rule);
      break;

    case RULE_OP_MANGLE_TRUNCATE_AT:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_REPLACE:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0(rule);
      GET_P1(rule);
      break;

    case RULE_OP_MANGLE_PURGECHAR:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0(rule);
      break;

    case RULE_OP_MANGLE_TOGGLECASE_REC:
      return -1;
      break;

    case RULE_OP_MANGLE_DUPECHAR_FIRST:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_DUPECHAR_LAST:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_DUPECHAR_ALL:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_SWITCH_FIRST:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_SWITCH_LAST:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case RULE_OP_MANGLE_SWITCH_AT:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      GET_P1_CONV(rule);
      break;

    case RULE_OP_MANGLE_CHR_SHIFTL:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_CHR_SHIFTR:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_CHR_INCR:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_CHR_DECR:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_REPLACE_NP1:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_REPLACE_NM1:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_DUPEBLOCK_FIRST:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_DUPEBLOCK_LAST:
      rule_buf[rule_pos] = rule_cmd;
      GET_P0_CONV(rule);
      break;

    case RULE_OP_MANGLE_TITLE:
      rule_buf[rule_pos] = rule_cmd;
      break;

    case 0:
      return rule_pos - 1;
      break;

    default:
      return -1;
      break;
    }
  }

  if (rule_cnt > 0)
  {
    return rule_pos;
  }

  return -1;
}

#include "cpu_rules.h"
