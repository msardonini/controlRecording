/**
 * @file messageStructure
 * @brief Header to contain the messsage structure used for communications between the host and the remote
 *
 * @author Mike Sardonini
 * @date 09/10/2018
 */

#include "string.h"

#define MAGIC_H1 0xAA
#define MAGIC_H2 0xF7
#define MAGIC_F1 0x8B
#define MAGIC_F2 0x4E

#define MODE_RECORDING 0x05
#define MODE_STANDBY 0x00


typedef struct messageStructure_t
{
	uint8_t magicHeader1;
	uint8_t magicHeader2;
	uint64_t timestamp_us;
	uint8_t isCommandMsg;
	uint8_t isStatusMsg;
	uint8_t mode;
	uint8_t magicFooter1;
	uint8_t magicFooter2;

}messageStructure_t;