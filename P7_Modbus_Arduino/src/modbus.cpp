#include "modbus.h"
#include "coils.h"

// uint8_t ByteArray[260]; // buffer de recepcion de los datos recibidos de los clientes
UINT16_VAL MBHoldingRegister[maxHoldingRegister];
UINT16_VAL MBInputRegister[maxInputRegister];
UINT16_VAL MBCoils;
UINT16_VAL MBDiscreteInputs;

//**************************************
//************* Funciones **************
//**************************************

void modbusSerial(uint8_t *ByteArray, uint16_t Length)
{
    uint8_t byteFN = ByteArray[MB_SER_FUNC]; //Guardo la funcion a realizar
    UINT16_VAL Start;
    UINT16_VAL WordDataLength;

    UINT16_VAL CRC;

    //Cheque si el ByteArray en la posicion 0 corresponde al Slave ID
    if ((CRC16(ByteArray, Length) == 0) && (ByteArray[MB_SER_UID] == ID_RTU_LOCAL))
    {
        
        //Switch de la maquina de estado para las distintas funciones disponibles en Modbus
        switch (byteFN)
        {
        case MB_FC_NONE:

            break;

        case MB_FC_READ_COILS: // 01 Read Coils

            break;

        case MB_FC_READ_DISCRETE_INPUTS: // funcion 02: Read Discrete Inputs

            break;

        case MB_FC_READ_REGISTERS: // Funcion 03: Read Holding Registers

            //Primero voy guardando en posiciones sucesivas de datos la info necesaria

            // direccion de comienzo Modbus
            Start.byte.HB = ByteArray[2];
            Start.byte.LB = ByteArray[3];

            // numero de datos
            WordDataLength.byte.HB = ByteArray[4];
            WordDataLength.byte.LB = ByteArray[5];

            // numero de bytes de datos de respuesta
            ByteArray[2] = WordDataLength.Val * 2;

            for (uint16_t i = 0; i < WordDataLength.Val; i++) // datos de respuesta
            {
                ByteArray[3 + i * 2] = MBHoldingRegister[i + Start.Val].byte.HB;
                ByteArray[4 + i * 2] = MBHoldingRegister[i + Start.Val].byte.LB;
            }

            // CRC
            CRC.Val = CRC16(ByteArray, ByteArray[2] + 3);
            ByteArray[ByteArray[2] + 3] = (CRC.byte.LB);
            ByteArray[ByteArray[2] + 4] = (CRC.byte.HB);

            // rs485(ByteArray, ByteArray[2] + 5);

            // uart_write_bytes(uart_port_t uart_num, const void* src, size_t size);

            uart_write_bytes(UART_NUM_0, (const char *)ByteArray, ByteArray[2] + 5);

            break;

        case MB_FC_READ_INPUT_REGISTERS: // 04 Read Input Registers Registers
                                         // direccion de cominzo Modbus
            Start.byte.HB = ByteArray[2];
            Start.byte.LB = ByteArray[3];

            // numero de datos
            WordDataLength.byte.HB = ByteArray[4];
            WordDataLength.byte.LB = ByteArray[5];

            // numero de bytes de datos de respuesta
            ByteArray[2] = WordDataLength.Val * 2;

            for (uint16_t i = 0; i < WordDataLength.Val; i++) // datos de respuesta
            {
                ByteArray[3 + i * 2] = MBInputRegister[i + Start.Val].byte.HB;
                ByteArray[4 + i * 2] = MBInputRegister[i + Start.Val].byte.LB;
            }

            // CRC
            CRC.Val = CRC16(ByteArray, ByteArray[2] + 3);
            ByteArray[ByteArray[2] + 3] = (CRC.byte.LB);
            ByteArray[ByteArray[2] + 4] = (CRC.byte.HB);

            // rs485(ByteArray, ByteArray[2] + 5);

            uart_write_bytes(UART_NUM_0, (const char *)ByteArray, ByteArray[2] + 5);

            break;

        case MB_FC_WRITE_COIL: // 05 Write COIL

            uart_write_bytes(UART_NUM_0, (const char *)ByteArray, 8);

            // Guardando Direccion del Coil a Modificar
            Start.byte.HB = ByteArray[2];
            Start.byte.LB = ByteArray[3];

            // Comando para encender o apagar Coil
            // 0xFF 00 -> Encender
            // 0x00 00 -> Apagar

            WordDataLength.byte.HB = ByteArray[4];
            WordDataLength.byte.LB = ByteArray[5];

            // comprueba el Address correspondiente a cada Coil con los dos bytes de Start
            switch (Start.Val)
            {
                // Dirección Coil 0
            case 0:
                // Comprueba si comando es ON
                if (WordDataLength.Val == 0xFF00)
                {
                    gpio_set_level(C0, 1); //Enciendo el coil
                    MBCoils.bits.b0 = 1;
                }
                // Si no comando OFF
                else
                {
                    gpio_set_level(C0, 0); //Enciendo el coil
                    MBCoils.bits.b0 = 0;
                }
                break;

                // Dirección Coil 1

            case 1:
                if (WordDataLength.Val == 0xFF00)
                {
                    gpio_set_level(C1, 1);
                    MBCoils.bits.b1 = 1;
                }
                else
                {
                    gpio_set_level(C1, 0);
                    MBCoils.bits.b1 = 0;
                }
                break;

                // Dirección coil 2

            case 2:
                if (WordDataLength.Val == 0xFF00)
                {
                    gpio_set_level(C2, 1);
                    MBCoils.bits.b2 = 1;
                }
                else
                {
                    gpio_set_level(C2, 0);
                    MBCoils.bits.b2 = 0;
                }
                break;

            default:
                break;
            }

            break;

        case MB_FC_WRITE_REGISTER: // 06 Write Holding Register

            // direccion de cominzo Modbus
            Start.byte.HB = ByteArray[2];
            Start.byte.LB = ByteArray[3];

            MBHoldingRegister[Start.Val].byte.HB = ByteArray[4];
            MBHoldingRegister[Start.Val].byte.LB = ByteArray[5];

            // rs485(ByteArray, 8);

            uart_write_bytes(UART_NUM_0, (const char *)ByteArray, 8);

            break;

        case MB_FC_WRITE_MULTIPLE_COILS: // 15 Write Coils

            break;

        case MB_FC_WRITE_MULTIPLE_REGISTERS: // 16 Write Holding Registers

            break;
        }
    }
}