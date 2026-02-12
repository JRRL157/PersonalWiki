/**
*
*HAL Driver (alt_avalon_spi_command): Ele é sequencial. Ele envia todos os bytes
de escrita e, só depois, começa a ler os bytes de recepção. No loopback, o que
você enviou "passou direto" pelo MISO enquanto o driver ainda estava apenas
escrevendo.

Seu Código (Acesso Direto): Você leu o registrador de RX imediatamente após a
transmissão. Como o SPI é um protocolo Full-Duplex, o dado entra no MISO no
exato momento em que sai pelo MOSI. Ao ler o registrador logo após o TMT
(Transmitter Empty), você capturou o dado que "voltou" pelo fio.
*
*/

#include "alt_types.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_spi.h"
#include "altera_avalon_spi_regs.h"
#include "io.h"
#include "system.h"
#include <stdio.h>
#include <unistd.h>

void eth_init_test() {
  printf("[DEBUG] SPI Base Address: 0x%08X\n", SPI_BASE);
  printf("[DEBUG] Reset PIO Base Address: 0x%08X\n", PIO_ETH_RST_BASE);
}
int main() {
  eth_init_test();
  printf("--- Teste de Loopback em Tempo Real ---\n");

  // Limpa qualquer lixo no RX
  IORD_ALTERA_AVALON_SPI_RXDATA(SPI_BASE);

  int i = 0;
  while (1) {
    // 1. Envia 0xA5
    IOWR_ALTERA_AVALON_SPI_TXDATA(SPI_BASE, 0xA5 + i);

    // 2. Aguarda a transmissão terminar (bit TRDY e TMT no Status)
    // O valor 0x60 que voce leu antes indica que esta pronto!
    while (!(IORD_ALTERA_AVALON_SPI_STATUS(SPI_BASE) &
             ALTERA_AVALON_SPI_STATUS_TMT_MSK))
      ;

    // 3. Lê o que voltou no exato momento da transmissão
    unsigned char recebido = IORD_ALTERA_AVALON_SPI_RXDATA(SPI_BASE);

    if (recebido == (0xA5 + i)) {
      printf("SUCESSO: Loopback Físico OK (0x%02X)\n", recebido);
    } else {
      printf("FALHA: Lido 0x%02X (Verifique o jumper)\n", recebido);
    }
    usleep(1000000);
    i++;
  }
  return 0;
}
