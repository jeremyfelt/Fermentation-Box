#ifndef OneWire_h
#define OneWire_h
 
#include <inttypes.h>
// you can exclude onewire_search by defining that to 0
#ifndef ONEWIRE_SEARCH
#define ONEWIRE_SEARCH 1
#endif
 
// You can exclude CRC checks altogether by defining this to 0
#ifndef ONEWIRE_CRC
#define ONEWIRE_CRC 1
#endif
 
 
 
// You can allow 16-bit CRC checks by defining this to 1
// (Note that ONEWIRE_CRC must also be 1.)
#ifndef ONEWIRE_CRC16
#define ONEWIRE_CRC16 1
#endif
 
#define FALSE 0
#define TRUE  1
 
class OneWire {
    private:
        uint16_t _pin;
        void DIRECT_WRITE_LOW(void);
        void DIRECT_MODE_OUTPUT(void);
        void DIRECT_WRITE_HIGH(void);
        void DIRECT_MODE_INPUT(void);
        uint8_t DIRECT_READ(void);
#if ONEWIRE_SEARCH
        // global search state
        unsigned char ROM_NO[8];
        uint8_t LastDiscrepancy;
        uint8_t LastFamilyDiscrepancy;
        uint8_t LastDeviceFlag;
#endif
 
    public:
        OneWire( uint16_t pin);
        
        // Perform a 1-Wire reset cycle. Returns 1 if a device responds
        // with a presence pulse.  Returns 0 if there is no device or the
        // bus is shorted or otherwise held low for more than 250uS
        uint8_t reset(void);
        
        // Issue a 1-Wire rom select command, you do the reset first.
        void select(const uint8_t rom[8]);
        
        // Issue a 1-Wire rom skip command, to address all on bus.
        void skip(void);
        
        // Write a byte. If 'power' is one then the wire is held high at
        // the end for parasitically powered devices. You are responsible
        // for eventually depowering it by calling depower() or doing
        // another read or write.
        void write(uint8_t v, uint8_t power = 0);
        
        void write_bytes(const uint8_t *buf, uint16_t count, bool power = 0);
        
        // Read a byte.
        uint8_t read(void);
        
        void read_bytes(uint8_t *buf, uint16_t count);
        
        // Write a bit. The bus is always left powered at the end, see
        // note in write() about that.
        void write_bit(uint8_t v);
        
        // Read a bit.
        uint8_t read_bit(void);
        
        // Stop forcing power onto the bus. You only need to do this if
        // you used the 'power' flag to write() or used a write_bit() call
        // and aren't about to do another read or write. You would rather
        // not leave this powered if you don't have to, just in case
        // someone shorts your bus.
        void depower(void);
 
#if ONEWIRE_SEARCH
        // Clear the search state so that if will start from the beginning again.
        void reset_search();
        
        // Setup the search to find the device type 'family_code' on the next call
        // to search(*newAddr) if it is present.
        void target_search(uint8_t family_code);
        
        // Look for the next device. Returns 1 if a new address has been
        // returned. A zero might mean that the bus is shorted, there are
        // no devices, or you have already retrieved all of them.  It
        // might be a good idea to check the CRC to make sure you didn't
        // get garbage.  The order is deterministic. You will always get
        // the same devices in the same order.
        uint8_t search(uint8_t *newAddr);
#endif
 
#if ONEWIRE_CRC
        // Compute a Dallas Semiconductor 8 bit CRC, these are used in the
        // ROM and scratchpad registers.
        static uint8_t crc8(uint8_t *addr, uint8_t len);

#if ONEWIRE_CRC16
        // Compute the 1-Wire CRC16 and compare it against the received CRC.
        // Example usage (reading a DS2408):
        //    // Put everything in a buffer so we can compute the CRC easily.
        //    uint8_t buf[13];
        //    buf[0] = 0xF0;    // Read PIO Registers
        //    buf[1] = 0x88;    // LSB address
        //    buf[2] = 0x00;    // MSB address
        //    WriteBytes(net, buf, 3);    // Write 3 cmd bytes
        //    ReadBytes(net, buf+3, 10);  // Read 6 data bytes, 2 0xFF, 2 CRC16
        //    if (!CheckCRC16(buf, 11, &buf[11])) {
        //        // Handle error.
        //    }    
        //          
        // @param input - Array of bytes to checksum.
        // @param len - How many bytes to use.
        // @param inverted_crc - The two CRC16 bytes in the received data.
        //                       This should just point into the received data,
        //                       *not* at a 16-bit integer.
        // @param crc - The crc starting value (optional)
        // @return True, iff the CRC matches.
        static bool check_crc16(const uint8_t* input, uint16_t len, const uint8_t* inverted_crc, uint16_t crc = 0);
        
