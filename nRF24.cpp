/** NRF24 control library.
 * This library implements the functionalities of the NRF24L01+ chip without
 * (almost) any further abstraction.
 *
 * Author: Dario Salvi (dariosalvi78 at gmail dot com)
 *
 * Based on the work of Mike McCauley (mikem at airspayce dot com)
 * Original code Id: NRF24.h,v 1.1 2012/12/12 11:17:16 mikem Exp mikem
 * Available at http://www.airspayce.com/mikem/arduino/NRF24/
 *
 * Differences with the original library:
 * - uses java doc documentation style
 * - adds reliability: almost every command is checked after being executed
 * - supports all pipes (not only 0 and 1)
 * - adds several functionalities that were missing in the original version
 * Still unsupported:
 * - acks with payload
 *
 * Licensed under the GPL license http://www.gnu.org/copyleft/gpl.html
 */
#include <SPI.h>
#include <nRF24.h>

//Static init of variables
uint8_t NRF24::_chipEnablePin = 9;
uint8_t NRF24::_chipSelectPin = 10;
uint8_t NRF24::_powerPin = -1;
uint8_t * NRF24::pipe0Address = NULL;
NRF24::NRF24PowerStatus NRF24::powerstatus = NRF24PowerDown;


void NRF24::configure(uint8_t chipEnablePin, uint8_t chipSelectPin, uint8_t powerPin)
{
    _chipEnablePin = chipEnablePin;
    _chipSelectPin = chipSelectPin;
    _powerPin = powerPin;

	pinMode(_chipEnablePin, OUTPUT);
	pinMode(_chipSelectPin, OUTPUT);

	pinMode(SCK, OUTPUT);
	pinMode(MOSI, OUTPUT);

	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);

	//SPI.setClockDivider(SPI_2XCLOCK_MASK); // 1 MHz SPI clock
	SPI.setClockDivider(SPI_CLOCK_DIV2); // 8MHz SPI clock
}


boolean NRF24::powerUpIdle(){
	if(powerstatus == NRF24PowerUpIdle) return true;

	if(powerstatus == NRF24PowerDown){

		digitalWrite(_chipEnablePin, LOW);
		digitalWrite(_chipSelectPin, HIGH);

    if(_powerPin >=0){
      digitalWrite(_powerPin, HIGH);
      delay(300);//additional delay for bringing up the chip
    }
		delay(100);

		// start the SPI library:
		SPI.begin();

		//Enables dynamic payloads and dynamic acks always
		spiWriteRegister(NRF24_REG_1D_FEATURE, NRF24_EN_DPL | NRF24_EN_DYN_ACK);
	}
	//Clear interrupts here
    spiWriteRegister(NRF24_REG_07_STATUS, NRF24_RX_DR | NRF24_TX_DS | NRF24_MAX_RT);

	uint8_t reg = spiReadRegister(NRF24_REG_00_CONFIG);
    reg = reg &  ~NRF24_PWR_UP; //set the power up bit to 0
    spiWriteRegister(NRF24_REG_00_CONFIG, reg);

	if((spiReadRegister(NRF24_REG_00_CONFIG) & NRF24_PWR_UP) != 0)
		return false;

	powerstatus = NRF24PowerUpIdle;
	return true;
}

boolean NRF24::powerDown()
{
  if (powerstatus == NRF24PowerDown)
	  return true;
  uint8_t reg = spiReadRegister(NRF24_REG_00_CONFIG);
  reg = reg &  ~NRF24_PWR_UP; //set the power up bit to 0
  spiWriteRegister(NRF24_REG_00_CONFIG, reg);

  digitalWrite(_chipEnablePin, LOW);
  digitalWrite(_chipSelectPin, LOW);

  SPI.end();

  if(_powerPin >=0)
    digitalWrite(_powerPin, LOW);

  digitalWrite(SCK, LOW);
  digitalWrite(MOSI, LOW);

  powerstatus = NRF24PowerDown;
  return true;
}

