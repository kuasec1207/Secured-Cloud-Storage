#include "../GF/GF.h"
#include "../misc.h"
#include <math.h>

uchar _AxBTable[256][256];
uchar _AxBTableOne[256*256];
void createAxBTable();

uint64_t MASK(uint64_t v);
uint64_t SHIFT_BYTES(uint64_t v);
void show64(uint64_t v);

void PQ_x64(uchar n, uchar **msg, size_t bytes);
void PQ_x64_LookupTable(uchar n, uchar **msg, size_t bytes);

void PQ_x64_LookupTable_1(uchar n, uchar *msg, uchar **cw, size_t bytes);

/**
    RAID6_Anvin_Decode
    @param n : disk
    @param msg: msg[0] is Q, [1] is P
    @param bytes: 單一陣列長度
*/
void RAID6_Anvin_Decode(uchar n, uchar **msg, size_t bytes);

/**
    _RAID6DecodeTwoData
    兩個資料錯誤，由'RAID6_Decode()'呼叫
    @param faila : 錯誤硬碟位置
    @param failb : 錯誤硬碟位置

*/
void _RAID6DecodeTwoData(uchar n, uchar **msg, size_t bytes, int faila, int failb);

/**
    _RAID6DecodePData
    一個資料與P錯誤，由'RAID6_Decode()'呼叫
    @param fail : 錯誤硬碟位置
*/
void _RAID6DecodePData(uchar n, uchar **msg, size_t bytes, int fail);

/**
    _RAID6DecodeQData
    一個資料與Q錯誤，由'RAID6_Decode()'呼叫
    @param fail : 錯誤硬碟位置
*/
void _RAID6DecodeQData(uchar n, uchar **msg, size_t bytes, int fail);

/**
    RAID6_Decode
    解碼兩個抹除錯誤
    @param n : 硬碟數量
    @param msg : 代解碼數據
    @param bytes : 單一陣列長度( 長度 msg[0]=msg[1]=msg[2]..  )
    @param faila : 錯誤硬碟位置
    @param failb : 錯誤硬碟位置
*/
void RAID6_Decode(uchar n, uchar **msg, size_t bytes, int faila, int failb);

void _XOR(const uchar *x, const uchar *y, uchar *s, size_t bytes);