        // Compute a Dallas Semiconductor 16 bit CRC.  This is required to check
        // the integrity of data received from many 1-Wire devices.  Note that the
        // CRC computed here is *not* what you'll get from the 1-Wire network,
        // for two reasons:
        //   1) The CRC is transmitted bitwise inverted.
        //   2) Depending on the endian-ness of your processor, the binary
        //      representation of the two-byte return value may have a different
        //      byte order than the two bytes you get from 1-Wire.
        // @param input - Array of bytes to checksum.
        // @param len - How many bytes to use.
        // @param crc - The crc starting value (optional)
        // @return The CRC16, as defined by Dallas Semiconductor.
        static uint16_t crc16(const uint8_t* input, uint16_t len, uint16_t crc = 0);
#endif
#endif
};
 
#endif
OneWire::OneWire(uint16_t pin) {
    pinMode(pin, INPUT);
     _pin = pin;
}
 
void OneWire::DIRECT_WRITE_LOW(void) {
    PIN_MAP[_pin].gpio_peripheral->BRR = PIN_MAP[_pin].gpio_pin;
}

void OneWire::DIRECT_MODE_OUTPUT(void) {
    GPIO_TypeDef *gpio_port = PIN_MAP[_pin].gpio_peripheral;
    uint16_t gpio_pin = PIN_MAP[_pin].gpio_pin;
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    if (gpio_port == GPIOA )
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    else if (gpio_port == GPIOB )
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    PIN_MAP[_pin].pin_mode = OUTPUT;
    GPIO_Init(gpio_port, &GPIO_InitStructure);
}

void OneWire::DIRECT_WRITE_HIGH(void) {
    PIN_MAP[_pin].gpio_peripheral->BSRR = PIN_MAP[_pin].gpio_pin;
}

void OneWire::DIRECT_MODE_INPUT(void) {
    GPIO_TypeDef *gpio_port = PIN_MAP[_pin].gpio_peripheral;
    uint16_t gpio_pin = PIN_MAP[_pin].gpio_pin;
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    if (gpio_port == GPIOA )
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    else if (gpio_port == GPIOB )
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    PIN_MAP[_pin].pin_mode = INPUT;
    GPIO_Init(gpio_port, &GPIO_InitStructure);
}

uint8_t OneWire::DIRECT_READ(void) {
    return GPIO_ReadInputDataBit(PIN_MAP[_pin].gpio_peripheral, PIN_MAP[_pin].gpio_pin);
}
// Perform the onewire reset function.  We will wait up to 250uS for
// the bus to come high, if it doesn't then it is broken or shorted
// and we return a 0;
//
// Returns 1 if a device asserted a presence pulse, 0 otherwise.
//
 
uint8_t OneWire::reset(void) {
    uint8_t r;
    uint8_t retries = 125;
    
    noInterrupts();
    DIRECT_MODE_INPUT();
    interrupts();
    // wait until the wire is high... just in case
    do {
        if (--retries == 0)
            return 0;
            
        delayMicroseconds(2);
    } while ( !DIRECT_READ());
    
    noInterrupts();
    DIRECT_WRITE_LOW();
    DIRECT_MODE_OUTPUT();   // drive output low
    interrupts();
    delayMicroseconds(480);
    noInterrupts();
    DIRECT_MODE_INPUT();    // allow it to float
    delayMicroseconds(70);
    r = !DIRECT_READ();
    interrupts();
    delayMicroseconds(410);
    return r;
}

