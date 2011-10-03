#define false (0)
#define true  (~0)


/* Public methods */
void max3421e(void); //constructor
void max3421eRegWr(u8 reg, u8 val);
u8 max3421eRegRd(u8 reg);
char* max3421eBytesWr(u8 reg, u8 nbytes, char* data);  // TO BE TESTED!
char* max3421eBytesRd(u8 reg, u8 nbytes, char* data); // TO BE TESTED!
void max3421ePowerOn(void);
u8 max3421eReset(void);
u8 max3421eTask(void);

/* Prvivate methods (as from Max3421e.cpp Arduino code) */
void spi_init(void);
void pinInit(void);
void busprobe(void);
u8 getVbusState(void);
u8 IntHandler(void);
u8 GpxHandler(void);
u8 GpxHandler(void);
u8 readINT(void);
u8 readGPX(void);