boolean NRF24::powerUpRx()
{
	if(powerstatus == NRF24PowerUpRX) return true;
	if(powerstatus == NRF24PowerDown) powerUpIdle();

	uint8_t reg = spiReadRegister(NRF24_REG_00_CONFIG);
    reg = (reg | NRF24_PWR_UP) | NRF24_PRIM_RX;
    spiWriteRegister(NRF24_REG_00_CONFIG, reg);

    //restore pipe 0 if any
    if(pipe0Address != NULL)
        if(!setPipeAddress(0, pipe0Address))
            return false;
    //If coming from Tx or power down clean queues
    flushTx();
    flushRx();

    digitalWrite(_chipEnablePin, HIGH);

    //wait the radio to come up
    delayMicroseconds(130);

	reg = spiReadRegister(NRF24_REG_00_CONFIG);
    if(((reg & NRF24_PWR_UP) == 0) || ((reg & NRF24_PRIM_RX) == 0))
        return false;

	powerstatus = NRF24PowerUpRX;
    return true;
}

boolean NRF24::powerUpTx()
{
	if(powerstatus == NRF24PowerUpTX) return true;
	if(powerstatus == NRF24PowerDown) powerUpIdle();

	uint8_t reg = spiReadRegister(NRF24_REG_00_CONFIG);
    reg = (reg | NRF24_PWR_UP) & ~NRF24_PRIM_RX;
    spiWriteRegister(NRF24_REG_00_CONFIG, reg);

    //If coming from Rx or power down clean queues
    flushTx();
    flushRx();

    digitalWrite(_chipEnablePin, HIGH);
    //wait the radio to come up
    delayMicroseconds(130);

	reg = spiReadRegister(NRF24_REG_00_CONFIG);
    if(((reg & NRF24_PWR_UP) == 0) || ((reg & NRF24_PRIM_RX) != 0))
        return true;//already in TX

	powerstatus = NRF24PowerUpTX;
    return true;
}

NRF24::NRF24PowerStatus NRF24::getPowerStatus() {
    return powerstatus;
}

// Low level commands for interfacing with the device
uint8_t NRF24::spiCommand(uint8_t command)
{
    digitalWrite(_chipSelectPin, LOW);
    uint8_t status = SPI.transfer(command);
    digitalWrite(_chipSelectPin, HIGH);
    return status;
}

// Read and write commands
uint8_t NRF24::spiRead(uint8_t command)
{
    digitalWrite(_chipSelectPin, LOW);
    SPI.transfer(command); // Send the address, discard status
    uint8_t val = SPI.transfer(0); // The MOSI value is ignored, value is read
    digitalWrite(_chipSelectPin, HIGH);
    return val;
}

uint8_t NRF24::spiWrite(uint8_t command, uint8_t val)
{
    digitalWrite(_chipSelectPin, LOW);
    uint8_t status = SPI.transfer(command);
    SPI.transfer(val); // New register value follows
    digitalWrite(_chipSelectPin, HIGH);
    return status;
}

void NRF24::spiBurstRead(uint8_t command, uint8_t* dest, uint8_t len)
{
    digitalWrite(_chipSelectPin, LOW);
    SPI.transfer(command); // Send the start address, discard status
    while (len--)
    {
        *dest++ = SPI.transfer(0); // The MOSI value is ignored, value is read
    }
    digitalWrite(_chipSelectPin, HIGH);
    // 300 microsecs for 32 octet payload
}

uint8_t NRF24::spiBurstWrite(uint8_t command, uint8_t* src, uint8_t len)
{
    digitalWrite(_chipSelectPin, LOW);
    uint8_t status = SPI.transfer(command);
    while (len--)
        SPI.transfer(*src++);
    digitalWrite(_chipSelectPin, HIGH);
    return status;
}

// Use the register commands to read and write the registers
uint8_t NRF24::spiReadRegister(uint8_t reg)
{
    return spiRead((reg & NRF24_REGISTER_MASK) | NRF24_COMMAND_R_REGISTER);
}

