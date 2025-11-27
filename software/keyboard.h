/*
 * Arquivo: keyboard.h
 * Descrição: Definições de registradores e scancodes do teclado.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

// Endereço base do controlador de teclado no barramento
#define KEYBOARD_AXI_BASE   0xe4a00000

// Registradores mapeados em memória
#define KEYBOARD_AXI_STATUS (*(volatile uint32_t *)(KEYBOARD_AXI_BASE + 0x010))
#define KEYBOARD_AXI_SDATA  (*(volatile uint32_t *)(KEYBOARD_AXI_BASE + 0x020))

// Bits de controle
#define KEYBOARD_AXI_STREADY (1 << 0)
#define KEYBOARD_AXI_STVALID (1 << 1) // Bit que indica dado pronto

// Scancodes (Códigos de varredura PS/2)
#define SCAN_A      0x1C
#define SCAN_D      0x23
#define SCAN_LEFT   0x6B
#define SCAN_RIGHT  0x74
#define SCAN_SPACE  0x29
#define SCAN_BREAK  0xF0

// Flags globais acessíveis pelo jogo
extern int key_left;
extern int key_right;
extern int key_fire;
extern int key_break;

void check_keyboard(void);

#endif
