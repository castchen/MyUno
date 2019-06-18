#ifndef __CMD_LIST_H__
#define __CMD_LIST_H__

enum CmdList
{
    kRetCode = 1000,
    kLogin = 1001,
    kLogout = 1002,
};

enum RetCodeList
{
    kCorrect = 0,
    kAccountOrPasswdError = 1,
};

#endif