uint8_t NRF24::spiWriteRegister(uint8_t reg, uint8_t val)
{
    return spiWrite((reg & NRF24_REGISTER_MASK) | NRF24_COMMAND_W_REGISTER, val);
}

void NRF24::spiBurstReadRegister(uint8_t reg, uint8_t* dest, uint8_t len)
{
    return spiBurstRead((reg & NRF24_REGISTER_MASK) | NRF24_COMMAND_R_REGISTER, dest, len);
}

uint8_t NRF24::spiBurstWriteRegister(uint8_t reg, uint8_t* src, uint8_t len)
{
    return spiBurstWrite((reg & NRF24_REGISTER_MASK) | NRF24_COMMAND_W_REGISTER, src, len);
}

uint8_t NRF24::statusRead()
{
    return spiReadRegister(NRF24_REG_07_STATUS);
//    return spiCommand(NRF24_COMMAND_NOP); // Side effect is to read status
}

uint8_t NRF24::flushTx()
{
    return spiCommand(NRF24_COMMAND_FLUSH_TX);
}

uint8_t NRF24::flushRx()
{
    return spiCommand(NRF24_COMMAND_FLUSH_RX);
}

boolean NRF24::setChannel(uint8_t channel)
{
    spiWriteRegister(NRF24_REG_05_RF_CH, channel & NRF24_RF_CH);
    uint8_t actch = getChannel();
    return (actch == channel);
}

uint8_t NRF24::getChannel()
{
    uint8_t reg = spiReadRegister(NRF24_REG_05_RF_CH);
    return reg;
}

boolean NRF24::getRPD()
{
    powerUpRx();
    delayMicroseconds(200);
    uint8_t rpd = spiReadRegister(NRF24_REG_09_RPD);
    return (rpd>0);
}

uint8_t NRF24::getRSSI()
{
    powerUpRx();
    delayMicroseconds(200);
    uint8_t rssi =0;
    for(int i=0; i<256; i++)
    {
        rssi += getRPD();
        delayMicroseconds(200);
        flushRx();
    }

    return rssi;
}

NRF24::NRF24CRC NRF24::getCRC()
{
    uint8_t reg = spiReadRegister(0);
    if((reg & NRF24_EN_CRC) == 0)
        return NRF24CRCNO;
    else
    {
        if((reg & NRF24_CRCO) == 0)
            return NRF24CRC1Byte;
        else return NRF24CRC2Bytes;
    }
}

boolean NRF24::setCRC(NRF24CRC crc)
{
    uint8_t reg = spiReadRegister(0);
    if(crc == NRF24CRCNO)
    {
        reg = reg & ~NRF24_EN_CRC;
    }
    else if(crc == NRF24CRC1Byte)
    {
        reg = reg | NRF24_EN_CRC & ~NRF24_CRCO;
    }
    else
    {
        reg = reg | NRF24_EN_CRC | NRF24_CRCO;
    }
    spiWriteRegister(NRF24_REG_00_CONFIG, reg);
    NRF24CRC actcrc = getCRC();
    if(actcrc == crc) return true;
    else return false;
}

boolean NRF24::setTXRetries(uint8_t delay, uint8_t count)
{
    spiWriteRegister(NRF24_REG_04_SETUP_RETR, ((delay << 4) & NRF24_ARD) | (count & NRF24_ARC));
    return true;
}

boolean NRF24::setAddressSize(NRF24AddressSize size)
{
    if((size != NRF24AddressSize3Bytes)
            && (size != NRF24AddressSize4Bytes)
            && (size != NRF24AddressSize5Bytes))
        return false;
    spiWriteRegister(NRF24_REG_03_SETUP_AW, size);
    delay(100); //just in case..
    NRF24AddressSize actsize = getAddressSize();
    if(size != actsize)
        return false;
    else return true;
}

NRF24::NRF24AddressSize NRF24::getAddressSize()
{
    uint8_t reg = spiReadRegister(3);
    reg = (reg & NRF24_AW);
    if(reg == 1)
        return NRF24AddressSize3Bytes;
    else if(reg == 2)
        return NRF24AddressSize4Bytes;
    else if(reg == 3)
        return NRF24AddressSize5Bytes;
}