void OneWire::write_bit(uint8_t v) {
    if (v & 1) {
        noInterrupts();
        DIRECT_WRITE_LOW();
        DIRECT_MODE_OUTPUT();   // drive output low
        delayMicroseconds(10);
        DIRECT_WRITE_HIGH();    // drive output high
        interrupts();
        delayMicroseconds(55);
    } else {
        noInterrupts();
        DIRECT_WRITE_LOW();
        DIRECT_MODE_OUTPUT();   // drive output low
        delayMicroseconds(65);
        DIRECT_WRITE_HIGH();    // drive output high
        interrupts();
        delayMicroseconds(5);
    }
}
 
//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
uint8_t OneWire::read_bit(void) {
    uint8_t r;
    noInterrupts();
    DIRECT_MODE_OUTPUT();
    DIRECT_WRITE_LOW();
    delayMicroseconds(3);
    DIRECT_MODE_INPUT();    // let pin float, pull up will raise
    delayMicroseconds(10);
    r = DIRECT_READ();
    interrupts();
    delayMicroseconds(53);
    return r;
}
 
//
// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set 'power' to 1, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//
void OneWire::write(uint8_t v, uint8_t power /* = 0 */) {
    uint8_t bitMask;
    
    for (bitMask = 0x01; bitMask; bitMask <<= 1)
        OneWire::write_bit( (bitMask & v)?1:0);
    
    if ( !power) {
        noInterrupts();
        DIRECT_MODE_INPUT();
        DIRECT_WRITE_LOW();
        interrupts();
    }
}
 
void OneWire::write_bytes(const uint8_t *buf, uint16_t count, bool power /* = 0 */) {
    for (uint16_t i = 0 ; i < count ; i++)
        write(buf[i]);
        
    if (!power) {
        noInterrupts();
        DIRECT_MODE_INPUT();
        DIRECT_WRITE_LOW();
        interrupts();
    }
}
 
//
// Read a byte
//
uint8_t OneWire::read() {
    uint8_t bitMask;
    uint8_t r = 0;
    
    for (bitMask = 0x01; bitMask; bitMask <<= 1)
        if ( OneWire::read_bit())
            r |= bitMask;

    return r;
}
 
void OneWire::read_bytes(uint8_t *buf, uint16_t count) {
    for (uint16_t i = 0 ; i < count ; i++)
        buf[i] = read();
}
 
//
// Do a ROM select
//
void OneWire::select(const uint8_t rom[8]) {
    uint8_t i;
    
    write(0x55);           // Choose ROM
    
    for (i = 0; i < 8; i++)
        write(rom[i]);
}
 
//
// Do a ROM skip
//
void OneWire::skip() {
    write(0xCC);          // Skip ROM
}
 
void OneWire::depower() {
    noInterrupts();
    DIRECT_MODE_INPUT();
    interrupts();
}
 
#if ONEWIRE_SEARCH
 
//
// You need to use this function to start a search again from the beginning.
// You do not need to do it for the first search, though you could.
//
void OneWire::reset_search() {
    // reset the search state
    LastDiscrepancy = 0;
    LastDeviceFlag = FALSE;
    LastFamilyDiscrepancy = 0;
    
    for(int i = 7; ; i--) {
        ROM_NO[i] = 0;
        
        if ( i == 0)
            break;
    }
}
 
// Setup the search to find the device type 'family_code' on the next call
// to search(*newAddr) if it is present.
//
void OneWire::target_search(uint8_t family_code) {
    // set the search state to find SearchFamily type devices
    ROM_NO[0] = family_code;
    
    for (uint8_t i = 1; i < 8; i++)
        ROM_NO[i] = 0;
    
    LastDiscrepancy = 64;
    LastFamilyDiscrepancy = 0;
    LastDeviceFlag = FALSE;
}
 
