void send_gamestate(int gamestate[]);

int check_timeout(void);
int getbtns(void);
void bn_sw_time(void);

void user_isr(void);
void game_loop();
void game_init(void);
int calculate_baudrate_divider(int sysclk, int baudrate, int highspeed);

void enable_interrupt(void);