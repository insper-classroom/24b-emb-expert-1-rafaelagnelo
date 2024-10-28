#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/sockets.h"

#define WIFI_SSID "SEU_SSID"
#define WIFI_PASS "SUA_SENHA"

bool wifi_connect() {
    if (cyw43_arch_init()) {
        printf("Falha ao iniciar Wi-Fi\n");
        return false;
    }
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return false;
    }
    return true;
}

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);

    if (!wifi_connect()) {
        return 1;
    }

    while (true) {
        uint16_t valor = adc_read();
        char dados[200]; // Buffer aumentado para evitar estouro.

        snprintf(dados, sizeof(dados),
                 "POST /data HTTP/1.1\r\n"
                 "Host: IP_DO_SERVIDOR:5000\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %d\r\n\r\n"
                 "{\"valor\": %d}", 
                 strlen("{\"valor\": %d}") + 10, valor);

        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in servidor;
        servidor.sin_family = AF_INET;
        servidor.sin_addr.s_addr = inet_addr("IP_DO_SERVIDOR");
        servidor.sin_port = htons(5000);

        if (connect(socket_fd, (struct sockaddr*)&servidor, sizeof(servidor)) == 0) {
            write(socket_fd, dados, strlen(dados));
        } else {
            printf("Erro ao conectar ao servidor\n");
        }
        close(socket_fd);

        sleep_ms(10000);
    }

    cyw43_arch_deinit();
    return 0;
}