//
// Perform a search. If this function returns a '1' then it has
// enumerated the next device and you may retrieve the ROM from the
// OneWire::address variable. If there are no devices, no further
// devices, or something horrible happens in the middle of the
// enumeration then a 0 is returned.  If a new device is found then
// its address is copied to newAddr.  Use OneWire::reset_search() to
// start over.
//
// --- Replaced by the one from the Dallas Semiconductor web site ---
//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
uint8_t OneWire::search(uint8_t *newAddr) {
   uint8_t id_bit_number;
   uint8_t last_zero, rom_byte_number, search_result;
   uint8_t id_bit, cmp_id_bit;
 
   unsigned char rom_byte_mask, search_direction;
 
   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = 0;
 
   // if the last call was not the last one
   if (!LastDeviceFlag)
   {
      // 1-Wire reset
      if (!reset())
      {
         // reset the search
         LastDiscrepancy = 0;
         LastDeviceFlag = FALSE;
         LastFamilyDiscrepancy = 0;
         return FALSE;
      }
 
      // issue the search command
      write(0xF0);
 
      // loop to do the search
      do
      {
         // read a bit and its complement
         id_bit = read_bit();
         cmp_id_bit = read_bit();
 
         // check for no devices on 1-wire
         if ((id_bit == 1) && (cmp_id_bit == 1))
            break;
         else
         {
            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit)
               search_direction = id_bit;  // bit write value for search
            else
            {
               // if this discrepancy if before the Last Discrepancy
               // on a previous next then pick the same as last time
               if (id_bit_number < LastDiscrepancy)
                  search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
               else
                  // if equal to last pick 1, if not then pick 0
                  search_direction = (id_bit_number == LastDiscrepancy);
 
               // if 0 was picked then record its position in LastZero
               if (search_direction == 0)
               {
                  last_zero = id_bit_number;
 
                  // check for Last discrepancy in family
                  if (last_zero < 9)
                     LastFamilyDiscrepancy = last_zero;
               }
            }
 
            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
              ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
              ROM_NO[rom_byte_number] &= ~rom_byte_mask;
 
            // serial number search direction write bit
            write_bit(search_direction);
 
            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;
 
            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            if (rom_byte_mask == 0)
            {
                rom_byte_number++;
                rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7
 
      // if the search was successful then
      if (!(id_bit_number < 65))
      {
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
         LastDiscrepancy = last_zero;
 
         // check for last device
         if (LastDiscrepancy == 0)
            LastDeviceFlag = TRUE;
 
         search_result = TRUE;
      }
   }
 
   // if no device found then reset counters so next 'search' will be like a first
   if (!search_result || !ROM_NO[0])
   {
      LastDiscrepancy = 0;
      LastDeviceFlag = FALSE;
      LastFamilyDiscrepancy = 0;
      search_result = FALSE;
   }
   for (int i = 0; i < 8; i++) newAddr[i] = ROM_NO[i];
   return search_result;
  }
 
#endif
 
#if ONEWIRE_CRC
// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
//
 
 
//
// Compute a Dallas Semiconductor 8 bit CRC directly.
// this is much slower, but much smaller, than the lookup table.
//
uint8_t OneWire::crc8( uint8_t *addr, uint8_t len) {
        uint8_t crc = 0;
       
    while (len--) {
        uint8_t inbyte = *addr++;
        
        for (uint8_t i = 8; i; i--) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            
            if (mix)
                crc ^= 0x8C;
                
            inbyte >>= 1;
        }
    }
    
    return crc;
}
#endif
 
#if ONEWIRE_CRC16
bool OneWire::check_crc16(const uint8_t* input, uint16_t len, const uint8_t* inverted_crc, uint16_t crc) {
    crc = ~crc16(input, len, crc);
    return (crc & 0xFF) == inverted_crc[0] && (crc >> 8) == inverted_crc[1];
}
 
uint16_t OneWire::crc16(const uint8_t* input, uint16_t len, uint16_t crc) {
    static const uint8_t oddparity[16] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };
    
    for (uint16_t i = 0 ; i < len ; i++) {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = input[i];
        cdata = (cdata ^ crc) & 0xff;
        crc >>= 8;
    
        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
            crc ^= 0xC001;
    
        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }
    
    return crc;
}
#endif