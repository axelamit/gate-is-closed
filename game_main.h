void user_isr(void);
void game_loop();
void game_init(void);
int calculate_baudrate_divider(int sysclk, int baudrate, int highspeed);