#ifndef KEYBOARD_H
#define KEYBOARD_H

// Configurações de hardware (mantido do PDF: AXI para teclado PS/2)
#define KEYBOARD_AXI_BASE   0xe4a00000
#define KEYBOARD_AXI_STATUS (*(volatile uint32_t *)(KEYBOARD_AXI_BASE + 0x010))
#define KEYBOARD_AXI_SDATA  (*(volatile uint32_t *)(KEYBOARD_AXI_BASE + 0x020))
#define KEYBOARD_AXI_STVALID (1 << 1)

// Códigos de scan
#define SCAN_A      0x1C
#define SCAN_D      0x23
#define SCAN_LEFT   0x6B
#define SCAN_RIGHT  0x74
#define SCAN_SPACE  0x29
#define SCAN_BREAK  0xF0

extern int key_left, key_right, key_fire, key_break;

void check_keyboard() {
    uint8_t sc = KEYBOARD_AXI_SDATA;  // Fake read para ativar transferência
    int timeout = 20;  // Timeout ajustado para permitir continuação se não houver dados
    while (!(KEYBOARD_AXI_STATUS & KEYBOARD_AXI_STVALID) && timeout--) ;
    if (timeout > 0) {
        sc = KEYBOARD_AXI_SDATA;  // Real read
        printf("Tecla lida: 0x%x\n", sc);  // Print no terminal para depuração
        if (sc == SCAN_BREAK) {
            key_break = 1;
        } else if (key_break) {
            // Release
            if (sc == SCAN_A || sc == SCAN_LEFT) key_left = 0;
            if (sc == SCAN_D || sc == SCAN_RIGHT) key_right = 0;
            if (sc == SCAN_SPACE) key_fire = 0;
            key_break = 0;
        } else {
            // Press
            if (sc == SCAN_A || sc == SCAN_LEFT) key_left = 1;
            if (sc == SCAN_D || sc == SCAN_RIGHT) key_right = 1;
            if (sc == SCAN_SPACE) key_fire = 1;
        }
    }
}
#endif
