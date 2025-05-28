#include "util_task1.h"

int main() {
    char input[MAX_COMMAND_LENGTH];
    while (1) {
        printf("1. Sing in\n2. Sing up\n> ");
        if (!fgets(input, sizeof(input), stdin)) continue;
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "1") == 0) {
            char login[MAX_LOGIN_LENGTH + 1];
            int pin;
            printf("Login: ");
            if (!fgets(login, sizeof(login), stdin)) continue;
            login[strcspn(login, "\n")] = '\0';
            
            printf("PIN: ");
            if (scanf("%d", &pin) != 1) {
                printf("Authorization error: %s\n", ErrorToString(ERROR_INVALID_PIN));
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n');

            Response auth = authentificate_user(login, pin);
            if (auth.error == ERROR_NONE) {
                command_loop(&auth.data.user);
            } else {
                printf("Authorization error: %s\n", ErrorToString(auth.error));
            }
        } else if (strcmp(input, "2") == 0) {
            char login[MAX_LOGIN_LENGTH + 1];
            int pin;
            printf("Login: ");
            if (!fgets(login, sizeof(login), stdin)) continue;
            login[strcspn(login, "\n")] = '\0';
            
            printf("PIN: ");
            if (scanf("%d", &pin) != 1) {
                printf("Registration error: %s\n", ErrorToString(ERROR_INVALID_PIN));
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n');

            Response reg = register_user(login, pin);
            if (reg.error != ERROR_NONE) {
                printf("Registration error: %s\n", ErrorToString(reg.error));
            } else {
                printf("Registration successful!\n");
            }
        } else {
            printf("Wrong choice\n");
        }
    }

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
    return 0;
}