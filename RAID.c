include "RAID.h"
#include <time.h>

void createAxBTable(){
	for(int a=0;a<256;a++){
		for(int b=0;b<256;b++){
            _AxBTable[a][b] = gf_multi_table(a, 2)^b;
            _AxBTableOne[(a<<8)+b] = _AxBTable[a][b];
		}
    }
}

/* 製作遮罩 */
uint64_t MASK(uint64_t v)
{
    v &= 0x8080808080808080; /* Extract the top bits */
    return (v<<1)-(v>>7); /* Overflow on the top bit is OK */
}

/* 向左移位(*2) */
uint64_t SHIFT_BYTES(uint64_t v){
    return (v<<1)&0xfefefefefefefefe;
}

/* 顯示64Bit */
void show64(uint64_t v)
{
    int i;
    printf("0x");
    for(i=7;i>=0;i--){
        printf("%02X ", (v>>(i*8)&0xff));
    }
//    printf("\n");
}

/**
    PQ_x64 八個一組計算 PQ
    n =>
    msg => 待編碼數據
    bytes => msg of Size
*/
void PQ_x64(uchar n, uchar **msg, size_t bytes){
    uint64_t wtmp;
    uint64_t **d = (uint64_t**)msg;
    uint64_t *q = d[0];
    uint64_t *p = d[1];
    uint64_t *wd;
    int zlast = n-1;

    for(register int i=0;i<bytes;i+=8, p++, q++)
    {
        *p = *q = *((uint64_t*)(&msg[zlast][i]));
        for(register int z=zlast-1;z>1;z--)// disk
        {
            wd = ((uint64_t*)(&msg[z][i]));
            /* q */
            wtmp = MASK(*q)&0x1d1d1d1d1d1d1d1d;
            *q  = (*q<<1)&0xfefefefefefefefe;
            *q ^= wtmp;

            *q ^= *wd;//q
            /* q */
            *p ^= *wd; //p
        }
    }
}

/**
    PQ_x64_LookupTable
    n =>
    msg => 待編碼數據
    bytes => msg of Size
*/
void PQ_x64_LookupTable(uchar n, uchar **msg, size_t bytes){
    uchar *qPtr = msg[0];
    uint64_t *p    = ((uint64_t*)(&msg[1][0]));
    uint64_t *qx64 = ((uint64_t*)(&msg[0][0]));
    uchar *mul2 = _mul[2];
    uchar *mul4 = _mul[4];
    int c = n%2;
    int zlast1 = n-1;
    int zlast2 = n-2;
    uchar *LastPtr1 = msg[zlast1];
    uchar *LastPtr2 = msg[zlast2];
    uchar *Last1;
    uchar *Last2;
    uchar *qtmpPtr;

    register int byte, z;
    for(register int i=0;i<bytes;i+=8,p++, qx64++)
    {
        *p = (*((uint64_t*)(&msg[zlast1][i])))^
             (*((uint64_t*)(&msg[zlast2][i])));

        /* q (Ax+B) */
        for(byte=0, qPtr=qx64;byte<8;byte++)
            *qPtr++ = _AxBTable[*LastPtr1++][*LastPtr2++];

        /* k>4 */
        for(z=zlast2-1;z>2;z-=2)
        {
            /* *(X^2) */
            qtmpPtr = qx64;
            qPtr    = qx64;
            for(byte=0;byte<8;byte++, qPtr++)
                *qPtr = mul4[*qtmpPtr++];

            /* input data */
            *p ^= (*((uint64_t*)(&msg[z][i]  )))^
                  (*((uint64_t*)(&msg[z-1][i])));

            /* q */
            Last1 = &msg[z][i];
            Last2 = &msg[z-1][i];
            qPtr  = qx64;
            for(byte=0;byte<8;byte++)
                *qPtr++ ^= _AxBTable[*Last1++][*Last2++];
        }

        if(c) /* 單數 */
        { /* (m2X+m1)*2+m0 */
            qPtr=qx64;
            for(byte=0;byte<8;byte++, qPtr++)
                *qPtr = mul2[*qPtr];
            /* q */
            *qx64 ^= *((uint64_t*)(&msg[2][i])); // [2] is d0
            *p    ^= *((uint64_t*)(&msg[2][i]));
        }
    }
}

