#ifndef __CMD_LIST_H__
#define __CMD_LIST_H__

enum CmdList
{
    kCmdRetCode = 1000,
    kCmdLogin = 1001,
    kCmdLogout = 1002,
};

enum RetCodeList
{
    kRetCodeCorrect = 0,
    kRetCodeAccountOrPasswdError = 1,
};

#endif
