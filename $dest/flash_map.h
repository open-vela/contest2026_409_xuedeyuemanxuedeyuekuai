/** 
  ****************************************************************************** 
  * @file   custom_mem_map.h 
  * @author Sifli software development team 
  ****************************************************************************** 
*/ 
/** 
 * @attention 
 * Copyright (c) 2019 - 2022,  Sifli Technology 
 * 
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this 
 *    list of conditions and the following disclaimer. 
 * 
 * 2. Redistributions in binary form, except as embedded into a Sifli integrated circuit 
 *    in a product or a software update for such product, must reproduce the above 
 *    copyright notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 
 * 3. Neither the name of Sifli nor the names of its contributors may be used to endorse 
 *    or promote products derived from this software without specific prior written permission. 
 * 
 * 4. This software, with or without modification, must only be used with a 
 *    Sifli integrated circuit. 
 * 
 * 5. Any software provided in binary form under this license must not be reverse 
 *    engineered, decompiled, modified and/or disassembled. 
 * 
 * THIS SOFTWARE IS PROVIDED BY SIFLI TECHNOLOGY "AS IS" AND ANY EXPRESS 
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL SIFLI TECHNOLOGY OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 * 
 */ 

#ifndef _FLASH_MAP_H_
#define _FLASH_MAP_H_


#define FLASH_PART_PATH(id)              FLASH_PART##id##_PATH
#define FLASH_PART_RESET(id)             FLASH_PART##id##_RESET
#define FLASH_PART_REAL_ADDR(id)         FLASH_PART##id##_REAL_ADDR


#undef  HCPU_RO_DATA_SIZE
#define HCPU_RO_DATA_SIZE                     0

#undef  FLASH_TABLE_START_ADDR
#define FLASH_TABLE_START_ADDR                0x12000000

#undef  FLASH_TABLE_SIZE
#define FLASH_TABLE_SIZE                      0x4000

#undef  FLASH_BOOT_LOADER_START_ADDR
#define FLASH_BOOT_LOADER_START_ADDR          0x12004000

#undef  FLASH_BOOT_LOADER_SIZE
#define FLASH_BOOT_LOADER_SIZE                0xE000

#undef  BOOTLOADER_XIP_ADDR
#define BOOTLOADER_XIP_ADDR                   0x20020000

#undef  BOOTLOADER_RAM_DATA_START_ADDR
#define BOOTLOADER_RAM_DATA_START_ADDR        0x20030000

#undef  BOOTLOADER_RAM_DATA_SIZE
#define BOOTLOADER_RAM_DATA_SIZE              0xE000

#undef  NMI_SEC_CODE_START_ADDR
#define NMI_SEC_CODE_START_ADDR               0x601F7000

#undef  NMI_SEC_CODE_SIZE
#define NMI_SEC_CODE_SIZE                     0x8C00

#undef  NMI_SEC_SHARE_START_ADDR
#define NMI_SEC_SHARE_START_ADDR              0x601FFC00

#undef  NMI_SEC_SHARE_SIZE
#define NMI_SEC_SHARE_SIZE                    0x400

#define RES_IMAGES_PATH                       "/images/"
#define RES_FONT_PATH                         "/fonts/"
#define APP_IMAGES_FS_PATH                    "/images_fs/"
#define APP_DYNAMIC_APP_PATH                  "/dynamic_app/"
#define APP_LANG_PATH                         "/language/"
#define SUPPORT_SCRIPT_PATH                   "/support_script/"
#define APP_CFG_PATH                          "/app_config/"
#define MUSIC_PATH                            "/music/"
#define RING_PATH                             "/ring/"
#define OTA_RES_PATH                          "/dyn/ota_res_diff/"
#define BUILT_IN_APP_PATH                     "/built_in_app/"
#define ALIPAY_DATA_PATH                      "/alipay_transport/"
#define TEE_SEC_DATA_PATH                     "/tee_sec_data/"
#define GUI_TOOL_PATH                         "/gui_tool/"

#define FLASH_PART0_NAME                 "upay"
#define FLASH_PART0_DEVICE               "flash2"
#define FLASH_PART0_PATH                 ""
#define FLASH_PART0_RESET                1
#define FLASH_PART0_OFFSET               0x12000
#define FLASH_PART0_BASE_ADDR            0x12012000
#define FLASH_PART0_REAL_ADDR            0x12012000
#define FLASH_PART0_SIZE                 0x4000