void PQ_x64_LookupTable_1(uchar n, uchar *msg, uchar **cw, size_t bytes)
{
    uchar *q = cw[0];
    uchar *p = cw[1];
    uchar *_one = msg;
//    bytes = bytes / 2;
    for(register int i=0;i<bytes;i+=2, _one+=2)
//    for(register int i=0;i<bytes;i++, _one+=2)
    {
        *p++ = *_one ^ *(_one+1);
        *q++ = _AxBTableOne[*((unsigned short *)_one)];
//        *q++ = _AxBTableOne[(*_one <<8) | *(_one+1)];
    }
}

void RAID6_Anvin_Decode(uchar n, uchar **msg, size_t bytes){
    uchar z;
    uchar **tmp_msg = safe_malloc(uchar*, n);
    tmp_msg[0] = safe_malloc(uchar, bytes);
    tmp_msg[1] = safe_malloc(uchar, bytes);
    for(int i=2;i<n;i++){
        tmp_msg[i] = msg[i];
    }

    uchar *pBytePtr = tmp_msg[1];
    uchar *qBytePtr = tmp_msg[0];
    uint64_t *pPtr = (uint64_t*)tmp_msg[1];
    uint64_t *qPtr = (uint64_t*)tmp_msg[0];

    uchar pp, qq, loc;

    /* Get PQ */
    PQ_x64(n, tmp_msg, bytes);
//    printArray(n, tmp_msg, bytes);

    for(register int i=0;i<bytes;)
    {
        *pPtr++ ^= *((uint64_t*)&msg[1][i]);
        *qPtr++ ^= *((uint64_t*)&msg[0][i]);
        z = 8;
        while(z--)
        {
//            printf("qBar:%2X, pBar:%2X \n", *qBytePtr, *pBytePtr);
            if(*pBytePtr!=0 && *qBytePtr==0){
                /* p Drive Corruption */
                msg[1][i] ^= *pBytePtr;
            }else if(*pBytePtr==0 && *qBytePtr!=0){
                /* q Drive Corruption */
                msg[0][i] ^= *qBytePtr;
            }else { // Data Corruption
                pp = _exp[*pBytePtr];
                qq = _exp[*qBytePtr];
                qq = (!qq)?0xff:qq;/* FE 0 -> FE FF*/
                loc = 2+abs(pp-qq);
    //            printf("%d, correct=%X\n", loc, *pBytePtr);
                msg[loc][i] ^= *pBytePtr;
            }
            i++;
            pBytePtr++;
            qBytePtr++;
        }
    }
    free(tmp_msg[0]);
    free(tmp_msg[1]);
    free(tmp_msg);
}

void _RAID6DecodeTwoData(uchar n, uchar **msg, size_t bytes, int faila, int failb){
    printf("=====_RAID6DecodeTwoData=====\n");
    uchar **tmp_msg = safe_malloc(uchar*, n);
    tmp_msg[0] = safe_malloc(uchar, bytes);
    tmp_msg[1] = safe_malloc(uchar, bytes);
    for(int i=2;i<n;i++)
        tmp_msg[i] = msg[i];

    uchar *_mul_failaBase = _mul[_log[faila-2]];
    uchar *_mul_failbBase = _mul[_log[failb-2]];
    uchar *_mul_failBase  = _mul[_inv[(_log[faila-2])^(_log[failb-2])]];
    /* 錯誤位置 */
    uchar *failMsg0 = msg[faila];
    uchar *failMsg1 = msg[failb];
    /* P P', Q Q' */
    uint64_t *mPp = (uint64_t*)msg[1];
    uint64_t *mQp = (uint64_t*)msg[0];
    uint64_t *tPx64p = (uint64_t*)tmp_msg[1];
    uint64_t *tQx64p = (uint64_t*)tmp_msg[0];
    uchar *tPp = tmp_msg[1];
    uchar *tQp = tmp_msg[0];
    /*計算 PQ'*/
    PQ_x64_LookupTable(n, tmp_msg, bytes);

    /* PQ Bar */
    for(register int i=0;i<bytes;i+=8)
    {
        *tPx64p++ ^= *mPp++;
        *tQx64p++ ^= *mQp++;
    }

    for(register int i=0;i<bytes;i++, tPp++, tQp++)
    {
        *failMsg0++ = _mul_failBase[*tQp^_mul_failbBase[*tPp]];
        *failMsg1++ = _mul_failBase[*tQp^_mul_failaBase[*tPp]];
    }
    free(tmp_msg[0]);
    free(tmp_msg[1]);
    free(tmp_msg);
}

