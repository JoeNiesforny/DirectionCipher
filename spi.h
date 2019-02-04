
/*
CS  : SPI_CE0_N
SDO : SPI_MOSI
SDA : SPI_MISO
SCL : SPI_CLK
*/

void spi_master_init(void);
void spi_master_write(char data);
char spi_master_read();
void spi_start_transmission();
void spi_end_transmission();
