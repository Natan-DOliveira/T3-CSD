/*
 * Arquivo: keyboard.c
 * Descrição: Driver de teclado PS/2 via AXI bus.
 * Responsável por interpretar scancodes e gerenciar estados das teclas.
 */

#include <hf-risc.h>
#include "keyboard.h"

// Variáveis globais de estado das teclas (1 = Pressionado, 0 = Solto)
int key_left = 0;
int key_right = 0;
int key_fire = 0;
int key_break = 0;

void check_keyboard(void) {
    // 1. Verificação de Status: O bit 1 (STVALID) indica se há dado novo no buffer
    if (!(KEYBOARD_AXI_STATUS & KEYBOARD_AXI_STVALID)) return;

    // 2. Leitura do Scancode (SDATA)
    uint8_t sc = KEYBOARD_AXI_SDATA;

    // 3. Tratamento de Break Code (0xF0)
    // O protocolo PS/2 envia F0 antes de enviar a tecla que foi solta.
    if (sc == 0xF0) {
        // Precisamos esperar o próximo byte para saber QUAL tecla foi solta.
        // Implementamos um timeout para evitar que o processador trave se o hardware falhar.
        int timeout = 500000; 
        
        while (!(KEYBOARD_AXI_STATUS & KEYBOARD_AXI_STVALID) && timeout--) ;
        
        if (timeout > 0) {
            uint8_t next_sc = KEYBOARD_AXI_SDATA;
            
            // Zera a flag da tecla correspondente
            if (next_sc == SCAN_A || next_sc == SCAN_LEFT) key_left = 0;
            else if (next_sc == SCAN_D || next_sc == SCAN_RIGHT) key_right = 0;
            else if (next_sc == SCAN_SPACE) key_fire = 0;
        } else {
            // [ROBUSTEZ] Timeout ocorreu: Perdemos o código da tecla solta.
            // Por segurança, paramos todo movimento para o boneco não "enganchar".
            key_left = 0;
            key_right = 0;
            key_fire = 0;
        }
        key_break = 0; 
    }
    // 4. Tratamento de Make Code (Tecla Pressionada)
    // Ignora 0xE0 (prefixo de teclas estendidas)
    else if (sc != 0xE0) { 
        if (sc == SCAN_A || sc == SCAN_LEFT) key_left = 1;
        else if (sc == SCAN_D || sc == SCAN_RIGHT) key_right = 1;
        else if (sc == SCAN_SPACE) key_fire = 1;
    }
}