void _RAID6DecodePData(uchar n, uchar **msg, size_t bytes, int fail){
    printf("=====_RAID6DecodePData=====\n");
    uchar **tmp_msg = safe_malloc(uchar*, n);
    tmp_msg[0] = safe_malloc(uchar, bytes);
    tmp_msg[1] = safe_malloc(uchar, bytes);
    for(int i=2;i<n;i++)
        tmp_msg[i] = msg[i];

    /* 錯誤位置 */
    uchar *failMsg = msg[fail];
    uchar *_mul_failBase = _mul[_inv[_log[fail-2]]];

    /* Q Q' */
    uint64_t *mQx64p = (uint64_t*)msg[0];
    uint64_t *tQx64p = (uint64_t*)tmp_msg[0];
    uchar *tQp = tmp_msg[0];

    /*計算 PQ'*/
    PQ_x64(n, tmp_msg, bytes);
    for(register int i=0;i<bytes;i+=8){
        *tQx64p++ ^= *mQx64p++;
    }
    for(register int i=0;i<bytes;i++){
        *failMsg++ = _mul_failBase[*tQp++];
    }

    free(tmp_msg[0]);
    free(tmp_msg[1]);
    free(tmp_msg);
}

void _RAID6DecodeQData(uchar n, uchar **msg, size_t bytes, int fail){
    printf("=====_RAID6DecodeQData=====\n");
    uchar **tmp_msg = safe_malloc(uchar*, n);
    tmp_msg[0] = safe_malloc(uchar, bytes);
    tmp_msg[1] = safe_malloc(uchar, bytes);
    for(int i=2;i<n;i++)
        tmp_msg[i] = msg[i];

    uchar PBar;
    /* 錯誤位置 */
    uint64_t *failMsg = (uint64_t*)msg[fail];
    /* P P', Q Q' */
    uint64_t *mPx64p = (uint64_t*)msg[1];
    uint64_t *tPx64p = (uint64_t*)tmp_msg[1];

    /*計算 PQ'*/
    PQ_x64(n, tmp_msg, bytes);
    for(register int i=0;i<bytes;i+=8){
        *failMsg++ = *tPx64p++^*mPx64p++;
    }

    free(tmp_msg[0]);
    free(tmp_msg[1]);
    free(tmp_msg);
}

void RAID6_Decode(uchar n, uchar **msg, size_t bytes, int faila, int failb){
    if(faila>failb){
        int tmp = failb;
        failb = faila;
        faila = tmp;
    }
/*
    PQ 0 1
    QD 0 >1

    PD 1 >1
    DD >1 >1
*/
    if(faila>=1){
        if(faila==1)
        {/* PD */
            _RAID6DecodePData(n, msg, bytes, failb);
        }else
        {/* DD */
            _RAID6DecodeTwoData(n,msg,bytes,faila, failb);
        }
    }else{
        if(failb>1)
        {/* QD */
            _RAID6DecodeQData(n, msg, bytes, failb);
        }else
        {/* PQ */
        }
    }
}

/*
	msg lens = n*(f+1)
	SRC (n, n-2, f=2)
	---------- I/O -----------
	msg[0 ]=> x0
	msg[1 ]=> x1
	msg[2 ]=> x2 d0*
	msg[3 ]=> x3 d1*

	msg[4 ]=> y0
	msg[5 ]=> y1
	msg[6 ]=> y2 d2*
	msg[7 ]=> y3 d3*

	msg[8 ]=> s0(x0^y0)
	msg[9 ]=> s1(x1^y1)
	msg[10]=> s2(x2^y2)
	msg[11]=> s3(x3^y3)
*/
void SimpleRegeneratingCode(uchar n, uchar **msg, size_t bytes)
{
    int f=2;
    uint64_t **s = safe_malloc(uint64_t*, n);
    for(int i=0;i<n;i++)
        s[i] = (uint64_t*)msg[n*2+i];

    /* MDS Code(4, 2) */
    PQ_x64(n, msg  ,bytes);
    PQ_x64(n, msg+n,bytes);

    /* XOR */
	for(int i=0;i<n;i++)
		_XOR(msg[i], msg[n+i],msg[n*2+i], bytes);
    free(s);
}