#define FLASH_PART1_NAME                 "upay_safety"
#define FLASH_PART1_DEVICE               "flash2"
#define FLASH_PART1_PATH                 ""
#define FLASH_PART1_RESET                0
#define FLASH_PART1_OFFSET               0x16000
#define FLASH_PART1_BASE_ADDR            0x12016000
#define FLASH_PART1_REAL_ADDR            0x12016000
#define FLASH_PART1_SIZE                 0x2000


#define FLASH_PART2_NAME                 "dfu"
#define FLASH_PART2_DEVICE               "flash2"
#define FLASH_PART2_PATH                 ""
#define FLASH_PART2_RESET                1
#define FLASH_PART2_OFFSET               0x18000
#define FLASH_PART2_BASE_ADDR            0x12018000
#define FLASH_PART2_REAL_ADDR            0x12018000
#define FLASH_PART2_SIZE                 0x4000


#define FLASH_PART3_NAME                 "ble"
#define FLASH_PART3_DEVICE               "flash2"
#define FLASH_PART3_PATH                 ""
#define FLASH_PART3_RESET                1
#define FLASH_PART3_OFFSET               0x1C000
#define FLASH_PART3_BASE_ADDR            0x1201c000
#define FLASH_PART3_REAL_ADDR            0x1201c000
#define FLASH_PART3_SIZE                 0x4000


#define FLASH_PART4_NAME                 "app"
#define FLASH_PART4_DEVICE               "flash2"
#define FLASH_PART4_PATH                 ""
#define FLASH_PART4_RESET                1
#define FLASH_PART4_OFFSET               0x20000
#define FLASH_PART4_BASE_ADDR            0x12020000
#define FLASH_PART4_REAL_ADDR            0x12020000
#define FLASH_PART4_SIZE                 0x30000


#define FLASH_PART5_NAME                 "msg"
#define FLASH_PART5_DEVICE               "flash2"
#define FLASH_PART5_PATH                 ""
#define FLASH_PART5_RESET                1
#define FLASH_PART5_OFFSET               0x50000
#define FLASH_PART5_BASE_ADDR            0x12050000
#define FLASH_PART5_REAL_ADDR            0x12050000
#define FLASH_PART5_SIZE                 0x0


#define FLASH_PART6_NAME                 "contact"
#define FLASH_PART6_DEVICE               "flash2"
#define FLASH_PART6_PATH                 ""
#define FLASH_PART6_RESET                1
#define FLASH_PART6_OFFSET               0x50000
#define FLASH_PART6_BASE_ADDR            0x12050000
#define FLASH_PART6_REAL_ADDR            0x12050000
#define FLASH_PART6_SIZE                 0x8000


#define FLASH_PART7_NAME                 "his_hr"
#define FLASH_PART7_DEVICE               "flash2"
#define FLASH_PART7_PATH                 ""
#define FLASH_PART7_RESET                1
#define FLASH_PART7_OFFSET               0x58000
#define FLASH_PART7_BASE_ADDR            0x12058000
#define FLASH_PART7_REAL_ADDR            0x12058000
#define FLASH_PART7_SIZE                 0x4000


#define FLASH_PART8_NAME                 "his_sport"
#define FLASH_PART8_DEVICE               "flash2"
#define FLASH_PART8_PATH                 ""
#define FLASH_PART8_RESET                1
#define FLASH_PART8_OFFSET               0x5C000
#define FLASH_PART8_BASE_ADDR            0x1205c000
#define FLASH_PART8_REAL_ADDR            0x1205c000
#define FLASH_PART8_SIZE                 0x4000


#define FLASH_PART9_NAME                 "his_sleep"
#define FLASH_PART9_DEVICE               "flash2"
#define FLASH_PART9_PATH                 ""
#define FLASH_PART9_RESET                1
#define FLASH_PART9_OFFSET               0x60000
#define FLASH_PART9_BASE_ADDR            0x12060000
#define FLASH_PART9_REAL_ADDR            0x12060000
#define FLASH_PART9_SIZE                 0x4000


