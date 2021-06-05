#pragma once

// https://docs.microsoft.com/ru-ru/windows/win32/menurc/versioninfo-resource?redirectedfrom=MSDN

#define APP_INFO_PRODUCTVERSION         0,15,1,0
#define APP_INFO_PRODUCTVERSION_STR     "0.15.1"

#define APP_INFO_FILEVERSION            APP_INFO_PRODUCTVERSION
#define APP_INFO_FILEVERSION_STR        APP_INFO_PRODUCTVERSION_STR

#define APP_INFO_PRODUCTNAME_STR        "AxelChat"
#define APP_INFO_COMPANYNAME_STR        "Axel_k, 3dproger"
#define APP_INFO_FILEDESCRIPTION_STR    APP_INFO_PRODUCTNAME_STR
#define APP_INFO_INTERNALNAME_STR       APP_INFO_PRODUCTNAME_STR
#define APP_INFO_LEGALCOPYRIGHT_STR     "Copyright © 2020-2021 " APP_INFO_COMPANYNAME_STR
#define APP_INFO_LEGALTRADEMARKS1_STR   "All Rights Reserved"
#define APP_INFO_LEGALTRADEMARKS2_STR   APP_INFO_LEGALTRADEMARKS1_STR
#define APP_INFO_ORIGINALFILENAME_STR   "AxelChat.exe"

#define APP_INFO_COMPANYDOMAIN_STR      "https://www.youtube.com/channel/UCujkj8ZgVkscm34GA1Z8wTQ"

                                        //  English (USA),      Russian(Russia)
#define APP_INFO_RC_TRANSLATION             0x409, 1200,        0x0419, 1200
