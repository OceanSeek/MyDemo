#ifndef __ModbusRtuMaster
#define __ModbusRtuMaster

int Init_ModbusRtuMaster(int DevNo);

/*
 *  asdu
 * */
typedef   struct{

    uint8_t         _devaddr;	//设备地址
    uint8_t      	_fn;		//功能码线圈类型
    uint16_t   		_askaddr;	//请求地址
    uint16_t        _num;		//请求寄存器数量
    uint16_t        _crc;		//crc校验码
}MODBUSASK_T, *PMODBUSASK_T;




#endif