#define FLASH_PART10_NAME                "his_call_log"
#define FLASH_PART10_DEVICE              "flash2"
#define FLASH_PART10_PATH                ""
#define FLASH_PART10_RESET               1
#define FLASH_PART10_OFFSET              0x64000
#define FLASH_PART10_BASE_ADDR           0x12064000
#define FLASH_PART10_REAL_ADDR           0x12064000
#define FLASH_PART10_SIZE                0x4000


#define FLASH_PART11_NAME                "dfu_mng_code"
#define FLASH_PART11_DEVICE              "flash2"
#define FLASH_PART11_PATH                ""
#define FLASH_PART11_RESET               0
#define FLASH_PART11_OFFSET              0x00068000
#define FLASH_PART11_BASE_ADDR           0x12068000
#define FLASH_PART11_REAL_ADDR           0x12068000
#define FLASH_PART11_SIZE                0x80000


#define FLASH_PART12_NAME                "solution_code"
#define FLASH_PART12_DEVICE              "flash2"
#define FLASH_PART12_PATH                ""
#define FLASH_PART12_RESET               0
#define FLASH_PART12_OFFSET              0xE8000
#define FLASH_PART12_BASE_ADDR           0x120e8000
#define FLASH_PART12_REAL_ADDR           0x120e8000
#define FLASH_PART12_SIZE                0x400000


#define FLASH_PART13_NAME                "img"
#define FLASH_PART13_DEVICE              "flash2"
#define FLASH_PART13_PATH                ""
#define FLASH_PART13_RESET               0
#define FLASH_PART13_OFFSET              0x4E8000
#define FLASH_PART13_BASE_ADDR           0x124e8000
#define FLASH_PART13_REAL_ADDR           0x124e8000
#define FLASH_PART13_SIZE                0x1e0000


#define FLASH_PART14_NAME                "font"
#define FLASH_PART14_DEVICE              "flash2"
#define FLASH_PART14_PATH                ""
#define FLASH_PART14_RESET               0
#define FLASH_PART14_OFFSET              0x6C8000
#define FLASH_PART14_BASE_ADDR           0x126c8000
#define FLASH_PART14_REAL_ADDR           0x126c8000
#define FLASH_PART14_SIZE                0x1b0000


#define FLASH_PART15_NAME                "tiny_font"
#define FLASH_PART15_DEVICE              "flash2"
#define FLASH_PART15_PATH                ""
#define FLASH_PART15_RESET               0
#define FLASH_PART15_OFFSET              0x878000
#define FLASH_PART15_BASE_ADDR           0x12878000
#define FLASH_PART15_REAL_ADDR           0x12878000
#define FLASH_PART15_SIZE                0x0


#define FLASH_PART16_NAME                "image_ota"
#define FLASH_PART16_DEVICE              "flash2"
#define FLASH_PART16_PATH                ""
#define FLASH_PART16_RESET               0
#define FLASH_PART16_OFFSET              0x878000
#define FLASH_PART16_BASE_ADDR           0x12878000
#define FLASH_PART16_REAL_ADDR           0x12878000
#define FLASH_PART16_SIZE                0x0


#define FLASH_PART17_NAME                "dfu_download_buffer"
#define FLASH_PART17_DEVICE              "flash2"
#define FLASH_PART17_PATH                ""
#define FLASH_PART17_RESET               0
#define FLASH_PART17_OFFSET              0x878000
#define FLASH_PART17_BASE_ADDR           0x12878000
#define FLASH_PART17_REAL_ADDR           0x12878000
#define FLASH_PART17_SIZE                0x0


#define FLASH_PART18_NAME                "root"
#define FLASH_PART18_DEVICE              "flash2"
#define FLASH_PART18_PATH                "/"
#define FLASH_PART18_RESET               0
#define FLASH_PART18_OFFSET              0x878000
#define FLASH_PART18_BASE_ADDR           0x12878000
#define FLASH_PART18_REAL_ADDR           0x12878000
#define FLASH_PART18_SIZE                0x788000