boolean NRF24::areAddressesEquals(uint8_t *addr1, uint8_t* addr2, uint8_t len)
{
    for(int i=0; i<len; i++)
    {
        if(addr1[i] != addr2[i])
            return false;
    }
    return true;
}

boolean NRF24::setTransmitAddress(uint8_t* address)
{
    int len = getAddressSize()+2;
    spiBurstWriteRegister(NRF24_REG_10_TX_ADDR, address, len);
    uint8_t actadd[len];
    if(!getTransmitAddress(actadd))
        return false;
    return areAddressesEquals(address, actadd, len);
}

boolean NRF24::getTransmitAddress(uint8_t * address)
{
    int len = getAddressSize()+2;
    spiBurstReadRegister(NRF24_REG_10_TX_ADDR, address, len);
    return true;
}

boolean NRF24::setPipeAddress(uint8_t pipe, uint8_t* address)
{
    if(pipe == 0)
        pipe0Address = 0;
    int len = getAddressSize()+2;
    //TODO: only send first byte for byte 1,2,3,4,5, or maybe it works anyway?
    spiBurstWriteRegister(NRF24_REG_0A_RX_ADDR_P0 + pipe, address, len);
    uint8_t curraddr[len];
    if(!getPipeAddress(pipe, curraddr))
        return false;
    return areAddressesEquals(address, curraddr, len);
}

boolean NRF24::getPipeAddress(uint8_t pipe, uint8_t * address)
{
    uint8_t len = getAddressSize()+2;
    if((pipe == 0) || (pipe == 1))
    {
        spiBurstReadRegister(NRF24_REG_0A_RX_ADDR_P0 + pipe, address, len);
        return true;
    }
    else if((pipe ==2) || (pipe == 3) || (pipe == 4) || (pipe == 5))
    {
        if(!getPipeAddress(1, address)) //Get base address
            return false;
        uint8_t lastbyte[1];
        spiBurstReadRegister(NRF24_REG_0A_RX_ADDR_P0 + pipe, lastbyte, 1);
        address[len-1] = lastbyte[0];
        return true;
    }
    else return false;
}

boolean NRF24::enablePipe(uint8_t pipe)
{
    uint8_t reg = spiReadRegister(NRF24_REG_02_EN_RXADDR);
    reg = reg | (NRF24_ERX_P0 + pipe);
    spiWriteRegister(NRF24_REG_02_EN_RXADDR, reg);
    return isPipeEnabled(pipe);
}


boolean NRF24::isPipeEnabled(uint8_t pipe)
{
    uint8_t reg = spiReadRegister(NRF24_REG_02_EN_RXADDR);
    return !((reg & (NRF24_ERX_P0 + pipe)) ==0);
}


boolean NRF24::setAutoAck(uint8_t pipe, boolean autoack)
{
    uint8_t reg = spiReadRegister(NRF24_REG_01_EN_AA);
    if(autoack)
    {
        reg = reg | NRF24_ENAA_P0 + pipe;
        spiWriteRegister(NRF24_REG_01_EN_AA, reg);
        return isAutoAckEnabled(pipe);
    }
    else
    {
        reg = reg & ~(NRF24_ENAA_P0 + pipe);
        spiWriteRegister(NRF24_REG_01_EN_AA, reg);
        return !isAutoAckEnabled(pipe);
    }
}

boolean NRF24::isAutoAckEnabled(uint8_t pipe)
{
    uint8_t reg = spiReadRegister(NRF24_REG_01_EN_AA);
    return !((reg & (NRF24_ENAA_P0 + pipe)) ==0);
}

