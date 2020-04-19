/*
 *  sha1.h
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved
 *
 *****************************************************************************
 *  $Id: sha1.h 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This class implements the Secure Hashing Standard as defined
 *      in FIPS PUB 180-1 published April 17, 1995.
 *
 *      Many of the variable names in the SHA1Context, especially the
 *      single character names, were used because those were the names
 *      used in the publication.
 *
 *      Please read the file sha1.c for more information.
 *
 */

#ifndef _SHA1_H
#define _SHA1_H

#include <stdint.h>

/** 
 *  This structure will hold context information for the hashing
 *  operation
 */
typedef struct SHA1Context
{
	uint32_t Message_Digest[5]; 	/* Message Digest (output)          */
	uint32_t Length_Low;        	/* Message length in bits           */
	uint32_t Length_High;       	/* Message length in bits           */
    uint8_t Message_Block[64]; 	/* 512-bit message blocks      		*/
    uint8_t Message_Block_Index; 	/* Index into message block array   */
    uint8_t Computed;         	/* Is the digest computed?          */
    uint8_t Corrupted;        	/* Is the message digest corruped?  */
} SHA1Context;

void SHA1Reset(SHA1Context *context);
int32_t SHA1Result(SHA1Context *context);
void SHA1Input(SHA1Context *context, uint8_t *message_array, uint32_t length);
void endian_swap(uint32_t *x);

#endif