#define FLASH_PART19_NAME                "dyn"
#define FLASH_PART19_DEVICE              "flash2"
#define FLASH_PART19_PATH                "/dyn"
#define FLASH_PART19_RESET               0
#define FLASH_PART19_OFFSET              0x1000000
#define FLASH_PART19_BASE_ADDR           0x13000000
#define FLASH_PART19_REAL_ADDR           0x13000000
#define FLASH_PART19_SIZE                0x0


#define FLASH_PART20_NAME                "music"
#define FLASH_PART20_DEVICE              "flash2"
#define FLASH_PART20_PATH                "/music"
#define FLASH_PART20_RESET               0
#define FLASH_PART20_OFFSET              0x1000000
#define FLASH_PART20_BASE_ADDR           0x13000000
#define FLASH_PART20_REAL_ADDR           0x13000000
#define FLASH_PART20_SIZE                0x0


#define FLASH_PART21_NAME                "video"
#define FLASH_PART21_DEVICE              "flash2"
#define FLASH_PART21_PATH                "/video"
#define FLASH_PART21_RESET               0
#define FLASH_PART21_OFFSET              0x1000000
#define FLASH_PART21_BASE_ADDR           0x13000000
#define FLASH_PART21_REAL_ADDR           0x13000000
#define FLASH_PART21_SIZE                0x0


#define FLASH_PART22_NAME                "debug_assert"
#define FLASH_PART22_DEVICE              "sd0"
#define FLASH_PART22_PATH                "/dyn/debug/"
#define DEBUG_ASSERT_PATH                "/dyn/debug/"
#define FLASH_PART22_RESET               1
#define FLASH_PART22_OFFSET              0x0
#define FLASH_PART22_BASE_ADDR           0x62000000
#define FLASH_PART22_REAL_ADDR           0x62000000
#define FLASH_PART22_SIZE                0x80000


#define FLASH_PART23_NAME                "debug_log"
#define FLASH_PART23_DEVICE              "sd0"
#define FLASH_PART23_PATH                "/dyn/debug/"
#define DEBUG_LOG_PATH                   "/dyn/debug/"
#define FLASH_PART23_RESET               1
#define FLASH_PART23_OFFSET              0x0
#define FLASH_PART23_BASE_ADDR           0x62000000
#define FLASH_PART23_REAL_ADDR           0x62000000
#define FLASH_PART23_SIZE                0x40000


#define FLASH_PART24_NAME                "debug_metrics"
#define FLASH_PART24_DEVICE              "sd0"
#define FLASH_PART24_PATH                "/dyn/debug/"
#define DEBUG_METRICS_PATH               "/dyn/debug/"
#define FLASH_PART24_RESET               1
#define FLASH_PART24_OFFSET              0x0
#define FLASH_PART24_BASE_ADDR           0x62000000
#define FLASH_PART24_REAL_ADDR           0x62000000
#define FLASH_PART24_SIZE                0x40000


#define FLASH_PART25_NAME                "debug_monkey"
#define FLASH_PART25_DEVICE              "sd0"
#define FLASH_PART25_PATH                "/dyn/debug/"
#define DEBUG_MONKEY_PATH                "/dyn/debug/"
#define FLASH_PART25_RESET               1
#define FLASH_PART25_OFFSET              0x0
#define FLASH_PART25_BASE_ADDR           0x62000000
#define FLASH_PART25_REAL_ADDR           0x62000000
#define FLASH_PART25_SIZE                0x20000


#define FLASH_PART26_NAME                "external_sd"
#define FLASH_PART26_DEVICE              "sd0"
#define FLASH_PART26_PATH                "/external_sd"
#define FLASH_PART26_RESET               0
#define FLASH_PART26_OFFSET              0x0
#define FLASH_PART26_BASE_ADDR           0x0
#define FLASH_PART26_REAL_ADDR           0x0
#define FLASH_PART26_SIZE                0x1f79000


#define FAL_PART_TABLE            \
        {                         \
            FAL_PART_DEF(0),      \
            FAL_PART_DEF(1),      \
            FAL_PART_DEF(2),      \
            FAL_PART_DEF(3),      \
            FAL_PART_DEF(4),      \
            FAL_PART_DEF(6),      \
            FAL_PART_DEF(7),      \
            FAL_PART_DEF(8),      \
            FAL_PART_DEF(9),      \
            FAL_PART_DEF(10),     \
            FAL_PART_DEF(11),     \
            FAL_PART_DEF(12),     \
            FAL_PART_DEF(13),     \
            FAL_PART_DEF(14),     \
            FAL_PART_DEF(18),     \
            FAL_PART_DEF(22),     \
            FAL_PART_DEF(23),     \
            FAL_PART_DEF(24),     \
            FAL_PART_DEF(25),     \
        }


