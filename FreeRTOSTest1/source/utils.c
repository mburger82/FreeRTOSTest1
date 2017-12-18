//
// File: utils.c
//
// xmega demo program VG Feb,Mar,Oct 2011
//
// CPU:   atxmega32A4, atxmega128a1
//
// Version: 1.2.1
//
#include "all.h"
#include "string.h"

resetReason_t getResetReason(void) {
	resetReason_t returnValue = RESETREASON_POWERONRESET;
	// software reset ?
	if( RST.STATUS & RST_SRF_bm )
	{
		// reset this bit
		RST.STATUS = RST_SRF_bm;
		returnValue = RESETREASON_SOFTWARERESET;
	}
	// power on reset ?
	else if( RST.STATUS & RST_PORF_bm)
	{
		// reset this bit
		RST.STATUS = RST_PORF_bm;
		returnValue = RESETREASON_POWERONRESET;
	}
	// debugger reset ?
	else if( RST.STATUS & RST_PDIRF_bm)
	{
		// reset this bit
		RST.STATUS = RST_PDIRF_bm;
		returnValue = RESETREASON_DEBUGGERRESET;
	}
	// external reset ?
	else if( RST.STATUS & RST_EXTRF_bm)
	{
		// reset this bit
		RST.STATUS = RST_EXTRF_bm;
		returnValue = RESETREASON_EXTERNALRESET;
	}
	return returnValue;
}

void insert_c(char* str,u8 pos,char c)
{
	u8 len=strlen(str);
	u8 i;

	if(str<(char *)0x2000 && str>=(char *)0x3000)
		error(ERR_INVALID_STRING_ADDRESS);	

	if(pos>len)
		return;

	for(i=0;i<len-pos;i++)
		str[len-i] = str[len-1-i];

	str[pos] = c;
	str[len+1] = '\0';	
}

// As a preparation, write a '\0' at the end of the string.
// If you have done this, you may use this function to check 
// whether the 0 is still there (or overwritten).
void strPrep(char* str,u8 len)
{
	// is the string in the allowed ram address range?
	if(str>=(char *)0x2000 && str<(char *)0x3000)
		str[len-1] = '\0';
	else
		error(ERR_INVALID_STRING_ADDRESS);	
}

void strCheck(char* str,u8 len)
{
	if(str[len-1] != '\0')
		error(ERR_STRING_OVERFLOW);
			
	if(!(str>=(char *)0x2000 && str<(char *)0x3000))
		error(ERR_INVALID_STRING_ADDRESS);	
}

// Calculate the checksum of a datagram.
// The string must be without the leading '$' and trailing '*' !
u8 calcChksum(char* datagramStr)
{
	u8 chksum=0;
	u8 i=0;
	u8 len = strlen(datagramStr);
	for(i=0;i<len;i++)
	{
		chksum ^= datagramStr[i];
	}
	return chksum;
}

// datagram state machine handlers
void sm_init(dgStateMaschine_t* sm)
{
	sm->state = outside;
}

static bool internalSetState(dgStateMaschine_t* sm, char cChar);

// returns false if the checksum is not as expected.
bool sm_setState(dgStateMaschine_t* sm, char cChar)
{
	return internalSetState(sm, cChar);
}

static bool internalSetState(dgStateMaschine_t* sm, char cChar)
{
	switch(sm->state)
	{
		case outside:
			if( cChar=='$' || cChar=='!' )
			{
				sm->chksum = 0; // '$' is not part of the check sum, start with new checksum calc here!
				sm->state  = inside ;	
			}
		break;
		case inside:			
			if( cChar == '*' )
				// '*' is not in the check sum, omit checksum calc here!
				sm->state = eom; // next 2 chars are the checksum		 
			else
				sm->chksum ^= cChar;
			if( cChar == '\r' || cChar == '\n' )
				sm->state = outside;
		break;
		case eom:	
			(sm->chkstr)[0] = cChar;	
			sm->state = chks1;		 
		break;
		case chks1:
			(sm->chkstr)[1] = cChar;
			(sm->chkstr)[2] = '\0';
			sm->state = outside; // not inside a datagram anymore
		
			u16 cs = strtoul(sm->chkstr,NULL,16);
		 	if( (u8)(sm->chksum) != cs )
				return false;
		break;
		default:
			sm->state = outside;
		break;
	}
	return true; // everything OK, including checksum
}

// EOF utils.c