void _XOR(const uchar *x, const uchar *y, uchar *s, size_t bytes){
    uint64_t *sx64 = (uint64_t*)s;
    uint64_t *xx64 = (uint64_t*)x;
    uint64_t *yx64 = (uint64_t*)y;
    for(register int i=0;i<bytes;i+=8){
        *sx64++ = *((uint64_t*)xx64++)^*((uint64_t*)yx64++);
    }
}

#define CYCLE(val, n) (val<0)?(n+val)%n:val
void SRC_Repair(uchar n, uchar* node[20][5], size_t bytes, int repair_node){
	/* AssessRepair { i-2, i-1, i+1, i+2 }*/
	int AR[4] =
	{
		CYCLE(repair_node-2, n),
		CYCLE(repair_node-1, n),
		CYCLE(repair_node+1, n),
		CYCLE(repair_node+2, n)
	};
//	for(int i=0;i<4;i++){
//        printf("%d\n", AR[i]);
//	}
	/* STEP 1 Repair X */
    _XOR(node[AR[1]][1], node[AR[0]][2], node[repair_node][0], bytes);

	/* STEP 2 Repair Y */
	_XOR(node[AR[2]][0], node[AR[1]][2], node[repair_node][1], bytes);

	/* STEP 3 Repair S */
	_XOR(node[AR[3]][0], node[AR[2]][1], node[repair_node][2], bytes);
//	printf("%X %X %X\n", node[AR[1]][1][0], node[AR[0]][2][0], node[repair_node][0][0]);
}

void SRC_Decode(uchar n, uchar* node[20][5], size_t bytes, uchar* data,int faila, int failb){
    if(faila>failb){
        int tmp = failb;
        failb = faila;
        faila = tmp;
    }

    uchar **X = safe_malloc(uchar*, n);
    uchar **Y = safe_malloc(uchar*, n);
//    for(int i=0;i<(n-2);i++){
//        X[i] = safe_malloc(uchar, bytes);
//        if(X[i]==NULL) { for(int j=i;j>=0;j--) free(X[i]); }
//        Y[i] = safe_malloc(uchar, bytes);
//		if(Y[i]==NULL) { for(int j=i;j>=0;j--) free(Y[i]); }
//    }

	for(int i=0;i<n;i++){
		if(i==faila || i==failb){
			X[i] = safe_malloc(uchar, bytes); if(X[i]==NULL) { for(int j=i-1;j>=0;j--) free(X[i]); }
			Y[i] = safe_malloc(uchar, bytes); if(Y[i]==NULL) { for(int j=i-1;j>=0;j--) free(Y[i]); }
		}else{
			X[i] = node[i][0];
			Y[(i+1)%n] = node[i][1];
		}
	}

/*
    PQ 0 1
    QD 0 >1

    PD 1 >1
    DD >1 >1
*/
//    if(faila>=1){
//        if(faila==1)
//        {/* PD */
//            _RAID6DecodePData(n, msg, bytes, failb);
//        }else
//        {/* DD */
//            _RAID6DecodeTwoData(n,msg,bytes,faila, failb);
//        }
//    }else{
//        if(failb>1)
//        {/* QD */
//            _RAID6DecodeQData(n, msg, bytes, failb);
//        }else
//        {/* PQ */
//        }
//    }
	for(int i=0;i<n;i++){
        free(X[i]);
	}
    free(X);
	for(int i=0;i<n;i++){
        free(Y[i]);
	}
    free(Y);
}

void SRC_SimpleDecode(uchar n, uchar* node[20][5], size_t bytes, uchar* data){
	uchar *p = data;
    int k = n-2;

	/* 處理 X[2] X[3]*/
	for(int i=0;i<k;i++, p+=bytes){
		memcpy(p, node[(2+i)%n][0],bytes);
	}

	/* 處理 Y[2] Y[3]*/
	for(int i=0;i<k;i++, p+=bytes){
		memcpy(p, node[(1+i)%n][1],bytes);
	}
}