boolean NRF24::setPayloadSize(uint8_t pipe, uint8_t size)
{
    if(size >32)
        return false;
    if(size == 0)
    {
        uint8_t reg = spiReadRegister(NRF24_REG_1C_DYNPD);
        reg = reg | (NRF24_DPL_P0 << pipe);
        spiWriteRegister(NRF24_REG_1C_DYNPD, reg);
    }
    else
    {
        //Unset dynamic payload first
        if(getPayloadSize(pipe) == 0)
        {
            uint8_t reg = spiReadRegister(NRF24_REG_1C_DYNPD);
            reg = reg & ~(NRF24_DPL_P0 << pipe);
            spiWriteRegister(NRF24_REG_1C_DYNPD, reg);
        }
        spiWriteRegister(NRF24_REG_11_RX_PW_P0 + pipe, size);
    }
    return (getPayloadSize(pipe) == size);
}

uint8_t NRF24::getPayloadSize(uint8_t pipe)
{
    uint8_t reg = spiReadRegister(NRF24_REG_1C_DYNPD);
    if((reg & (NRF24_DPL_P0 << pipe)) == 0)
    {
        reg = spiReadRegister(NRF24_REG_11_RX_PW_P0 + pipe);
        return reg;
    }
    else return 0;
}


boolean NRF24::setRF(NRF24DataRate data_rate, NRF24TransmitPower power)
{
    uint8_t value = (power << 1) & NRF24_PWR;
    // Ugly mapping of data rates to noncontiguous 2 bits:
    if (data_rate == NRF24DataRate250kbps)
        value |= NRF24_RF_DR_LOW;
    else if (data_rate == NRF24DataRate2Mbps)
        value |= NRF24_RF_DR_HIGH;
    // else NRF24DataRate1Mbps, 00
    spiWriteRegister(NRF24_REG_06_RF_SETUP, value);
    NRF24DataRate actrate = getDatarate();
    NRF24TransmitPower actpow = getTransmitPower();
    if((data_rate == actrate) && (power == actpow))
        return true;
    else return false;
}

NRF24::NRF24DataRate NRF24::getDatarate()
{
    uint8_t reg = spiReadRegister(6);
    boolean drl = (reg & NRF24_RF_DR_LOW) != 0;
    boolean drh = (reg & NRF24_RF_DR_HIGH) != 0;
    if(drl && !drh)
        return NRF24DataRate250kbps;
    else if(!drl && !drh)
        return NRF24DataRate1Mbps;
    else
        return NRF24DataRate2Mbps;
}

NRF24::NRF24TransmitPower NRF24::getTransmitPower()
{
    uint8_t reg = spiReadRegister(6);
    reg = (reg & NRF24_PWR)>>1;
    if(reg == 0)
        return NRF24TransmitPowerm18dBm;
    if(reg == 1)
        return NRF24TransmitPowerm12dBm;
    if(reg == 2)
        return NRF24TransmitPowerm6dBm;
    if(reg == 3)
        return NRF24TransmitPower0dBm;
}

boolean NRF24::setIRQMask(boolean mask_RX, boolean mask_TX, boolean mask_MAX_RT){

	uint8_t reg = spiReadRegister(0);
    if(mask_RX) {
		reg = reg | NRF24_MASK_RX_DR;
    } else {
        reg = reg & ~NRF24_MASK_RX_DR;
    }

	if(mask_TX) {
		reg = reg | NRF24_MASK_TX_DS;
    } else {
        reg = reg & ~NRF24_MASK_TX_DS;
    }

	if(mask_MAX_RT) {
		reg = reg | NRF24_MASK_MAX_RT;
    } else {
        reg = reg & ~NRF24_MASK_MAX_RT;
    }

    spiWriteRegister(NRF24_REG_00_CONFIG, reg);

	return (getIRQMaskRX() == mask_RX) &&
		(getIRQMaskTX() == mask_TX) &&
		(getIRQMaskRT() == mask_MAX_RT);
}

boolean NRF24::getIRQMaskRX(){
	uint8_t reg = spiReadRegister(0);
	return ((reg & NRF24_MASK_RX_DR) != 0);
}

boolean NRF24::getIRQMaskTX(){
	uint8_t reg = spiReadRegister(0);
	return ((reg & NRF24_MASK_TX_DS) != 0);
}

