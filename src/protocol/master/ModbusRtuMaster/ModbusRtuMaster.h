#ifndef __ModbusRtuMaster
#define __ModbusRtuMaster

int Init_ModbusRtuMaster(int DevNo);

/*
 *  asdu
 * */
typedef   struct{

    uint8_t         _devaddr;	//�豸��ַ
    uint8_t      	_fn;		//��������Ȧ����
    uint16_t   		_askaddr;	//�����ַ
    uint16_t        _num;		//����Ĵ�������
    uint16_t        _crc;		//crcУ����
}MODBUSASK_T, *PMODBUSASK_T;




#endif