#define FAL_PART_TABLE2           \
        {                         \
            FAL_PART_DEF(26),     \
        }


#define FS_MOUNT_TABLE            \
{                                 \
    .device_name = "root",        \
    .path = "/",                  \
    .filesystemtype = "lfs",      \
    .rwflag = 0,                  \
    .data = 0,                    \
},                                \
{                                 \
    .device_name = "external_sd", \
    .path = "/external_sd",       \
    .filesystemtype = "elm",      \
    .rwflag = 0,                  \
    .data = 0,                    \
},


#undef  DFU_FLASH_CODE_START_ADDR
#undef  DFU_FLASH_CODE_BURN_ADDR
#undef  DFU_FLASH_CODE_SIZE
#undef  DFU_FLASH_CODE_END_ADDR
#undef  DFU_FLASH_CODE_BASE_ADDR
#define DFU_FLASH_CODE_START_ADDR                  FLASH_PART_BASE_ADDR(11)
#define DFU_FLASH_CODE_BURN_ADDR                   FLASH_PART_REAL_ADDR(11)
#define DFU_FLASH_CODE_SIZE                        FLASH_PART_SIZE(11)
#define DFU_FLASH_CODE_END_ADDR                    (END_ADDR(DFU_FLASH_CODE_START_ADDR, DFU_FLASH_CODE_SIZE))


#undef  HCPU_FLASH_CODE_START_ADDR
#undef  HCPU_FLASH_CODE_BURN_ADDR
#undef  HCPU_FLASH_CODE_SIZE
#undef  HCPU_FLASH_CODE_END_ADDR
#undef  HCPU_FLASH_CODE_BASE_ADDR
#define HCPU_FLASH_CODE_START_ADDR                 FLASH_PART_BASE_ADDR(12)
#define HCPU_FLASH_CODE_BURN_ADDR                  FLASH_PART_REAL_ADDR(12)
#define HCPU_FLASH_CODE_SIZE                       FLASH_PART_SIZE(12)
#define HCPU_FLASH_CODE_END_ADDR                   (END_ADDR(HCPU_FLASH_CODE_START_ADDR, HCPU_FLASH_CODE_SIZE))


#undef  HCPU_FLASH2_IMG_START_ADDR
#undef  HCPU_FLASH2_IMG_BURN_ADDR
#undef  HCPU_FLASH2_IMG_SIZE
#undef  HCPU_FLASH2_IMG_END_ADDR
#undef  HCPU_FLASH2_IMG_BASE_ADDR
#define HCPU_FLASH2_IMG_START_ADDR                 FLASH_PART_BASE_ADDR(13)
#define HCPU_FLASH2_IMG_BURN_ADDR                  FLASH_PART_REAL_ADDR(13)
#define HCPU_FLASH2_IMG_SIZE                       FLASH_PART_SIZE(13)
#define HCPU_FLASH2_IMG_END_ADDR                   (END_ADDR(HCPU_FLASH2_IMG_START_ADDR, HCPU_FLASH2_IMG_SIZE))


#undef  HCPU_FLASH2_FONT_START_ADDR
#undef  HCPU_FLASH2_FONT_BURN_ADDR
#undef  HCPU_FLASH2_FONT_SIZE
#undef  HCPU_FLASH2_FONT_END_ADDR
#undef  HCPU_FLASH2_FONT_BASE_ADDR
#define HCPU_FLASH2_FONT_START_ADDR                FLASH_PART_BASE_ADDR(14)
#define HCPU_FLASH2_FONT_BURN_ADDR                 FLASH_PART_REAL_ADDR(14)
#define HCPU_FLASH2_FONT_SIZE                      FLASH_PART_SIZE(14)
#define HCPU_FLASH2_FONT_END_ADDR                  (END_ADDR(HCPU_FLASH2_FONT_START_ADDR, HCPU_FLASH2_FONT_SIZE))


