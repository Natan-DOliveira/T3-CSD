#include <hf-risc.h>
#include "keyboard.h"

// Alocação real das variáveis globais
int key_left = 0;
int key_right = 0;
int key_fire = 0;
int key_break = 0;

void check_keyboard(void) {
    // 1. Verificação Rápida: Tem dado novo? (Bit 1 = VALID)
    if (!(KEYBOARD_AXI_STATUS & KEYBOARD_AXI_STVALID)) return;

    // 2. Lê o dado imediatamente
    uint8_t sc = KEYBOARD_AXI_SDATA;

    // Debug (Pode comentar para limpar o terminal)
    // printf("Scan: %x\n", sc); 

    // 3. Processamento do Break Code (F0)
    if (sc == 0xF0) {
        // Espera pelo próximo byte (código da tecla solta)
        // Timeout generoso para garantir captura (~10ms)
        int timeout = 500000; 
        
        while (!(KEYBOARD_AXI_STATUS & KEYBOARD_AXI_STVALID) && timeout--) ;
        
        if (timeout > 0) {
            uint8_t next_sc = KEYBOARD_AXI_SDATA;
            // printf("Rel: %x\n", next_sc); // Debug
            
            // Atualiza estado específico
            if (next_sc == SCAN_A || next_sc == SCAN_LEFT) key_left = 0;
            else if (next_sc == SCAN_D || next_sc == SCAN_RIGHT) key_right = 0;
            else if (next_sc == SCAN_SPACE) key_fire = 0;
        } else {
            // [CORREÇÃO CRÍTICA]
            // Se deu timeout aqui, significa que o hardware perdeu o código da tecla.
            // Para evitar que o boneco ande sozinho para sempre, resetamos tudo.
            // É melhor parar de andar do que bater na parede.
            key_left = 0;
            key_right = 0;
            key_fire = 0;
        }
        key_break = 0; 
    }
    else if (sc != 0xE0) { 
        // Processamento de Press
        if (sc == SCAN_A || sc == SCAN_LEFT) key_left = 1;
        else if (sc == SCAN_D || sc == SCAN_RIGHT) key_right = 1;
        else if (sc == SCAN_SPACE) key_fire = 1;
    }
}