boolean NRF24::getIRQMaskRT(){
	uint8_t reg = spiReadRegister(0);
	return ((reg & NRF24_MASK_MAX_RT) != 0);
}

boolean NRF24::send(uint8_t* data, uint8_t len, boolean noack)
{
    powerUpTx(); //set to transmit mode

	if(! noack)  //if ack is set
    {
        //Set both TX_ADDR and RX_ADDR_P0 for auto-ack with Enhanced ShockBurst:
        //From manual:
        //Set RX_ADDR_P0 equal to this address to handle
        //automatic acknowledge if this is a PTX device with
        //Enhanced ShockBurst enabled
        int len = getAddressSize();
        byte addr[len];
        if(getTransmitAddress(addr))
			setPipeAddress(0, addr);
    }

    spiBurstWrite(noack ? NRF24_COMMAND_W_TX_PAYLOAD_NOACK : NRF24_COMMAND_W_TX_PAYLOAD, data, len);//send data
    //signal send
    //digitalWrite(_chipEnablePin, LOW);
    //delayMicroseconds(10);
    //digitalWrite(_chipEnablePin, HIGH);

    //Radio will return to Standby II mode after transmission is complete
    //Wait for either the Data Sent or Max ReTries flag, signalling the
    //end of transmission
    uint8_t status;
    unsigned long starttime = millis();
    while (((millis() - starttime) < 2000) && //times out after 2 seconds
            !((status = statusRead()) & (NRF24_TX_DS | NRF24_MAX_RT)))
        ;

    // Must clear NRF24_MAX_RT if it is set, else no further comm
    spiWriteRegister(NRF24_REG_07_STATUS, status | ~NRF24_TX_DS | ~NRF24_MAX_RT);
    if (status & NRF24_MAX_RT)
    {
        flushTx();
        return false;
    }

    // Return true if data sent
    return (status & NRF24_TX_DS)!=0;
}

boolean NRF24::isSending()
{
    return !(spiReadRegister(NRF24_REG_00_CONFIG) & NRF24_PRIM_RX) && !(statusRead() & (NRF24_TX_DS | NRF24_MAX_RT));
}

boolean NRF24::available()
{
    if (spiReadRegister(NRF24_REG_17_FIFO_STATUS) & NRF24_RX_EMPTY)
        return false;
    // Manual says that messages > 32 octets should be discarded
    if (spiRead(NRF24_COMMAND_R_RX_PL_WID) > 32)
    {
        flushRx();
        return false;
    }
    return true;
}

void NRF24::waitAvailable()
{
    if(!powerUpRx())
        return;
    while (!available())
        ;
}

boolean NRF24::waitAvailableTimeout(uint16_t timeout)
{
    if(!powerUpRx())
        return false;

    unsigned long starttime = millis();
    while ((millis() - starttime) < timeout)
	{
        if (available())
            return true;
	}
    return false;
}

boolean NRF24::recv(uint8_t* pipe, uint8_t* buf, uint8_t* len)
{
    // 0 microsecs @ 8MHz SPI clock
    if (!available())
        return false;

    // Clear read interrupt
    spiWriteRegister(NRF24_REG_07_STATUS, NRF24_RX_DR);

    // 32 microsecs (if immediately available)
    *len = spiRead(NRF24_COMMAND_R_RX_PL_WID);
    uint8_t pipen = (spiReadRegister(NRF24_REG_07_STATUS) & NRF24_RX_P_NO) >> 1;

    if(pipen > 5) return false;
    else *pipe = pipen;
    // 44 microsecs
    spiBurstRead(NRF24_COMMAND_R_RX_PAYLOAD, buf, *len);
    // 140 microsecs (32 octet payload)

    return true;
}

void NRF24::printRegisters()
{
    uint8_t registers[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0d, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d};
    uint8_t i;
    for (i = 0; i < sizeof(registers); i++)
    {
        Serial.print(i, HEX);
        Serial.print(": ");
        Serial.println(spiReadRegister(i), HEX);
    }
}