#undef  HCPU_FLASH2_TINY_FONT_START_ADDR
#undef  HCPU_FLASH2_TINY_FONT_BURN_ADDR
#undef  HCPU_FLASH2_TINY_FONT_SIZE
#undef  HCPU_FLASH2_TINY_FONT_END_ADDR
#undef  HCPU_FLASH2_TINY_FONT_BASE_ADDR
#define HCPU_FLASH2_TINY_FONT_START_ADDR           FLASH_PART_BASE_ADDR(15)
#define HCPU_FLASH2_TINY_FONT_BURN_ADDR            FLASH_PART_REAL_ADDR(15)
#define HCPU_FLASH2_TINY_FONT_SIZE                 FLASH_PART_SIZE(15)
#define HCPU_FLASH2_TINY_FONT_END_ADDR             (END_ADDR(HCPU_FLASH2_TINY_FONT_START_ADDR, HCPU_FLASH2_TINY_FONT_SIZE))


#undef  HCPU_FLASH2_IMG_UPGRADE_START_ADDR
#undef  HCPU_FLASH2_IMG_UPGRADE_BURN_ADDR
#undef  HCPU_FLASH2_IMG_UPGRADE_SIZE
#undef  HCPU_FLASH2_IMG_UPGRADE_END_ADDR
#undef  HCPU_FLASH2_IMG_UPGRADE_BASE_ADDR
#define HCPU_FLASH2_IMG_UPGRADE_START_ADDR         FLASH_PART_BASE_ADDR(16)
#define HCPU_FLASH2_IMG_UPGRADE_BURN_ADDR          FLASH_PART_REAL_ADDR(16)
#define HCPU_FLASH2_IMG_UPGRADE_SIZE               FLASH_PART_SIZE(16)
#define HCPU_FLASH2_IMG_UPGRADE_END_ADDR           (END_ADDR(HCPU_FLASH2_IMG_UPGRADE_START_ADDR, HCPU_FLASH2_IMG_UPGRADE_SIZE))


#undef  DFU_RES_FLASH_CODE_START_ADDR
#undef  DFU_RES_FLASH_CODE_BURN_ADDR
#undef  DFU_RES_FLASH_CODE_SIZE
#undef  DFU_RES_FLASH_CODE_END_ADDR
#undef  DFU_RES_FLASH_CODE_BASE_ADDR
#define DFU_RES_FLASH_CODE_START_ADDR              FLASH_PART_BASE_ADDR(17)
#define DFU_RES_FLASH_CODE_BURN_ADDR               FLASH_PART_REAL_ADDR(17)
#define DFU_RES_FLASH_CODE_SIZE                    FLASH_PART_SIZE(17)
#define DFU_RES_FLASH_CODE_END_ADDR                (END_ADDR(DFU_RES_FLASH_CODE_START_ADDR, DFU_RES_FLASH_CODE_SIZE))


#undef  HCPU_FS_ROOT_START_ADDR
#undef  HCPU_FS_ROOT_BURN_ADDR
#undef  HCPU_FS_ROOT_SIZE
#undef  HCPU_FS_ROOT_END_ADDR
#undef  HCPU_FS_ROOT_BASE_ADDR
#define HCPU_FS_ROOT_START_ADDR                    FLASH_PART_BASE_ADDR(18)
#define HCPU_FS_ROOT_BURN_ADDR                     FLASH_PART_REAL_ADDR(18)
#define HCPU_FS_ROOT_SIZE                          FLASH_PART_SIZE(18)
#define HCPU_FS_ROOT_END_ADDR                      (END_ADDR(HCPU_FS_ROOT_START_ADDR, HCPU_FS_ROOT_SIZE))


#undef  PSRAM_DATA_START_ADDR
#undef  PSRAM_DATA_SIZE
#define PSRAM_DATA_START_ADDR                      0x60000000
#define PSRAM_DATA_SIZE                            0x00800000


#undef  PSRAM_BASE
#undef  PSRAM_SIZE
#define PSRAM_BASE                  (0x60000000)
#define PSRAM_SIZE                  (0x00800000)

#undef  PSRAM2_BASE_ADDR
#undef  PSRAM2_SIZE

#endif /* _FLASH_MAP_H_ */

