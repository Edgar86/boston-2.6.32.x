/**+===========================================================================
  File: alog_ram_console.h

  Description:


  Note:


  Author:
        Bryan Hsieh May-30-2010
-------------------------------------------------------------------------------
** FIHSPEC CONFIDENTIAL
** Copyright(c) 2009 FIHSPEC Corporation. All Rights Reserved.
**^M
** The source code contained or described herein and all documents related
** to the source code (Material) are owned by FIHSPEC Technology Corporation.
** The Material is protected by worldwide copyright and trade secret laws and
** treaty provisions. No part of the Material may be used, copied, reproduced,
** modified, published, uploaded, posted, transmitted, distributed, or disclosed
** in any way without FIHSPEC prior express written permission.
============================================================================+*/

typedef enum  {
	LOG_TYPE_MAIN = 0,
	LOG_TYPE_RADIO,
	LOG_TYPE_EVENTS,
//SW2-5-2-MP-DbgCfgTool-02+[
	LOG_TYPE_SYSTEM,
//SW2-5-2-MP-DbgCfgTool-02+]
	LOG_TYPE_NUM,
	LOG_TYPE_ALL = 0xFF,
} LogType;

typedef enum  {
	SYNC_BEFORE = 0,
	SYNC_AFTER
} SyncType;
 
int alog_ram_console_write_log(const LogType log_type, char * s, int count);


void alog_ram_console_write_entry(const LogType log_type,
							char *priority,
							char * tag, int tag_bytes,
							char * msg, int msg_bytes);

void alog_ram_console_sync_time(const LogType log_type, const SyncType sync_type);

