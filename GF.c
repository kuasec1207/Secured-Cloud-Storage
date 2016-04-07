#include "GF.h"

int gf_init(){
	if(isIniGF == 1)
		return;

    isIniGF = 0;
    uchar p=29;
    int i,j;
    uchar val = 1;
    _log[0] = val;
    _exp[val] = 0;
    for(i=1;i<=255;i++){
        //val = (val<<1)^((val>>7&0x01)*p);
        val = (val<<1)^((val&0x80)?p:0);
        //val = (val&0x80)?(val<<1)^p:(val<<1);
        //printf("%d,%d\n",i,val);
        if(i==255){
            printf("\n");
        }
        _log[i]=val;
        _exp[val]=i%255;
        //printf("%d, %d\n",i,val);
    }
    /* create multi table */
    for(i=0;i<=255;i++){
    	for(j=0;j<=255;j++){
    		_mul[i][j]=gf_multi(i, j);
    	}
        /* create inv table */
        _inv[i] = (i==0) ? 0 : ((i==1) ? 1 : (_log[255-_exp[i]]));
    }
    return isIniGF = 1;
}
/**
 * multi
 */
uchar gf_multi(uchar a, uchar b){
    uchar c=0;
    int i;
    uchar p = 29;
    for(i=7;i>=1;i--){
        c^=(((a>>i)&0x01)*b);
        //c=((c<<1)^(((c>>7)&0x01)*p));
        c = (c<<1)^((c&0x80)?p:0);
    }
    c=c^(a&0x01)*b;
    return c;
}
/**
 * div table
 */
uchar gf_div_table(uchar a, uchar b){
    /*
		it may a negative number, so using int type
	*/
	if(a==0 || b==0) return 0;
	int tmp = _exp[a]-_exp[b];
	tmp = (tmp<0) ? tmp+255 : tmp;
    return _log[tmp];
}
/**
 *	multi table
 */
uchar gf_multi_table(uchar a, uchar b){
//    if(a==0 || b==0){
//        return 0;
//    }

//    return _log[(_exp[a]+_exp[b])%FIELD];
    return _mul[a][b];
}

uchar gf_inverse_table(uchar a){
	return _inv[a];
}

uchar gf_mul1(uchar a, uchar b){
    return _mul[a][b];
}
uchar gf_mul2(uchar a, uchar b){
    if(a==0 || b==0){
        return 0;
    }
    return _log[(_exp[a]+_exp[b])%FIELD];
}
